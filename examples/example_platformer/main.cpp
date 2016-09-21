#include <SDL/SDL.h>

#include "../../framework_core/fsm.h"
#include "../../framework_core/random.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/tiles.h"
#include "../../framework_core/timer.h"
#include "../../framework_draw/draw.h"

enum {
    e_object_player,
    e_object_camera
};

struct service_t {

    object_factory_t * factory_;
    draw_t * draw_;
    collision_map_t * map_;

    object_ref_t player_;
};

struct player_t : public object_ex_t<e_object_player, player_t> {
    const int32_t _WIDTH = 4;
    const int32_t _HEIGHT = 10;

    typedef fsm_t<player_t> player_fsm_t;
    typedef player_fsm_t::fsm_state_t player_state_t;

    service_t * service_;
    vec2f_t pos_[2];
    float dx_;

    player_fsm_t fsm_;
    player_state_t fsm_state_air_; // falling jumping
    player_state_t fsm_state_run_; // running on ground
    player_state_t fsm_state_slide_; // wall slide

    player_t(object_service_t s)
        : object_ex_t()
        , service_(static_cast<service_t*>(s))
        , fsm_(this)
        , fsm_state_air_(&player_t::tick_air)
        , fsm_state_run_(&player_t::tick_run)
        , fsm_state_slide_(&player_t::tick_slide)
        , dx_(0.f)
    {
        pos_[0] = pos_[1] = vec2f_t{64, 64};
    }

    rectf_t bound() const {
        return rectf_t{
            pos_[1].x-_WIDTH,
            pos_[1].y-_HEIGHT,
            pos_[1].x+_WIDTH,
            pos_[1].y };
    }

    // bound swept in size by velocity
    rectf_t swept_bound() const {
        rectf_t fat_bound = bound();
        const vec2f_t vel = pos_[1] - pos_[0];
        fat_bound.x0 -= max2(vel.x, 0.f);
        fat_bound.y0 -= max2(vel.y, 0.f);
        fat_bound.x1 -= min2(vel.x, 0.f);
        fat_bound.y1 -= min2(vel.y, 0.f);
        return fat_bound;
    }

    void tick_run() {
        const float _GRAVITY  = .4f;
        const float _FRICTION = 0.3f;
        const float _Y_FRINGE = 1.f;

        rectf_t bound = swept_bound();
        bound.y1 += _Y_FRINGE;

        vec2f_t res;
        if (!service_->map_->collide(bound, res)) {
            fsm_.state_change(fsm_state_air_);
            return;
        }
        else {
            // reduce size slightly to ensure continuous collision
            res.y -= signv<float>(res.y) * _Y_FRINGE;
            // apply collision response as impulse
            pos_[1] += res;
            // find velocity (verlet)
            const vec2f_t vel = pos_[1] - pos_[0];
            // integrate
            pos_[0] = pos_[1];
            pos_[1] += vel + vec2f_t {dx_, _GRAVITY};
            // ground friction
            pos_[0].x = lerp(pos_[0].x, pos_[1].x, _FRICTION);
        }
    }

    void tick_air() {
        const float _GRAVITY  = .4f;
        const float _X_RESIST = .2f;
        const float _Y_RESIST = .04f;

        vec2f_t res;
        if (service_->map_->collide(swept_bound(), res)) {
            const vec2f_t vel = pos_[1] - pos_[0];
            const bool falling = vel.y > 0.f;
            // feet landed on something
            if (res.y < 0.f && falling) {
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
            // integrate
            pos_[0] = pos_[1];
            pos_[1] += vel + vec2f_t{dx_, _GRAVITY};
        }
        // air resistance
        {
            pos_[0].x = lerp(pos_[0].x, pos_[1].x, _X_RESIST);
            pos_[0].y = lerp(pos_[0].y, pos_[1].y, _Y_RESIST);
        }
    }

    void tick_slide() {
        const float _GRAVITY = 0.4f;
        const float _FRICTION = 0.4f;

        const bool falling = (pos_[1] - pos_[0]).y > 0.f;
        vec2f_t res;
        if (!service_->map_->collide(swept_bound(), res)) {
            // no collision so falling
            fsm_.state_change(fsm_state_air_);
            return;
        }
        // feet landed on something
        if (res.y < 0.f && falling) {
            fsm_.state_change(fsm_state_run_);
            return;
        }
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
        pos_[0].y = lerp(pos_[0].y, pos_[1].y, _FRICTION);
    }

    void jump() {
        const float _WJMP_Y = 4.f;
        const float _WJMP_X = 3.f;
        const float _JMP_SIZE = 4.f;
        const int32_t _X_SENSE = 2;
        // if we are on the ground
        if (fsm_.state() == fsm_state_run_) {
            pos_[0].y += _JMP_SIZE;
            fsm_.state_change(fsm_state_air_);
            return;
        }
        // if we are sliding
        if (fsm_.state() == fsm_state_slide_) {
            rectf_t b = bound();
            // fatten bound in x axis
            b.x0 -= _X_SENSE;
            b.x1 += _X_SENSE;
            // test for collision to deduce slide side
            vec2f_t res;
            if (service_->map_->collide(b, res)) {
                if (res.x > 0.f) {
                    pos_[1] += vec2f_t {_WJMP_X,-_WJMP_Y};
                }
                if (res.x < 0.f) {
                    pos_[1] += vec2f_t {-_WJMP_X,-_WJMP_Y};
                }
                fsm_.state_change(fsm_state_air_);
                return;
            }
        }
    }

    vec2f_t get_pos() const {
        return pos_[1];
    }

    vec2f_t get_velocity() const {
        return pos_[1] - pos_[0];
    }

    void move(float dx) {
        dx_ = dx;
    }

    virtual void tick() override {
        // tick the fsm
        if (fsm_.empty()) {
            fsm_.state_push(fsm_state_air_);
        }
        fsm_.tick();
        // fsm state indicator
        if (fsm_.state() == fsm_state_run_) {
            service_->draw_->colour_ = 0xff0000;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }
        if (fsm_.state() == fsm_state_air_) {
            service_->draw_->colour_ = 0x00ff00;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }
        if (fsm_.state() == fsm_state_slide_) {
            service_->draw_->colour_ = 0x0000ff;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }
        // draw player body
        service_->draw_->colour_ = 0x408060;
        service_->draw_->rect(recti_t(swept_bound()));
        service_->draw_->colour_ = 0x406080;
        service_->draw_->rect(recti_t(bound()));
    }
};

struct camera_t : public object_ex_t<e_object_camera, camera_t> {

    service_t & service_;

    vec2f_t target_;
    vec2f_t pos_;

    camera_t(object_service_t s)
        : service_(*reinterpret_cast<service_t*>(s))
        , pos_{32, 32}
    {
    }

    virtual void tick() override {
        // if we have a valid player
        if (service_.player_.valid()) {
            // get player state
            player_t & player = service_.player_->cast<player_t>();
            const vec2f_t pos = player.get_pos();
            const vec2f_t vel = player.get_velocity();
            // project where the player is going
            const vec2f_t proj = pos + vel * 12.f;
            // raycast from player to future point
            vec2f_t hit;
            if (service_.map_->raycast(pos + vec2f_t{0, -8}, proj, hit)) {
                hit = vec2f_t::nearest(pos, hit, proj);
                // ease towards map intersection
                target_ = vec2f_t::lerp(target_, hit, .5f);
            }
            else {
                // ease towards future projection
                target_ = vec2f_t::lerp(target_, proj, .5f);
            }
        }
        // smooth out camera position
        pos_ = vec2f_t::lerp(pos_, target_, 0.2f);
        // draw position and target
        service_.draw_->colour_ = 0x509030;
        service_.draw_->plot(vec2i_t(pos_));
        service_.draw_->colour_ = 0x304050;
        service_.draw_->circle(vec2i_t(target_), 3);
        // draw screen frame
        service_.draw_->colour_ = 0xa0a0a0;
        service_.draw_->line(pos_+vec2f_t{-64, -48}, pos_+vec2f_t{ 64, -48});
        service_.draw_->line(pos_+vec2f_t{-64,  48}, pos_+vec2f_t{ 64,  48});
        service_.draw_->line(pos_+vec2f_t{-64,  48}, pos_+vec2f_t{-64, -48});
        service_.draw_->line(pos_+vec2f_t{ 64,  48}, pos_+vec2f_t{ 64, -48});
    }
};

struct app_t {

    static const int32_t _MAP_WIDTH = 320 / 16;
    static const int32_t _MAP_HEIGHT = 240 / 16;

    SDL_Surface * screen_;
    bitmap_t target_;

    bool active_;

    timing_t<uint32_t> timer_;
    random_t rand_;

    draw_t draw_;
    collision_map_t map_;
    object_factory_t factory_;

    service_t service_;

    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , map_(vec2i_t{_MAP_WIDTH, _MAP_HEIGHT}, vec2i_t{16, 16})
        , rand_(0x12345)
        , factory_(&service_)
    {
        service_.draw_ = &draw_;
        service_.map_ = &map_;
    }

    bool app_init() {
        if (SDL_Init(SDL_INIT_VIDEO)) {
            return false;
        }
        screen_ = SDL_SetVideoMode(640, 480, 32, 0);
        if (!screen_) {
            return false;
        }
        if (!bitmap_t::create(320, 240, target_)) {
            return false;
        }
        if (!target_.valid()) {
            return false;
        }
        draw_.set_target(target_);
        // setup frame timer
        timer_.period_ = 1000 / 30;
        timer_.func_ = SDL_GetTicks;
        timer_.reset();
        return true;
    }

    bool map_init() {

        const int32_t _CHANCE = 10;

        map_.clear(0);
        uint8_t * tile = map_.get();
        for (int32_t y=0; y<map_.size().y; ++y) {
            for (int32_t x=0; x<map_.size().x; ++x) {
                bool set = rand_.rand_chance(_CHANCE) || y==(map_.size().y-1);
                tile[x + y * map_.size().x] = set ? e_tile_solid : 0;
            }
        }
        // preprocess map for collision data
        map_.preprocess();
        return true;
    }

    bool map_draw() {
        draw_.colour_ = 0x204060;
        uint8_t * tile = map_.get();
        for (int32_t y=0; y<map_.size().y; ++y) {
            for (int32_t x=0; x<map_.size().x; ++x) {
                const uint8_t t = tile[x + y * map_.size().x];
                if (!(t & e_tile_solid)) {
                    continue;
                }
                draw_.rect(recti_t(x * 16, y * 16, 16, 16, recti_t::e_relative));
            }
        }
    }

    bool poll_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {

                switch (event.key.keysym.sym) {
                case (SDLK_ESCAPE):
                    return false;
                case (SDLK_UP):
                    service_.player_->cast<player_t>().jump();
                    break;
                default:
                    break;
                }
            }
        }
        return true;
    }

    bool tick() {
        draw_.colour_ = 0x202020;
        draw_.clear();
        map_draw();

        float dx = 0.f;
        {
            const uint8_t *key = SDL_GetKeyState(nullptr);
            dx += key[SDLK_LEFT] ? -1.f : 0.f;
            dx += key[SDLK_RIGHT] ? 1.f : 0.f;
        }

        player_t & player = service_.player_->cast<player_t>();
        player.move(dx);

        factory_.tick();
        return true;
    }

    bool main(const int argc, char *args[]) {
        if (!app_init()) {
            return false;
        }
        if (!map_init()) {
            return false;
        }

        factory_.add_creator<player_t>();
        service_.player_ = factory_.create(e_object_player);

        factory_.add_creator<camera_t>();
        object_ref_t camera = factory_.create(e_object_camera);

        while (active_) {
            if (!poll_events()) {
                return false;
            }
            if (timer_.frame()) {
                if (!tick()) {
                    return false;
                }
                assert(screen_);
                draw_.render_2x(screen_->pixels,
                                screen_->pitch / sizeof(uint32_t));
                SDL_Flip(screen_);
            }
            else {
                SDL_Delay(1);
            }
        }
        return true;
    }
};

int main(const int argc, char *args[]) {
    app_t app;
    return app.main(argc, args) ? 0 : 1;
}
