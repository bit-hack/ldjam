#pragma once

#include <cassert>
#include <cstdint>
#include <array>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "../framework/vec2.h"
#include "../framework/random.h"
#include "../framework/timer.h"

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
extern uint64_t time_func();

struct draw_t
{
    SDL_Window * window_;
    SDL_Renderer * render_;
    delta_time_t shake_time_;
    prng::seed_t seed_;

    float shake_;
    std::array<vec2f_t, 2> shake_offset_;

    draw_t()
        : window_(nullptr)
        , render_(nullptr)
        , shake_(0.f)
        , shake_time_(time_func, 30)
        , seed_(0xbeef)
    {
        shake_offset_[0] = vec2f_t{
            prng::randfs(seed_),
            prng::randfs(seed_)};
        shake_offset_[1] = vec2f_t{
            prng::randfs(seed_),
            prng::randfs(seed_)};
    }

    bool init(const uint32_t w, const uint32_t h) {
        assert(!(render_ || window_));
        if (SDL_CreateWindowAndRenderer(w*2, h*2, 0, &window_, &render_)) {
            return false;
        }
        if (!(window_ && render_)) {
            return false;
        }
        SDL_RenderSetLogicalSize(render_, w, h);
        return true;
    }

    void present() {
        assert(render_);
        SDL_RenderPresent(render_);
        SDL_SetRenderDrawColor(render_, 0x10, 0x10, 0x10, 0);
        SDL_RenderClear(render_);

        shake_ = shake_ <= 0.f ? 0.f : (shake_ * 0.8f - 0.05f);

        while (shake_time_.deltai()) {
            shake_time_.step();
            shake_offset_[0] = shake_offset_[1];
            shake_offset_[1] = vec2f_t{
                prng::randfs(seed_),
                prng::randfs(seed_)};
        }

    }

    void draw_colour(uint8_t r, uint8_t g, uint8_t b) {
        SDL_SetRenderDrawColor(render_, r, g, b, 0);
    }

    void draw_rect(int x, int y, int w, int h) {

        vec2f_t offs = shake_offset_[0] + (shake_offset_[1]-shake_offset_[0]) * shake_time_.deltaf();
        offs *= shake_;

        SDL_Rect r = { x + offs.x, y + offs.y, w, h };
        SDL_RenderDrawRect(render_, &r);
    }
};
