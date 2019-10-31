#include "framework_sdl.h"

namespace tengu {
bool sdl_framework_t::tick() {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: {
            sdl_event_quit_t e;
            event_stream.send(&e);
            break;
        }
        }
    }

    return true;
}
}// namespace tengu
