#pragma once

#include "../../framework_core/common.h"
#include "../../framework_core/vec2.h"

// todo:
//      app control with update and render
//      joystick, mouse and key abstraction

struct sdl_input_t {

    struct sdl_mouse_state_t {
        tengu::vec2i_t pos_;
    };

    bool get_mouse(sdl_mouse_state_t & out);
};

struct sdl_app_t;

struct sdl_framework_t {

    // 
    bool init();

    // call from the application main loop
    bool update();
    bool set_tick_rate();
    bool shutdown();
    bool is_active() const;

    // api wrappers
    sdl_input_t input_;

    // user provided app instance
    sdl_app_t * app_;
};

struct sdl_app_t {

    sdl_framework_t & sdl_;

    sdl_app_t(sdl_framework_t & sdl)
        : sdl_(sdl) {
    }

    virtual bool on_init() {}

    virtual bool on_update() {}

    virtual bool on_render(float delta) {}

    virtual bool on_free() {}
};
