#pragma once
#include "common.h"

struct player_anim_t {

    anim::sheet_t sheet_;

    anim::sequence_t run_;
    anim::sequence_t stand_;
    anim::sequence_t jump_;
    anim::sequence_t fall_;
    anim::sequence_t slide_;
    anim::sequence_t skid_;

    anim::anim_controller_t controller_;

    bitmap_t bitmap_;

    bool xflip_;

    enum {
        e_foot_fall,
        e_slide_loop
    };

    player_anim_t();

    void render(const vec2i_t & pos, draw_ex_t & draw_);

    void tick(int32_t delta);

    void set_x_dir(float dir);

    void play(anim::sequence_t & seq);

    bool event_foot_fall();
    bool event_slide_loop();

    const blit_info_t & blit_info() const {
        return blit_;
    }

protected:

    blit_info_t blit_;

    bool foot_fall_;
    bool slide_loop_;
};

struct player_splash_t : public object_ex_t<e_object_player_splash, player_splash_t> {
    static const uint32_t _ORDER = e_object_player_splash;

    draw_ex_t & draw_;
    blit_info_t info_;
    int32_t count_;

    player_splash_t(object_service_t svc)
        : draw_(static_cast<service_t*>(svc)->draw_)
        , count_(5)
    {
        order_ = _ORDER;
    }

    void init(const blit_info_t & info) {
        info_ = info;
        info_.type_ = e_blit_gliss;
    }

    virtual void tick() override {
        if (--count_ == 0) {
            ref_.dec();
            alive_ = false;
        }
        draw_.key_ = 0xff00ff;
        draw_.blit<true>(info_);
    }
};

struct player_shadow_t : public object_ex_t<e_object_player_shadow, player_shadow_t> {
    static const uint32_t _ORDER = e_object_player_shadow;

    service_t & service_;
    player_shadow_t(object_service_t service);
    virtual void tick() override;
};

struct player_t : public object_ex_t<e_object_player, player_t> {
    static const uint32_t _ORDER = e_object_player;

    const int32_t _WIDTH = 4;
    const int32_t _HEIGHT = 10;

    typedef fsm_t<player_t> player_fsm_t;
    typedef player_fsm_t::fsm_state_t player_state_t;

    service_t & service_;
    vec2f_t pos_[2];
    float dx_;

    player_fsm_t fsm_;
    player_state_t fsm_state_air_; // falling jumping
    player_state_t fsm_state_run_; // running on ground
    player_state_t fsm_state_slide_; // wall slide

    player_anim_t anim_;

    object_ref_t shadow_;

    player_t(object_service_t s);

    rectf_t bound() const;

    // bound swept in size by velocity
    rectf_t swept_bound() const;

    void tick_run();

    void tick_air();

    void tick_slide();

    void jump();

    vec2f_t get_pos() const;

    vec2f_t get_velocity() const;

    void move(float dx);

    virtual void tick() override;
};
