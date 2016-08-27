#include "../framework/timer.h"
#include "../framework/vec2.h"
#include "../framework/const.h"

#include "surge.h"

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
extern uint64_t time_func();

typedef std::array<vec2f_t, 9> spline_t;

namespace {
std::array<spline_t, 1> s_splines = {
    spline_t{{
        { 160, -64 },
        { 160, -32 },
        { 220, 160 },
        { 260, 130 },
        { 160, 100 },
        {  50, 130 },
        { 100, 160 },
        { 160,  60 },
        { 160,  60 },
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
    e_stars
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
struct obj_explode_t: public object_ex_t<e_explode, obj_explode_t>
{
    surge_t & surge_;
    float age_;
    bool alive_;

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
        if (age_ >= 64.f) {
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
    
    void init(const vec2f_t & pos) {
        prng::seed_t seed = prng::seed_t(time_func());
        for (particle_t & p:part_) {
            p.pos_ = pos;
            p.vel_ = vec2f_t{ prng::grandfs(seed), prng::grandfs(seed) };

            float v = (p.vel_.x*p.vel_.x+p.vel_.y*p.vel_.y);
            p.size_ = 1.f / (v + (1.f - v) * .1f);
        }
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
    body_t body_;
    vec2f_t dir_;
    bool alive_;

    obj_bullet_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(vec2f_t{160, 120}, 4, this)
        , alive_(true)
    {
        assert(service);
    }

    virtual ~obj_bullet_t() override {
        surge_.spatial_.remove(&body_);
    }

    virtual void tick() override {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(128, 192, 255);
        draw.draw_rect(p.x-1, p.y-1, 3, 3);
        surge_.spatial_.move(&body_, body_.pos() + dir_);
        if (p.y < -32.f) {
            destroy();
        }
    }

    void init(const vec2f_t & pos, const vec2f_t & dir) {
        dir_ = dir;
        surge_.spatial_.insert(&body_);
        surge_.spatial_.move(&body_, pos + dir);
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
    body_t body_;
    bool alive_;

    obj_powerup_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , alive_(true)
        , body_(vec2f_t{160, 200}, 6, this)
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
        surge_.spatial_.move(&body_, body_.pos()+vec2f_t{ 0.f, 1.f });

        if (body_.pos().y > 260) {
            destroy();
        }
    }

    void init(const vec2f_t & pos) {
        surge_.spatial_.insert(&body_);
        surge_.spatial_.move(&body_, pos);
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
    body_t body_;
    float speed_;

    obj_player_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , spawn_timer_(time_func, 4)
        , shoot_timer_(time_func, 120*4)
        , body_(vec2f_t{160, 200}, 6, this)
        , speed_(3.f)
    {
        assert(service);
        fsm_.push_back(&obj_player_t::fsm_spawn);
        surge_.spatial_.insert(&body_);
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
        surge_.spatial_.move(&body_, body_.pos() + v);
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
    }

    void fsm_spawn() {
        if (spawn_timer_.deltai()>120) {
            fsm_.push_back(&obj_player_t::fsm_tick);
        }
        if (spawn_timer_.deltai()&1) {
            draw();
        }
        move();
    }

    virtual void tick() override {
        assert(fsm_.size());
        state_t func = fsm_.back();
        (*this.*func)();
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_enemy_t : public object_ex_t<e_enemy, obj_enemy_t>
{
    typedef void (obj_enemy_t::*state_t)();

    std::vector<state_t> fsm_;
    surge_t & surge_;
    vec2f_t ideal_pos_;
    body_t body_;
    bool alive_;
    float dt_;
    spline_t * spline_;
    float track_;

    obj_enemy_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(vec2f_t{160, 120}, 4, this)
        , spline_(nullptr)
        , track_(0.f)
    {
        assert(service);
        fsm_.push_back(&obj_enemy_t::fsm_float);
        fsm_.push_back(&obj_enemy_t::fsm_spline);
    }
    
    void on_death() {
        destroy();
        // sleep feels cool
        SDL_Delay(50);
        // screen shake feels awesome
        surge_.draw_.shake_ += 4.f;
        object_ref_t exp = surge_.factory_.create<obj_explode_t>(&surge_);
        exp->cast<obj_explode_t>().init(body_.pos());
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
                    on_death();
                    return true;
                }
            }
        }
        return false;
    }

    void draw() {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(256, 128, 64);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
    }

    vec2f_t calc_pos() const {
        const vec2f_t center = {160.f, 80.f};
        return ideal_pos_+(center-ideal_pos_) * (sinf(dt_)*0.1f);
    }

    void fsm_float() {
        if (!alive_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        // float movement
        {
            dt_ = dt_<0.f ? (dt_+C_2PI-0.1f) : dt_-0.1f;
            surge_.spatial_.move(&body_, calc_pos());
        }

        draw();
    }

    void fsm_spline() {
        if (!alive_ || !spline_) {
            return;
        }
        if (check_collide()) {
            return;
        }
        //
        if (int(track_+1)>spline_->size()) {
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
            surge_.spatial_.move(&body_, p);
            track_ += 0.04f;
        }
        draw();
    }

    virtual void tick() override {
        assert(fsm_.size());
        state_t func = fsm_.back();
        // dispatch to finite state machine
        (*this.*func)();
    }

    void init(const vec2f_t & p, float seed, uint32_t spline) {
        ideal_pos_ = p;
        dt_ = seed;
        surge_.spatial_.move(&body_, ideal_pos_);
        spline_ = &s_splines[spline];
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
        , spawn_timer_(time_func, 80)
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
                                    0);
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
struct obj_stars_t : public object_ex_t<e_stars, obj_stars_t>
{
    static const size_t C_STARS = 256;

    std::array<vec2f_t, C_STARS> pos_;
    std::array<float, C_STARS> depth_;

    prng::seed_t seed_;
    
    surge_t & surge_;

    obj_stars_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , seed_(0xcafe)
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
        for (size_t i = 0; i<C_STARS; ++i) {
            vec2f_t & pos = pos_[i];
            float & depth = depth_[i];
            pos.y += 3.f + 3.f * depth;
            if (pos.y>260) {
                pos.y = -32;
                pos.x = prng::randllu(seed_) % 320;
            }
            
            auto draw = surge_.draw_;
            draw.draw_colour(0x10 + 32.f * depth, 0x10 + 48.f * depth, 0x10 + 64 * depth);
            draw.draw_rect(pos.x-1, pos.y-1, 2, 2);
        }
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
surge_t::surge_t(draw_t & draw)
    : draw_(draw)
    , difficulty_(1.f)
{
    factory_.add_creator<obj_player_t>();
    factory_.add_creator<obj_enemy_t>();
    factory_.add_creator<obj_bullet_t>();
    factory_.add_creator<obj_wave_t>();
    factory_.add_creator<obj_explode_t>();
    factory_.add_creator<obj_powerup_t>();
    factory_.add_creator<obj_stars_t>();

    factory_.create<obj_stars_t>(this);
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
        wave_ = factory_.create<obj_wave_t>(this);
        difficulty_ += 1.f;
    }

    factory_.tick();
    factory_.collect();
}

void surge_t::init()
{
    player_ = factory_.create<obj_player_t>(this);
}
