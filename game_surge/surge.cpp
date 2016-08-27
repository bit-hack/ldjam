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
    e_bomb
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
        draw.draw_colour(128, 192, 255);
        for (particle_t & p:part_) {
            vec2f_t pos = p.pos_+p.vel_ * age_;
            p.vel_.y += 0.05f;
            draw.draw_rect(pos.x-p.size_, pos.y-p.size_, p.size_*2, p.size_*2);
        }
    }
    
    void init(const vec2f_t & pos, float scale) {
        scale_ = scale;
        prng::seed_t seed = prng::seed_t(time_func());
        for (particle_t & p:part_) {
            p.pos_ = pos;
            p.vel_ = vec2f_t{ prng::grandfs(seed), prng::grandfs(seed) + .5f } * scale;
            float v = (p.vel_.x*p.vel_.x+p.vel_.y*p.vel_.y);
            p.size_ = (1.f / (v + (1.f - v) * .1f)) * scale;
        }
        life_ = 64.f * scale;
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

    obj_bullet_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(surge_.spatial_, vec2f_t{160, 120}, 4, this)
        , alive_(true)
    {
        assert(service);
    }

    virtual void tick() override {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(128, 192, 255);
        draw.draw_rect(p.x-1, p.y-1, 3, 3);
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

    obj_powerup_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
    {
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(128, 256, 128);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
        body_.move(body_.pos()+vec2f_t{0.f, 1.f});
        if (body_.pos().y > 260) {
            destroy();
        }
    }

    void init(const vec2f_t & pos) {
        body_.move(pos);
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

    obj_bomb_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
        , seed_(time_func())
    {
    }

    virtual void tick() override {
        if (!alive_) {
            return;
        }
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(255, 128, 128);
        draw.draw_rect(p.x-1, p.y-1, 3, 3);
        body_.move(body_.pos()+body_.vel());
        if (body_.pos().y > 260) {
            destroy();
        }
    }

    void init(const vec2f_t & pos) {
        body_.move(pos);
        body_.vel() = vec2f_t::normalize(vec2f_t{prng::grandfs(seed_), 2.f}) * 2.7f;
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
        , body_(surge_.spatial_, vec2f_t{160, 200}, 6, this)
        , speed_(3.f)
        , alive_(true)
    {
        assert(service);
        fsm_.push_back(&obj_player_t::fsm_spawn);
        ref_.dec();
        defaults();
    }

    void shoot() {
        if (shoot_timer_.deltai()) {
            shoot_timer_.reset();
            object_ref_t bullet = surge_.factory_.create(e_bullet, &surge_);
            prng::seed_t seed = time_func();
            bullet->cast<obj_bullet_t>().init(body_.pos(),
                                              vec2f_t{prng::randfs(seed) * 0.1f, -8.f});
        }
    }

    void move() {
        const uint8_t * key = SDL_GetKeyboardState(nullptr);
        vec2f_t v = {0.f, 0.f};
        if (key[SDL_SCANCODE_LEFT]) {
            if (body_.pos().x > 32)
                v.x -= speed_;
        }
        if (key[SDL_SCANCODE_RIGHT]) {
            if (body_.pos().x < (320-32))
                v.x += speed_;
        }
        if (key[SDL_SCANCODE_X]) {
            shoot();
        }
        body_.move(body_.pos()+v);
    }

    void draw() {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(192, 192, 192);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
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
        }
        return false;
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            surge_.player_.dispose();
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_enemy_t: public object_ex_t<e_enemy, obj_enemy_t>
{
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
            prng::seed_t seed = time_func();
            if (prng::rand_chance(seed, 10)) {
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
        draw.draw_colour(256, 128, 64);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
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
            dive_timer_.step();
            if (prng::rand_chance(seed_, 20)) {
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
        vec2f_t & vel = body_.vel();
        vel += (vec2f_t{0.f, 0.2f} + vec2f_t{prng::grandfs(seed_), prng::grandfs(seed_)}) * 0.3f;
        vel = vec2f_t::normalize(vel) * 1.5f;
        body_.move(body_.pos() + vel);
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
        health_ = health;
    }

    void destroy() {
        if (alive_) {
            alive_ = false;
            ref_.dec();
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

        prng::seed_t temp = seed_;

        e->cast<obj_enemy_t>().init(pos,
                                    (pos.x / 100.f + pos.y / 80.f) * seed,
                                    prng::bitmix(temp) % s_splines.size(),
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
surge_t::surge_t(draw_t & draw)
    : draw_(draw)
    , difficulty_(1)
{
    factory_.add_creator<obj_player_t>();
    factory_.add_creator<obj_enemy_t>();
    factory_.add_creator<obj_bullet_t>();
    factory_.add_creator<obj_wave_t>();
    factory_.add_creator<obj_explode_t>();
    factory_.add_creator<obj_powerup_t>();
    factory_.add_creator<obj_stars_t>();
    factory_.add_creator<obj_bomb_t>();

    stars_ = factory_.create<obj_stars_t>(this);
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
void surge_t::tick()
{
    for (auto itt = enemy_.begin(); itt!=enemy_.end();) {
        object_ref_t & ref = *itt;
        if (ref.valid() && !ref->cast<obj_enemy_t>().alive_) {
            ref.dispose();
            itt = enemy_.erase(itt);
        }
        else {
            ++itt;
        }
    }

    if (enemy_.size()==0&&!wave_.valid()) {
        stars_->cast<obj_stars_t>().boost();
        wave_ = factory_.create<obj_wave_t>(this);
        ++difficulty_;
    }

    if (!player_.valid()) {
        player_ = factory_.create<obj_player_t>(this);
    }

    factory_.tick();
    factory_.collect();
}

void surge_t::init()
{
}
