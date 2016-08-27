#pragma once

#include <cassert>
#include <cstdint>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "../framework/vec2.h"

struct draw_t
{
    SDL_Window * window_;
    SDL_Renderer * render_;



    draw_t()
        : window_(nullptr)
        , render_(nullptr)
    {
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
    }

    void draw_colour(uint8_t r, uint8_t g, uint8_t b) {
        SDL_SetRenderDrawColor(render_, r, g, b, 0);
    }

    void draw_rect(int x, int y, int w, int h) {
        SDL_Rect r = { x, y, w, h };
        SDL_RenderDrawRect(render_, &r);
    }
};
