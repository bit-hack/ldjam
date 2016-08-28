#include "../framework/timer.h"
#include "../framework/vec2.h"
#include "../framework/const.h"

#include "surge.h"

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
extern uint64_t time_func();

typedef std::array<vec2f_t, 9> spline_t;

namespace {
std::array<spline_t, 6> s_splines = {
    spline_t{{
        { 160, -64 }, { 160, -32 }, { 220, 160 }, { 260, 130 }, { 160, 100 }, {  50, 130 }, { 100, 160 }, { 160,  60 }, { 160,  60 },
    }},
    spline_t{{
        { 65, -64 }, { 133,  16 }, { 180, 87 }, { 193, 150 }, { 113, 143 }, { 64, 90 }, { 180, 73 }, { 250, 97 }, { 180, 100 },
    }},
    spline_t{{
        { 314, 290 }, { 314, 210 }, { 300,  180 }, { 250, 150 }, { 180, 140 }, { 108, 140 }, { 64, 130 }, { 180, 60 }, { 250, 80 },
    }},
    spline_t{{
        { 35, 0 }, { 160, 100 }, { 280,  170 }, { 280, 20 }, { 160, 100 }, { 40, 170 }, { 35, 20 }, { 160, 100 }, { 160, 20 },
    }},
    spline_t{{
        { 160, -64 }, { 160, 50 }, { 100,  134 }, { 40, 220 }, { 160, 144 }, { 285, 220 }, { 234, 127 }, { 160, 45 }, { 160, 85 },
    }},
    spline_t{{
        { 35, -64 }, { 35, 120 }, { 35,  260 }, { 280, 260 }, { 280, 120 }, { 280, -64 }, { 160, -64 }, { 85, 45 }, { 220, 45 },
    }}
};
} // namespace {}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
enum {
    e_player,
    e_enemy,
    e_bullet,
    e_wave,
    e_powerup,
    e_explode,
    e_stars,
    e_bomb,
    e_boss,
    e_boss_missile,
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_stars_t : public object_ex_t<e_stars, obj_stars_t>
{
    static const size_t C_STARS = 256;

    std::array<vec2f_t, C_STARS> pos_;
    std::array<float, C_STARS> depth_;

    float speed_;
    prng::seed_t seed_;
    surge_t & surge_;
    float boost_;

    obj_stars_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , seed_(0xcafe)
        , speed_(3.f)
    {
        assert(service);
        for (size_t i = 0; i<C_STARS; ++i) {
            vec2f_t & pos = pos_[i];
            float & depth = depth_[i];
            pos.x = prng::randllu(seed_)%320;
            pos.y = prng::randllu(seed_)%240;
            depth = prng::randfu(seed_);
        }
    }

    virtual void tick() override {

        boost_ = minv<float>(boost_+0.03f, C_PI);

        const float speed = speed_+sinf(boost_)*2.f;

        const int length = int(2.5f + sinf(boost_));

        for (size_t i = 0; i<C_STARS; ++i) {
            vec2f_t & pos = pos_[i];
            float & depth = depth_[i];
            pos.y += speed + speed * depth;
            if (pos.y>260) {
                pos.y = -32;
                pos.x = prng::randllu(seed_) % 320;
            }
            
            auto draw = surge_.draw_;
            draw.draw_colour(0x10 + 32.f * depth, 0x10 + 48.f * depth, 0x10 + 64 * depth);
            draw.draw_rect(pos.x-1, pos.y-1, 2, length);
        }
    }

    void boost() {
        boost_ = 0.f;
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
struct obj_explode_t: public object_ex_t<e_explode, obj_explode_t>
{
    surge_t & surge_;
    float age_;
    bool alive_;
    float scale_;
    float life_;

    struct particle_t {
        vec2f_t pos_;
        vec2f_t vel_;
        float size_;
        bool bob_;
    };

    std::array<particle_t, 32> part_;

    obj_explode_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , age_(0.f)
        , alive_(true)
    {
        assert(service);
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        if (age_ >= life_) {
            destroy();
            return;
        }
        age_ += 3.f;
        auto draw = surge_.draw_;
        for (particle_t & p:part_) {
            vec2f_t pos = p.pos_+p.vel_ * age_;
            p.vel_.y += 0.05f;
#if 0
            draw.draw_colour(128, 192, 255);
            draw.draw_rect(pos.x-p.size_, pos.y-p.size_, p.size_*2, p.size_*2);
#else
            uint32_t size = clampv<uint32_t>(0, 5.f - p.size_, 4);

//            if (p.bob_) {
                draw.draw_sprite(sprite_t(e_sprite_smoke_1+size), pos);
//            }
            p.bob_ ^= true;
#endif
        }
    }
    
    void init(const vec2f_t & pos, float scale) {
        scale_ = scale;
        prng::seed_t seed = prng::seed_t(prng::bitmix(time_func()));
        for (particle_t & p:part_) {
            p.pos_ = pos;
            p.vel_ = vec2f_t{ prng::grandfs(seed), prng::grandfs(seed) + .5f } * scale;
            float v = (p.vel_.x*p.vel_.x+p.vel_.y*p.vel_.y);
            p.size_ = (1.f / (v + (1.f - v) * .1f)) * scale;
            p.bob_ = (prng::randllu(seed) & 1) == 1;
        }
        life_ = 48.f * scale;

        surge_.audio_.play_sound(sound_t(e_sound_explode_1 + (prng::randllu(seed)%4)));

    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
struct obj_bullet_t : public object_ex_t<e_bullet, obj_bullet_t>
{
    surge_t & surge_;
    body_ex_t body_;
    vec2f_t dir_;
    bool alive_;
    float dt_;

    obj_bullet_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(surge_.spatial_, vec2f_t{160, 120}, 4, this)
        , alive_(true)
        , dt_(0.f)
    {
        assert(service);
    }

    virtual void tick() override {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();

        dt_ += (dt_>C_2PI) ? -C_2PI : 3.0f;
#if 0
        draw.draw_colour(128, 192, 255);
        draw.draw_rect(p.x-2, p.y-2, 5, 5);
#else
        draw.draw_sprite(e_sprite_bullet, p+vec2f_t{sinf(dt_), 0.f});
#endif
        body_.move(body_.pos() + dir_);
        if (p.y < -32.f) {
            destroy();
        }
    }

    void init(const vec2f_t & pos, const vec2f_t & dir) {
        dir_ = dir;
        body_.move(pos + dir);
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_powerup_t: public object_ex_t<e_powerup, obj_powerup_t>
{
    surge_t & surge_;
    body_ex_t body_;
    bool alive_;
    bool blip_;

    obj_powerup_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
        , blip_(true)
    {
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
#if 0
        draw.draw_colour(128, 256, 128);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
#else
        if (blip_^=true)
            draw.draw_sprite(e_sprite_powerup_1, p);
#endif
        body_.move(body_.pos()+vec2f_t{0.f, 1.f});
        if (body_.pos().y > 260) {
            destroy();
            // 
            surge_.audio_.play_sound(e_sound_powerup_miss);
        }
    }

    void init(const vec2f_t & pos) {
        body_.move(pos);
        surge_.audio_.play_sound(e_sound_powerup_drop);
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_bomb_t: public object_ex_t<e_bomb, obj_bomb_t>
{
    surge_t & surge_;
    body_ex_t body_;
    bool alive_;
    vec2f_t vel_;
    prng::seed_t seed_;
    float dt_;

    obj_bomb_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
        , seed_(time_func())
        , dt_(0.f)
    {
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();

        dt_ += (dt_>C_2PI) ? -C_2PI : 3.0f;
#if 0
        draw.draw_colour(255, 128, 128);
        draw.draw_rect(p.x-1, p.y-1, 3, 3);
#else
        draw.draw_sprite(e_sprite_bomb, p+vec2f_t{ sinf(dt_) * 1.f, 0.f });
#endif
        body_.move(body_.pos()+body_.vel());
        if (body_.pos().y > 260) {
            destroy();
        }
    }

    void init(const vec2f_t & pos) {
        body_.move(pos);
        body_.vel() = vec2f_t::normalize(vec2f_t{prng::grandfs(seed_), 2.f}) * 2.7f;
        surge_.audio_.play_sound(e_sound_fire);
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_player_t : public object_ex_t<e_player, obj_player_t>
{
    typedef void (obj_player_t::*state_t)();

    surge_t & surge_;
    std::vector<state_t> fsm_;
    delta_time_t spawn_timer_;
    delta_time_t shoot_timer_;
    body_ex_t body_;
    float speed_;
    bool alive_;
    int32_t bob_;
    prng::seed_t seed_;
    float dt_;
    sprite_t sprite_;

    void defaults() {
        speed_ = 2.5f;
        shoot_timer_.interval_ = 1000/2;
        spawn_timer_.interval_ = 1000/15;
    }

    obj_player_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , spawn_timer_(time_func, 1000/15)
        , shoot_timer_(time_func, 120*4)
        , body_(surge_.spatial_, vec2f_t{160, 260}, 6, this)
        , speed_(3.f)
        , alive_(true)
        , bob_(0)
        , seed_(time_func())
        , dt_(0.f)
        , sprite_(e_sprite_player_c)
    {
        assert(service);
        fsm_.push_back(&obj_player_t::fsm_spawn);
        ref_.dec();
        defaults();
    }

    void shoot() {
        if (shoot_timer_.deltai()) {
            surge_.audio_.play_sound(e_sound_fire);
            shoot_timer_.reset();
            object_ref_t bullet = surge_.factory_.create(e_bullet, &surge_);
            prng::seed_t seed = time_func();
            bullet->cast<obj_bullet_t>().init(body_.pos(),
                                              vec2f_t{prng::randfs(seed) * 0.1f, -8.f});
        }
    }

    void move() {
        const uint8_t * key = SDL_GetKeyboardState(nullptr);

        float ydiff = 200 - body_.pos().y;

        sprite_ = e_sprite_player_c;

        vec2f_t v = {0.f, ydiff * 0.03f};
        if (key[SDL_SCANCODE_LEFT]) {
            if (body_.pos().x > 32)
                v.x -= speed_;
        sprite_ = e_sprite_player_l;
        }
        if (key[SDL_SCANCODE_RIGHT]) {
            if (body_.pos().x < (320-32))
                v.x += speed_;
            sprite_ = e_sprite_player_r;
        }
        if (fsm_.back()!=&obj_player_t::fsm_spawn) {
            if (key[SDL_SCANCODE_X]) {
                shoot();
            }
        }
        
        body_.move(body_.pos()+v);
    }

    void draw() {
        auto draw = surge_.draw_;
        dt_ += (dt_>C_2PI) ? -C_2PI : 0.05f;

        const vec2f_t p = body_.pos()+vec2f_t{0.f, sinf(dt_)*2.f};

#if 0
        draw.draw_colour(192, 192, 192);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
        
        if (bob_) {
            draw.draw_colour(192, 192, 255);
            draw.draw_rect(p.x-3, p.y+2, 6, 6);
            draw.draw_rect(p.x-2, p.y+8, 4, 4);
        }
#else

        draw.draw_sprite(sprite_, p);
        if (bob_) {
            draw.draw_sprite(e_sprite_thrust_1, p+vec2f_t{-1, 11.f});
        }
#endif
        bob_ ^= 1;
    }

    void fsm_tick() {
        draw();
        move();
        check_collide();
    }

    void fsm_spawn() {
        const uint64_t C_SPAWN_TIME = 45;
        if (spawn_timer_.deltai()>C_SPAWN_TIME) {
            fsm_.push_back(&obj_player_t::fsm_tick);
        }
        if (spawn_timer_.deltai()&1) {
            draw();
        }
        move();
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        assert(fsm_.size());
        state_t func = fsm_.back();
        (*this.*func)();
    }

    void kill() {
        // if we have not just spawned
        if (fsm_.back()!=&obj_player_t::fsm_spawn) {
            destroy();
            // sleep feels cool
            SDL_Delay(50);
            // screen shake feels awesome
            surge_.draw_.shake_ += 4.f;
            object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
            exp->cast<obj_explode_t>().init(body_.pos(), 2.f);
        }
    }

    void on_powerup() {
        surge_.audio_.play_sound(e_sound_powerup_pickup);
        switch (rand() % 2) {
        case (0):
            speed_ = minv<float>(speed_+1.f, 3.f);
            break;
        case (1):
            shoot_timer_.interval_ = maxv<uint64_t>(float(shoot_timer_.interval_) * .5f, 20);
            break;
        }
    }

    bool check_collide() {
        body_set_t set;
        surge_.spatial_.query_radius(body_.pos(), 8, set);
        for (auto itt:set) {
            if (!itt->obj_) {
                continue;
            }
            object_t & obj = *(itt->obj_);
            if (obj.is_a(e_powerup) && obj.cast<obj_powerup_t>().alive_) {
                obj.cast<obj_powerup_t>().destroy();
                on_powerup();
                continue;
            }
            if (obj.is_a(e_bomb) && obj.cast<obj_bomb_t>().alive_) {
                kill();
                obj.cast<obj_bomb_t>().destroy();
                continue;
            }
            if (obj.is_a(e_boss_missile)) {
                kill();
            }
        }
        return false;
    }

    void destroy() {
        if (alive_) {
            --surge_.lives_;
            surge_.audio_.play_sound(e_sound_player_die);
            alive_ = false;
            surge_.player_.dispose();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_enemy_t: public object_ex_t<e_enemy, obj_enemy_t>
{
    static const uint32_t C_POWERUP_CHANCE = 28;

    typedef void (obj_enemy_t::*state_t)();

    std::vector<state_t> fsm_;
    surge_t & surge_;
    vec2f_t ideal_pos_;
    body_ex_t body_;
    bool alive_;
    float dt_;
    spline_t * spline_;
    float track_;
    delta_time_t dive_timer_;
    prng::seed_t seed_;
    uint32_t health_;
    vec2f_t knockback_;

    obj_enemy_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(surge_.spatial_, vec2f_t{160, 120}, 4, this)
        , spline_(nullptr)
        , track_(0.f)
        , dive_timer_(time_func, 400)
        , seed_(time_func())
        , health_(1)
        , knockback_(vec2f_t::zero())
    {
        assert(service);
        fsm_.push_back(&obj_enemy_t::fsm_float);
        fsm_.push_back(&obj_enemy_t::fsm_spline);
    }

    void on_hit() {
        // sleep feels cool
        SDL_Delay(10);
        // screen shake feels awesome
        surge_.draw_.shake_ += 1.f;
        object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
        exp->cast<obj_explode_t>().init(body_.pos(), .5f);

        knockback_ = vec2f_t{prng::grandfs(seed_), -prng::randfu(seed_)} * 9.f;

        surge_.audio_.play_sound(e_sound_armour);
    }

    void on_death() {
        destroy();
        // sleep feels cool
        SDL_Delay(25);
        // screen shake feels awesome
        surge_.draw_.shake_ += 4.f;
        object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
        exp->cast<obj_explode_t>().init(body_.pos(), 1.f);
        // see if we should drop a powerup
        {
            if (prng::rand_chance(seed_, C_POWERUP_CHANCE)) {
                object_ref_t power = surge_.factory_.create<obj_powerup_t>(&surge_);
                power->cast<obj_powerup_t>().init(body_.pos());
            }
        }
    }

    bool check_collide() {
        // check if we should die
        body_set_t set;
        surge_.spatial_.query_radius(body_.pos(), 8, set);
        for (auto itt:set) {
            if (!itt->obj_) {
                continue;
            }
            object_t & obj = *(itt->obj_);
            if (obj.is_a(e_bullet) && obj.cast<obj_bullet_t>().alive_) {
                obj.cast<obj_bullet_t>().destroy();
                if (alive_) {
                    if (--health_) {
                        on_hit();
                    }
                    else {
                        on_death();
                        return true;
                    }
                }
            }
            if (obj.is_a(e_player)) {
                // collision with player!
                obj.cast<obj_player_t>().kill();
            }
        }
        return false;
    }

    void draw() {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos() + knockback_;
#if 0
        draw.draw_colour(256, 128, 64);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
#else
        draw.draw_sprite(e_sprite_enemy_2, p);
        draw.draw_sprite(e_sprite_enemy_1, p+vec2f_t{ sinf(dt_)*2.f, cosf(dt_)*2.f });
#endif
    }

    vec2f_t calc_pos() const {
        const vec2f_t center = {160.f, 80.f};
        return ideal_pos_+(center-ideal_pos_) * (sinf(dt_)*0.1f);
    }

    void drop_bomb() {
        object_ref_t bomb = surge_.factory_.create<obj_bomb_t>(&surge_);
        bomb->cast<obj_bomb_t>().init(body_.pos());
    }

    void fsm_float() {
        if (!alive_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        // floaty movement
        {
            dt_ = dt_<0.f ? (dt_+C_2PI-0.1f) : dt_-0.1f;
            body_.move(calc_pos());
        }
        draw();
        //
        while (dive_timer_.deltai()) {

            uint32_t chance = maxv<int32_t>(15, 100-int32_t(health_*10));

            dive_timer_.step();
            if (prng::rand_chance(seed_, chance)) {
                fsm_.push_back(&obj_enemy_t::fsm_dive);
                dive_timer_.reset();
                body_.vel() = vec2f_t::zero();
            }
            else
            if (prng::rand_chance(seed_, 40)) {
                drop_bomb();
            }
        }
    }

    void fsm_spline() {
        if (!alive_ || !spline_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        // are we finished following our spline
        if (int(track_+1)>spline_->size()) {
            dive_timer_.reset();
            fsm_.pop_back();
            return;
        }
        if ((health_>1) && prng::rand_chance(seed_, 240)) {
            drop_bomb();
        }
        // spline movement
        spline_t & spline = *spline_;
        {
            vec2f_t pm = int(track_-1) > 0 ? spline[int(track_-1)] : spline[int(track_)];
            vec2f_t p0 = spline[int(track_)];
            vec2f_t p1 = int(track_+1) < spline.size() ? spline[int(track_+1)] : calc_pos();
            vec2f_t p2 = int(track_+2) < spline.size() ? spline[int(track_+2)] : calc_pos();
            vec2f_t p = vec2f_t::hlerp(pm, p0, p1, p2, fpart(track_));
            body_.move(p);
            track_ += 0.04f;
        }
        draw();
    }

    void fsm_return() {
        if (!alive_ || !spline_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        track_ += 0.01f;
        if (vec2f_t::distance_sqr(body_.pos(), calc_pos()) < 10.f) {
            dive_timer_.reset();
            fsm_.pop_back();
            return;
        }
        vec2f_t diff = calc_pos()-body_.pos();
        body_.move(body_.pos()+diff * 0.03f);
        draw();
    }

    void fsm_dive() {
        if (!alive_ || !spline_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        if (body_.pos().y>260) {
            fsm_.pop_back();
            fsm_.push_back(&obj_enemy_t::fsm_return);
            body_.move(ideal_pos_+vec2f_t{0.f, -128.f});
            track_ = 0.f;
        }
        if (health_>1 && prng::rand_chance(seed_, 40)) {
            drop_bomb();
        }
        {
            vec2f_t & vel = body_.vel();
            vel += (vec2f_t{0.f, 0.2f} +vec2f_t{prng::grandfs(seed_), prng::grandfs(seed_)}) * 0.3f;
            vel = vec2f_t::normalize(vel) * 1.5f;
            body_.move(body_.pos()+vel);
        }
        draw();
    }

    virtual void tick() override {
        knockback_ *= .6f;
        assert(fsm_.size());
        state_t func = fsm_.back();
        // dispatch to finite state machine
        (*this.*func)();
    }

    void init(const vec2f_t & p, float seed, uint32_t spline, uint32_t health) {
        ideal_pos_ = p;
        dt_ = seed;
        body_.move(ideal_pos_);
        spline_ = &s_splines[spline];
        health_ = health / 2;
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_boss_missile_t: public object_ex_t<e_boss_missile, obj_boss_missile_t>
{
    surge_t & surge_;
    body_ex_t body_;
    bool alive_;
    vec2f_t vel_;
    prng::seed_t seed_;
    float speed_;
    uint32_t age_;

    obj_boss_missile_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
        , seed_(time_func())
        , speed_(5.5f)
        , age_(0)
    {
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        if (!surge_.player_.valid() || (++age_>60.f)) {
            destroy();
            return;
        }
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
#if 0
        draw.draw_colour(255, 128, 128);
        draw.draw_rect(p.x-2, p.y-2, 5, 5);
#else
        draw.draw_sprite(e_sprite_boss_missile, p);
#endif
        body_.move(body_.pos()+body_.vel());
        if (body_.pos().y > 240) {
            destroy();
        }
        vec2f_t diff = vec2f_t::normalize(surge_.player_->cast<obj_player_t>().body_.pos() - body_.pos());
        body_.vel() = vec2f_t::normalize(body_.vel()+diff * 0.3f) * speed_;
    }

    void init(const vec2f_t & pos, prng::seed_t seed) {
        seed_ = seed;
        body_.move(pos);
        body_.vel() = vec2f_t::normalize(vec2f_t{prng::grandfs(seed_),
                                                 prng::grandfs(seed_)+0.4f}) * speed_;
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
        }
    }
};

// 
struct obj_boss_t: public object_ex_t<e_boss, obj_boss_t>
{
    static const uint32_t C_PARTS = 18;

    struct state_t {
        float phase_;
        float ratio_a_;
        float ratio_b_;
    };

    std::array<state_t, 2> state_;

    surge_t & surge_;
    bool alive_;
    delta_time_t pattern_timer_;
    prng::seed_t seed_;
    float dt_;
    uint32_t part_count_;
    std::array<vec2f_t, C_PARTS> part_;
    float intro_;
    delta_time_t shoot_timer_;

    obj_boss_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , pattern_timer_(time_func, 1000 * 4)
        , seed_(0xbeeeeef)
        , dt_(0.f)
        , part_count_(C_PARTS)
        , intro_(0.f)
        , shoot_timer_(time_func, 1000 * 2)
    {
        assert(service);
        next_state();
        next_state();
        ref_.dec();
    }
    
    void get_state(state_t & out) const {
        float dt = pattern_timer_.deltaf();
        if (dt>1.f) dt = 1.f;
        if (dt<0.f) dt = 0.f;
        dt = smoothstep(dt);
        out.phase_   = lerp(state_[0].phase_,   state_[1].phase_,   dt);
        out.ratio_a_ = lerp(state_[0].ratio_a_, state_[1].ratio_a_, dt);
        out.ratio_b_ = lerp(state_[0].ratio_b_, state_[1].ratio_b_, dt);
    }

    void next_state() {
        state_[0] = state_[1];
        state_[1].phase_ = prng::randfu(seed_);
        switch (prng::randllu(seed_)%4) {
        case (0): state_[1].ratio_a_ = 1.f; state_[1].ratio_b_ = 1.f; break;
        case (1): state_[1].ratio_a_ = 1.f; state_[1].ratio_b_ = 2.f; break;
        case (2): state_[1].ratio_a_ = 1.f; state_[1].ratio_b_ = 3.f; break;
        case (3): state_[1].ratio_a_ = 2.f; state_[1].ratio_b_ = 3.f; break;
        }
    }

    vec2f_t get_pos(float delta) const {
        state_t s;
        get_state(s);
        vec2f_t temp = vec2f_t{
            sinf(dt_ + delta * (s.ratio_a_)),
            cosf(dt_ + delta * (s.ratio_b_) +
                 s.phase_ * C_PI) +
            sinf(dt_*8.f + delta*16.f) * 0.1f,
        };
        return vec2f_t::lerp(vec2f_t{ 160, -64 }, 
                             vec2f_t::scale(temp, vec2f_t{110, 50})+vec2f_t{160, 90},
                             intro_);
    }

    uint32_t scale(uint32_t index) const {
        return 6 + (index+1) / 2;
    }

    void draw() {
        const float offs = 1.f/float(part_count_);
        for (uint32_t i = 0; i<part_count_; ++i) {
            vec2f_t pos = part_[i];
            uint32_t size = scale(i);
            auto draw = surge_.draw_;
#if 0
            draw.draw_colour(256, 128, 64);
            draw.draw_rect(pos.x-size/2, pos.y-size/2, size, size);
#else
            if (i+1==part_count_) {
                // head section
                draw.draw_sprite(e_sprite_boss_2, pos, 1.f);
            }
            else {
                // body
                draw.draw_sprite(e_sprite_boss_1, pos, 0.3f + 0.7f*i*offs);
            }
#endif
        }
    }

    void on_hit(vec2f_t pos) {
        // sleep feels cool
        SDL_Delay(10);
        // screen shake feels awesome
        surge_.draw_.shake_ += 1.f;
        object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
        exp->cast<obj_explode_t>().init(pos, .5f);
        // 
        surge_.audio_.play_sound(e_sound_armour);
    }

    static const uint32_t C_POWERUP_CHANCE = 10;

    void on_destroy(vec2f_t pos) {
        // sleep feels cool
        SDL_Delay(10);
        // screen shake feels awesome
        surge_.draw_.shake_ += 4.f;
        object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
        exp->cast<obj_explode_t>().init(pos, 1.f);
        shoot_timer_.interval_ -= 30;
        // see if we should drop a powerup
        {
            if (prng::rand_chance(seed_, C_POWERUP_CHANCE)) {
                object_ref_t power = surge_.factory_.create<obj_powerup_t>(&surge_);
                power->cast<obj_powerup_t>().init(pos);
            }
        }
        // 
        surge_.audio_.play_sound(e_sound_boss_hurt);
    }

    virtual void shoot() {
        if (part_count_==0) {
            return;
        }
        if (!surge_.player_.valid()) {
            return;
        }
        // screen shake feels awesome
        surge_.draw_.shake_ += 1.f;
        // ping out a missile
        const vec2f_t pos = part_[part_count_-1];
        object_ref_t missile = surge_.factory_.create<obj_boss_missile_t>(&surge_);
        missile->cast<obj_boss_missile_t>().init(pos, prng::randllu(seed_));
        
        surge_.audio_.play_sound(e_sound_boss_fire);
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        
        intro_ += (intro_ < 1.f) ? 0.01f : 0.f;
        dt_ -= ((dt_ += 0.06f) < C_2PI) ? 0.f : C_2PI;

        while (pattern_timer_.deltaf() >= 1.f) {
            next_state();
            pattern_timer_.step();
        }

        body_set_t set;
        const float offs = C_PI/float(C_PARTS) * 0.4f;
        for (uint32_t i = 0; i<part_count_; ++i) {
            part_[i] = get_pos(offs*float(i));
            set.clear();
            surge_.spatial_.query_radius(part_[i], scale(i), set);

            for (auto itt:set) {
                if (!itt->obj_) {
                    continue;
                }
                object_t & obj = *(itt->obj_);
                if (obj.is_a(e_bullet) && obj.cast<obj_bullet_t>().alive_) {
                    obj.cast<obj_bullet_t>().destroy();

                    if (i==(part_count_-1)) {
                        on_destroy(part_[i]);
                        --part_count_;
                    }
                    else {
                        on_hit(part_[i]);
                    }
                }
            }
        }

        if (shoot_timer_.deltai()) {
            shoot_timer_.reset();
            shoot();
            shoot();
        }

        if (part_count_==0) {
            surge_.boss_.dispose();
        }

        draw();
    }

    void init() {
        pattern_timer_.reset();
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
            surge_.wave_.dispose();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_wave_t : public object_ex_t<e_wave, obj_wave_t>
{
    const int32_t C_COUNT = 24;
    const int32_t C_WIDTH = 8;
    const float C_SPACING = 32.f;
    const float C_OFFSET = (320-(C_WIDTH*C_SPACING)) / 2 + C_SPACING / 2;

    surge_t & surge_;
    uint32_t made_;
    bool alive_;
    prng::seed_t seed_;
    delta_time_t spawn_timer_;

    obj_wave_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , made_(0)
        , alive_(true)
        , seed_(time_func())
        , spawn_timer_(time_func, 100)
    {
        assert(service);
    }

    void spawn() {
        float seed = C_2PI / float(C_COUNT);
        object_ref_t e = surge_.factory_.create<obj_enemy_t>(&surge_);
        vec2f_t pos = vec2f_t{C_OFFSET + (made_ % C_WIDTH) * C_SPACING,
                              C_OFFSET + (made_ / C_WIDTH) * C_SPACING};
        
        e->cast<obj_enemy_t>().init(pos,
                                    (pos.x / 100.f + pos.y / 80.f) * seed,
                                    prng::bitmix(seed_) % s_splines.size(),
                                    surge_.difficulty_);
        surge_.enemy_.push_back(e);
        ++made_;
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        if (spawn_timer_.deltai()) {
            spawn_timer_.step();
            spawn();
            surge_.audio_.play_sound(e_sound_wave_spawn);
        }
        if (made_>=C_COUNT) {
            destroy();
        }
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
            surge_.wave_.dispose();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
surge_t::surge_t(draw_t & draw, audio_t & audio)
    : draw_(draw)
    , audio_(audio)
    , difficulty_(1)
    , lives_(0)
{
    factory_.add_creator<obj_player_t>();
    factory_.add_creator<obj_enemy_t>();
    factory_.add_creator<obj_bullet_t>();
    factory_.add_creator<obj_wave_t>();
    factory_.add_creator<obj_explode_t>();
    factory_.add_creator<obj_powerup_t>();
    factory_.add_creator<obj_stars_t>();
    factory_.add_creator<obj_bomb_t>();
    factory_.add_creator<obj_boss_t>();
    factory_.add_creator<obj_boss_missile_t>();

    stars_ = factory_.create<obj_stars_t>(this);
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
void surge_t::tick()
{
    for (auto itt = enemy_.begin(); itt!=enemy_.end();) {
        object_ref_t & ref = *itt;
        if (ref.valid()&&!ref->cast<obj_enemy_t>().alive_) {
            ref.dispose();
            itt = enemy_.erase(itt);
        }
        else {
            ++itt;
        }
    }

    if (player_.valid()) {
        if (enemy_.size()==0&&!wave_.valid()&&!boss_.valid()) {
            stars_->cast<obj_stars_t>().boost();

            if ((difficulty_%4==0)&&difficulty_||false) {
                audio_.play_sound(e_sound_boss_spawn);
                boss_ = factory_.create<obj_boss_t>(this);
            }
            else {
                wave_ = factory_.create<obj_wave_t>(this);
            }
            ++difficulty_;
        }
    }

    if (!player_.valid() && lives_) {
        player_ = factory_.create<obj_player_t>(this);
    }

    factory_.tick();
    factory_.collect();

    for (int i = 0; i<lives_; ++i) {
        draw_.draw_sprite(e_sprite_life, vec2f_t{4.f, 4.f+8.f*i});
    }

    if (lives_==0&&!player_.valid()) {
        // update menu
        draw_.draw_title();
        //
        const uint8_t * keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_X]) {
            lives_ = 3;
            clear_enemies();
            difficulty_ = 1;
        }
    }
}

void surge_t::clear_enemies() {
    for (auto itt = enemy_.begin(); itt!=enemy_.end();) {
        object_ref_t & ref = *itt;
        if (ref.valid() && ref->cast<obj_enemy_t>().alive_) {
            ref->cast<obj_enemy_t>().destroy();
        }
        else {
            ++itt;
        }
    }
    if (boss_.valid()) {
        boss_.dispose();
    }
}

void surge_t::init()
{
}
