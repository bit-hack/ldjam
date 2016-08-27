#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "../framework/timer.h"
#include "../framework/const.h"

#include "draw.h"
#include "surge.h"


uint64_t time_func() {
    return SDL_GetTicks();
}


struct game_t {
    
    static const int C_FPS = 30;

    draw_t draw_;
    delta_time_t frame_timer_;
    surge_t surge_;

    game_t()
        : draw_()
        , frame_timer_(time_func, 1000/C_FPS)
        , surge_(draw_)
    {
    }

    bool poll_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type==SDL_QUIT) {
                return false;
            }
            if (event.type==SDL_KEYDOWN) {
                if (event.key.keysym.scancode==SDL_SCANCODE_ESCAPE) {
                    return false;
                }
            }
        }
        return true;
    }

    bool tick() {
        surge_.tick();
        return true;
    }

    int main()
    {
        SDL_SetMainReady();

        if (SDL_Init(SDL_INIT_VIDEO)) {
            return 1;
        }

        if (!draw_.init(320, 240)) {
            return 1;
        }

        surge_.init();

        while (true) {

            if (!poll_events()) {
                return false;
            }

            if (uint64_t i = frame_timer_.deltai()) {
                if (i>C_FPS) {
                    // timer has exploded
                    frame_timer_.reset();
                }
                else {
                    frame_timer_.step();
                }
                tick();
                draw_.present();
            }
            else {
                // dont burn the CPU
                SDL_Delay(1);
            }
        }

        return 0;
    }
};

int main(const int argc, const char *args[]) {
    game_t game;
    return game.main();
}
