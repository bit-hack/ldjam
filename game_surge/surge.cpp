#include "../framework/timer.h"
#include "../framework/vec2.h"

#include "surge.h"

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
extern uint64_t time_func();

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
enum {
    e_player,
    e_enemy,
    e_bullet,
    e_wave,
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
        , shoot_timer_(time_func, 15)
        , body_(vec2f_t{160, 200}, 6)
        , speed_(2.f)
    {
        assert(service);
        fsm_.push_back(&obj_player_t::fsm_spawn);
        surge_.spatial_.insert(&body_);
    }

    ~obj_player_t() {

    }

    void shoot() {
        if (shoot_timer_.deltai()) {
            shoot_timer_.reset();
            surge_.factory_.create(e_bullet, &surge_);
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

    virtual void tick() {
        assert(fsm_.size());
        state_t func = fsm_.back();
        (*this.*func)();
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_enemy_t : public object_ex_t<e_enemy, obj_enemy_t>
{
    surge_t & surge_;
    body_t body_;

    obj_enemy_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
        , body_(vec2f_t{160, 120}, 4)
    {
        assert(service);
    }
    
    virtual void tick() {
        auto draw = surge_.draw_;
        const vec2f_t p = body_.pos();
        draw.draw_colour(256, 128, 64);
        draw.draw_rect(p.x-4, p.y-4, 8, 8);
    }

    void init(const vec2f_t & p) {
        surge_.spatial_.move(&body_, p);
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_bullet_t : public object_ex_t<e_bullet, obj_bullet_t>
{
    surge_t & surge_;

    obj_bullet_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
    {
        assert(service);
    }

    virtual void tick() {
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_wave_t : public object_ex_t<e_wave, obj_wave_t>
{
    surge_t & surge_;

    obj_wave_t(object_service_t service)
        : object_ex_t()
        , surge_(*static_cast<surge_t*>(service))
    {
        // dont keep our own ref
        this->ref_.dec();
        assert(service);
    }

    void place() {

        const int32_t C_COUNT = 24;
        const int32_t C_WIDTH = 8;
        const float C_SPACING = 32.f;
        const float C_OFFSET = (320-(C_WIDTH*C_SPACING)) / 2 + C_SPACING / 2;

        for (int i = 0; i<C_COUNT; ++i) {
            object_ref_t e = surge_.factory_.create<obj_enemy_t>(&surge_);
            e->cast<obj_enemy_t>().init(
                vec2f_t{ C_OFFSET + (i % C_WIDTH) * C_SPACING, 
                         C_OFFSET + (i / C_WIDTH) * C_SPACING });

            surge_.enemy_.push_back(e);
        }
    }

    virtual void tick() {
        place();
        surge_.wave_.dispose();
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
surge_t::surge_t(draw_t & draw)
    : draw_(draw)
{
    factory_.add_creator<obj_player_t>();
    factory_.add_creator<obj_enemy_t>();
    factory_.add_creator<obj_bullet_t>();
    factory_.add_creator<obj_wave_t>();
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
void surge_t::tick()
{
    if (enemy_.size()==0&&!wave_.valid()) {
        wave_ = factory_.create<obj_wave_t>(this);
    }

    factory_.tick();
    factory_.collect();
}

void surge_t::init()
{
    player_ = factory_.create<obj_player_t>(this);
}
