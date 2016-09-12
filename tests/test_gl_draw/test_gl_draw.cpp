#include <array>
#include <SDL/SDL.h>

#include "../../framework_gl_draw/gl_draw.h"

namespace {
    SDL_Surface * screen_;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    screen_ = SDL_SetVideoMode(320*2, 240*2, 32, SDL_OPENGL);
    if (!screen_) {
        return false;
    }
    return true;
}

void test_simple() {
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 1> tests = {{
    TEST(test_simple)
}};

struct main_t {

    main_t() 
        : test_  (0    )
        , pause_ (false)
        , active_(true )
    {
    }

    void tick_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type==SDL_KEYDOWN) {
                if (event.key.keysym.sym==SDLK_LEFT) {
                    --test_;
                }
                if (event.key.keysym.sym==SDLK_RIGHT) {
                    ++test_;
                }
                if (event.key.keysym.sym==SDLK_SPACE) {
                    pause_ ^= true;
                }
                if (event.key.keysym.sym==SDLK_ESCAPE) {
                    active_ = false;
                }
            }
            if (event.type==SDL_QUIT) {
                active_ = false;
            }
        }
    }

    int main(const int argc, char *args[]) {
        if (!init()) {
            return 1;
        }
        while (active_) {
            tick_events();
            if (!pause_) {
                const auto test = tests[test_ % tests.size()];
                test.func_();
                SDL_GL_SwapBuffers();
            }
            SDL_Delay(1000/25);
        }
        return 0;
    }

    uint32_t test_;
    bool active_;
    bool pause_;
};

int main(int argc, char ** args) {
    main_t main_class;
    return main_class.main(argc, args);
}
