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
        e_foot_fall
    };

    player_anim_t();

    void render(const vec2i_t & pos, draw_t & draw_);

    void tick(int32_t delta);

    void set_x_dir(float dir);

    void play(anim::sequence_t & seq);
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

    player_anim_t anim_;

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
