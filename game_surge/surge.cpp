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
struct service_t
{
    surge_t & surge_;
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_player_t : public object_ex_t<e_player, obj_player_t>
{
    service_t & service_;

    typedef void (obj_player_t::*state_t)();
    std::vector<state_t> fsm_;
    vec2f_t pos_;

    delta_time_t timer_;

    obj_player_t(object_service_t service)
        : object_ex_t()
        , service_(*static_cast<service_t*>(service))
        , timer_(time_func, 4)
    {
        assert(service);
        fsm_.push_back(&obj_player_t::fsm_spawn);
    }

    void fsm_tick() {

    }

    void fsm_spawn() {
        if (timer_.deltai()>30) {
            fsm_.push_back(&obj_player_t::fsm_tick);
        }
        else {

        }
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
    service_t & service_;

    obj_enemy_t(object_service_t service)
        : object_ex_t()
        , service_(*static_cast<service_t*>(service))
    {
    }
    
    virtual void tick() {
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_bullet_t : public object_ex_t<e_bullet, obj_bullet_t>
{
    service_t & service_;

    obj_bullet_t(object_service_t service)
        : object_ex_t()
        , service_(*static_cast<service_t*>(service))
    {
    }

    virtual void tick() {
    }
};

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct obj_wave_t : public object_ex_t<e_wave, obj_wave_t>
{
    service_t & service_;

    obj_wave_t(object_service_t service)
        : object_ex_t()
        , service_(*static_cast<service_t*>(service))
    {
    }

    virtual void tick() {
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
        wave_ = factory_.create<obj_wave_t>();
    }

    factory_.tick();
    factory_.collect();
}

void surge_t::init()
{
    player_ = factory_.create<obj_player_t>();
}
