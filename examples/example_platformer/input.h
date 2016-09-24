#pragma once
#include <cstdint>
#include <array>

#include <SDL/SDL.h>

#include "../../framework_core/vec2.h"

struct gamepad_t {
    static const uint32_t C_BUTTONS = 2;

    gamepad_t() {
        button_.fill(false);
        delta_.fill(false);
        axis_ = vec2f_t{0.f, 0.f};
    }

    virtual ~gamepad_t() {}

    enum {
        e_button_x,
        e_button_z,
    };

    vec2f_t axis_;
    std::array<bool, C_BUTTONS> button_;
    std::array<bool, C_BUTTONS> delta_;

    virtual void tick() = 0;
};

struct gamepad_key_t : public gamepad_t {

    gamepad_key_t()
        : gamepad_t()
    {
    }

    virtual void tick() {
        uint8_t * keys = SDL_GetKeyState(nullptr);
        if (keys) {
            axis_ = vec2f_t{0.f, 0.f};
            axis_.x -= keys[SDLK_LEFT]  ? 1.f : 0.f;
            axis_.x += keys[SDLK_RIGHT] ? 1.f : 0.f;
            axis_.y -= keys[SDLK_UP]    ? 1.f : 0.f;
            axis_.y += keys[SDLK_DOWN]  ? 1.f : 0.f;
            const bool bx_ = keys[SDLK_x] != 0;
            const bool bz_ = keys[SDLK_z] != 0;
            delta_[e_button_x]  = bx_ ^ button_[e_button_x];
            delta_[e_button_z]  = bz_ ^ button_[e_button_z];
            button_[e_button_x] = bx_;
            button_[e_button_z] = bz_;
        }
    }
};

struct gamepad_joy_t : public gamepad_t {

    static const uint32_t X_BUTTON = 2;
    static const uint32_t Z_BUTTON = 1;

    SDL_Joystick * joy_;

    gamepad_joy_t()
        : gamepad_t()
        , joy_(nullptr)
    {
    }

    virtual ~gamepad_joy_t() {
        close();
    }

    bool open(int32_t id) {
        if (SDL_NumJoysticks()) {
            joy_ = SDL_JoystickOpen(id);
        }
        return joy_ != nullptr;
    }

    bool valid() const {
        return joy_ != nullptr;
    }

    void close() {
        if (joy_) {
            SDL_JoystickClose(joy_);
            joy_ = nullptr;
        }
    }

    virtual void tick() {
        if (!joy_) {
            return;
        }

        const int16_t dx = SDL_JoystickGetAxis(joy_, 0);
        const int16_t dy = SDL_JoystickGetAxis(joy_, 1);

        axis_ = vec2f_t{0.f, 0.f};

        axis_.x -= dx < -64 ? 1.f : 0.f;
        axis_.x += dx > +64 ? 1.f : 0.f;

        axis_.y -= dy < -64 ? 1.f : 0.f;
        axis_.y += dy > +64 ? 1.f : 0.f;

        const bool bx_ = (SDL_JoystickGetButton(joy_, X_BUTTON) != 0);
        const bool bz_ = (SDL_JoystickGetButton(joy_, Z_BUTTON) != 0);

        delta_[e_button_x]  = bx_ ^ button_[e_button_x];
        delta_[e_button_z]  = bz_ ^ button_[e_button_z];

        button_[e_button_x] = bx_;
        button_[e_button_z] = bz_;
    }
};
