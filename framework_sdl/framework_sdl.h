#pragma once
#define _SDL_main_h
#include <SDL/SDL.h>

#include "../framework_core/event.h"
#include "../framework_core/timer.h"

namespace tengu {

// frame rate limiter

struct sdl_frame_timer_t {

    sdl_frame_timer_t()
        : fps_(25.f /* PAL */)
        , timer_(get_ticks, uint32_t(1000.f / fps_))
    {
    }

    void set_fps(float fps)
    {
        timer_.period_ = uint32_t(1000.f / (fps_ = fps));
    }

    timing_t<uint32_t>& timer()
    {
        return timer_;
    }

protected:
    static uint32_t get_ticks()
    {
        return SDL_GetTicks();
    }

    float fps_;
    timing_t<uint32_t> timer_;
};

struct sdl_event_quit_t : public event_t {
    sdl_event_quit_t()
        : event_t(SDL_QUIT)
    {
    }
};

struct sdl_framework_t {

    bool tick();

    event_stream_t event_stream;
};

} // namespace tengu
