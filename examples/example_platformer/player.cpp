#include "player.h"
#include "camera.h"
#include "particles.h"
#include "map.h"

using namespace tengu;

player_anim_t::player_anim_t()
    : sheet_(108, 24)
    , run_("run", ta_sequence_t::e_end_loop)
    , stand_("stand", ta_sequence_t::e_end_hold)
    , jump_("jump", ta_sequence_t::e_end_hold)
    , fall_("fall", ta_sequence_t::e_end_hold)
    , slide_("slide", ta_sequence_t::e_end_loop)
    , skid_("skid", ta_sequence_t::e_end_hold)
    , xflip_(false)
{
    sheet_.add_grid(12, 12);
    controller_.set_sheet(&sheet_);
    // construct animation sequences
    run_.op_interval(3).op_offset(-6, -13)
            .op_event(e_foot_fall).op_frame(1).op_frame(2)
            .op_event(e_foot_fall).op_frame(3).op_frame(4);
    stand_.op_offset(-6, -13).op_frame(0);
    jump_ .op_offset(-6, -13).op_frame(5);
    fall_ .op_offset(-6, -13).op_frame(6);
    skid_ .op_offset(-6, -13).op_frame(7);
    slide_.op_interval(6).op_offset(-6, -13).op_event(e_slide_loop).op_frame(8);
    // set to stand by default
    controller_.push_sequence(&run_);
    // load the sprite sheet
    bitmap_.load("assets/ninja.bmp");
}

bool player_anim_t::event_foot_fall() {
    bool out = foot_fall_;
    foot_fall_ = false;
    return out;
}

bool player_anim_t::event_slide_loop() {
    bool out = slide_loop_;
    slide_loop_ = false;
    return out;
}

void player_anim_t::render(const vec2i_t & pos, draw_ex_t & draw_) {
    if (!bitmap_.valid()) {
        return;
    }
    vec2i_t offset;
    if (!controller_.get_offset(offset.x, offset.y)) {
        return;
    }
    recti_t src;
    if (!controller_.get_frame(src)) {
        return;
    }
    blit_ = blit_info_t {
        &bitmap_,
        pos + offset,
        src,
        e_blit_key,
        xflip_
    };
    draw_.key_ = 0xff00ff;
    draw_.blit<true>(blit_);
}

void player_anim_t::tick(int32_t delta) {
    controller_.tick(delta);
    uint32_t event;
    while (controller_.get_event(event)) {
        foot_fall_ |= (event == e_foot_fall);
        slide_loop_ |= (event == e_slide_loop);
    }
}

void player_anim_t::set_x_dir(float dir) {
    xflip_ = dir<0.f;
}

void player_anim_t::play(ta_sequence_t & seq) {
    if (!controller_.is_playing(&seq)) {
        controller_.set_sequence(&seq);
    }
}

player_shadow_t::player_shadow_t(object_service_t service)
    : service_(*static_cast<service_t*>(service))
{
    order_ = _ORDER;
}

void player_shadow_t::tick() {

    object_ref_t player_ref = service_.objects_["player"];
    if (!player_ref.valid()) {
        return;
    }
    player_t & player = player_ref->cast<player_t>();
    const vec2f_t p = player.pos_[1];
    vec2f_t hit;
    obj_map_t & map = service_.objects_["map"]->cast<obj_map_t>();
    if (map.collide_.raycast(vec2f_t{p.x, p.y-4.f},
                              vec2f_t{p.x, p.y+10.f}, hit)) {

        if (vec2f_t::distance_sqr(p, hit) < 32*32) {
            service_.draw_.colour_ = 0x303030;
            service_.draw_.rect<true>(
                    recti_t(rectf_t{
                            hit.x - 3,
                            hit.y + 0,
                            hit.x + 3,
                            hit.y + 2}));
        }
    }
}

player_t::player_t(object_service_t s)
    : object_ex_t()
    , service_(*static_cast<service_t*>(s))
    , fsm_(this)
    , fsm_state_air_(&player_t::tick_air)
    , fsm_state_run_(&player_t::tick_run)
    , fsm_state_slide_(&player_t::tick_slide)
    , dx_(0.f)
{
    pos_[0] = pos_[1] = vec2f_t{64, 64};
    order_ = _ORDER;
    shadow_ = service_.factory_.create<player_shadow_t>();
}

rectf_t player_t::bound() const {
    return rectf_t{
            pos_[1].x-_WIDTH,
            pos_[1].y-_HEIGHT,
            pos_[1].x+_WIDTH,
            pos_[1].y };
}

// bound swept in size by velocity
rectf_t player_t::swept_bound() const {
    rectf_t fat_bound = bound();
    const vec2f_t vel = pos_[1] - pos_[0];
    fat_bound.x0 -= maxv(vel.x, 0.f);
    fat_bound.y0 -= maxv(vel.y, 0.f);
    fat_bound.x1 -= minv(vel.x, 0.f);
    fat_bound.y1 -= minv(vel.y, 0.f);
    return fat_bound;
}

void player_t::tick_run() {
    const float _GRAVITY  = .4f;
    const float _FRICTION = 0.3f;
    const float _Y_FRINGE = 1.f;
    // create foot fall dust splodges
    if (anim_.event_foot_fall()) {
        const vec2f_t vel = pos_[1] - pos_[0];
        service_.factory_.create<dust_t>(
            2,
            pos_[1],
            vec2f_t{0.f, 0.f} - vel * .1f,
            vec2f_t{0.f, 0.0f},
            .2f
        );
    }
    // integration stage
    {
        //
        const vec2f_t vel = pos_[1] - pos_[0];
        // select animation to play
        anim_.play((absv(vel.x) > 0.4f) ? anim_.run_ : anim_.stand_);
        // integrate
        pos_[0] = pos_[1];
        pos_[1] += vel + vec2f_t {dx_, _GRAVITY};
        // ground friction
        pos_[0].x = lerp(pos_[0].x, pos_[1].x, _FRICTION);
    }
    // collision response
    rectf_t bound = swept_bound();
    vec2f_t res;
    obj_map_t & map = service_.objects_["map"]->cast<obj_map_t>();
    if (map.collide_.collide(bound, /* vel */ pos_[1]-pos_[0], res)) {
        // reduce size slightly to ensure continuous collision
        res.y -= signv<float>(res.y) * _Y_FRINGE;
        // apply collision response as impulse
        pos_[1] += res;
        // avoid bounce with certain responses
        pos_[0].y = minv(pos_[0].y, pos_[1].y);
    }
    else {
        fsm_.state_change(fsm_state_air_);
    }
}

void player_t::tick_air() {
    const float _GRAVITY  = .4f;
    const float _X_RESIST = .2f;
    const float _Y_RESIST = .04f;

    vec2f_t res;
    obj_map_t & map = service_.objects_["map"]->cast<obj_map_t>();
    if (map.collide_.collide(swept_bound(), res)) {
        const vec2f_t vel = pos_[1] - pos_[0];
        const bool falling = vel.y > 0.f;
        // feet landed on something
        if (res.y < 0.f && falling) {

            if (vel.y > 2.f) {
                service_.factory_.create<dust_t>(
                        4,
                        pos_[1],
                        vec2f_t{0.f, 0.f},
                        vec2f_t{0.f, 0.0f},
                        0.3f
                );
            }

            if (vel.y > 6.f) {
                service_.objects_["camera"]->cast<camera_t>().shake(1.f);
            }


            fsm_.state_change(fsm_state_run_);
            return;
        }
        // being pushed in the x axis
        if (absv(res.x) >= 0.f && (res.y == 0.f)) {
            if (signv(res.x) != signv(vel.x)) {
                fsm_.state_change(fsm_state_slide_);
                return;
            }
        }
        // apply collision as impulse
        pos_[1] += res;
        //
        if (res.y > 0.f) {
            pos_[0].y = pos_[1].y;
        }
    }
    // glide threw the air
    {
        // find velocity (verlet)
        const vec2f_t vel = pos_[1] - pos_[0];
        // select animation to play
        anim_.play( (vel.y < 0.0f) ? anim_.jump_ : anim_.fall_);
        // integrate
        pos_[0] = pos_[1];
        pos_[1] += vel + vec2f_t{dx_, _GRAVITY};
    }
    // air resistance
    {
        // find velocity (verlet)
        const vec2f_t vel = pos_[1] - pos_[0];
        gamepad_t & gamepad = *service_.gamepad_;

        if (!gamepad.button_[gamepad_joy_t::e_button_x] && vel.y < 0.f) {
            pos_[0].y = lerp(pos_[0].y, pos_[1].y, 0.2f);
        }

        pos_[0].x = lerp(pos_[0].x, pos_[1].x, _X_RESIST);
        pos_[0].y = lerp(pos_[0].y, pos_[1].y, _Y_RESIST);
    }
}

void player_t::tick_slide() {
    const float _GRAVITY = .4f;
    const float _FRICTION_DOWN = .4f;
    const float _FRICTION_UP = .1f;

    const bool falling = (pos_[1] - pos_[0]).y > 0.f;
    vec2f_t res;
    obj_map_t & map = service_.objects_["map"]->cast<obj_map_t>();
    if (!map.collide_.collide(swept_bound(), res)) {
        // no collision so falling
        fsm_.state_change(fsm_state_air_);
        return;
    }
    // head hit something
    if (res.y > 0.f) {
        pos_[1].y += res.y;
    }
    // feet landed on something
    if (res.y < 0.f && falling) {
        fsm_.state_change(fsm_state_run_);
        return;
    }
    // create dust while sliding
    if (anim_.event_slide_loop()) {
        service_.factory_.create<dust_t>(
            2,
            pos_[1] + vec2f_t{0,-10},
            vec2f_t{0.f, 0.f},
            vec2f_t{0.f, 0.1f},
            .2f
        );
    }
    // select animation to play
    anim_.set_x_dir(res.x);
    anim_.play(anim_.slide_);
    // reduce size slightly to ensure continuous collision
    res.x -= signv(res.x);
    // apply resolution as simple translate
    pos_[1].x += res.x;
    pos_[0].x = pos_[1].x;
    // find velocity (verlet)
    const vec2f_t vel = pos_[1] - pos_[0];
    // integrate
    pos_[0] = pos_[1];
    pos_[1] += vel + vec2f_t{0.f, _GRAVITY};
    // wall friction
    pos_[0].y = lerp(pos_[0].y, pos_[1].y,
                     (vel.y > 0.f) ? _FRICTION_DOWN :
                                     _FRICTION_UP);
}

void player_t::jump() {
    if (fsm_.empty())
        return;

    const float _WJMP_Y = 6.f;
    const float _WJMP_X = 3.f;
    const float _JMP_SIZE = 4.f;
    const int32_t _X_SENSE = 2;

    bool jumping = false;

    // if we are on the ground
    if (!jumping && fsm_.state() == fsm_state_run_) {
        pos_[0].y += _JMP_SIZE;
        fsm_.state_change(fsm_state_air_);
        service_.factory_.create<dust_t>(
            4,
            pos_[1],
            vec2f_t{0.f, .5f},
            vec2f_t{0.f, 0.0f},
            .5f
        );
        jumping = true;
    }
    // if we are sliding
    if (!jumping && fsm_.state() == fsm_state_slide_) {
        rectf_t b = bound();
        // fatten bound in x axis
        b.x0 -= _X_SENSE;
        b.x1 += _X_SENSE;
        // test for collision to deduce slide side
        vec2f_t res;
        obj_map_t & map = service_.objects_["map"]->cast<obj_map_t>();
        if (map.collide_.collide(b, res)) {
            if (res.x > 0.f) {
                pos_[1] += vec2f_t {_WJMP_X,-_WJMP_Y};
            }
            if (res.x < 0.f) {
                pos_[1] += vec2f_t {-_WJMP_X,-_WJMP_Y};
            }
            fsm_.state_change(fsm_state_air_);
            service_.factory_.create<dust_t>(
                4,
                pos_[1],
                vec2f_t{0.f, 0.f},
                vec2f_t{0.f, 0.0f},
                .5f
            );
            jumping = true;
        }
    }
    //
    if (jumping) {
        service_.factory_.create<player_splash_t>(anim_.blit_info());
    }
}

vec2f_t player_t::get_pos() const {
    return pos_[1];
}

vec2f_t player_t::get_velocity() const {
    return pos_[1] - pos_[0];
}

void player_t::move(float dx) {
    dx_ = dx;
    if (dx!=0.f) {
        anim_.set_x_dir(dx);
    }
}

void player_t::tick() {
    // tick the fsm
    if (fsm_.empty()) {
        fsm_.state_push(fsm_state_air_);
    }
    fsm_.tick();
#if 1
    // fsm state indicator
    if (fsm_.state() == fsm_state_run_) {
        service_.draw_.colour_ = 0xff0000;
    }
    if (fsm_.state() == fsm_state_air_) {
        service_.draw_.colour_ = 0x00ff00;
    }
    if (fsm_.state() == fsm_state_slide_) {
        service_.draw_.colour_ = 0x0000ff;
    }
    service_.draw_.circle<false>(vec2i_t{8, 16}, 3);
#endif
#if 0
    // draw player body
    service_->draw_.colour_ = 0x408060;
    service_->draw_.rect<true>(recti_t(swept_bound()));
    service_->draw_.colour_ = 0x406080;
    service_->draw_.rect<true>(recti_t(bound()));
#endif
    // draw using animation controller
    anim_.tick(1);
    anim_.render(vec2i_t(pos_[1]), service_.draw_);
}
