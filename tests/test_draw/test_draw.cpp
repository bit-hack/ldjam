#include <SDL/SDL.h>

#include "../../framework_core/random.h"
#include "../../framework_draw/draw.h"

namespace {
    SDL_Surface * screen_;
    draw_t draw_;
    bitmap_t bitmap_;
    random_t random_(0x12345);
}

bool init() {

    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    screen_ = SDL_SetVideoMode(320*2, 240*2, 32, 0);
    if (!screen_) {
        return false;
    }
    if (!bitmap_t::create(320, 240, bitmap_)) {
        return false;
    }
    if (!bitmap_.valid()) {
        return false;
    }
    draw_.set_target(bitmap_);
    return true;
}

void render() {
    draw_.colour_ = 0x202020;
    draw_.clear();


    draw_.viewport(recti_t {32, 32, 320-32, 240-32});

    for (int i=0; i<100; ++i) {
        const vec2i_t p = vec2i_t {
            random_.randllu() % 320,
            random_.randllu() % 240};
        draw_.colour_ = random_.randllu();
        draw_.circle(p, random_.randllu() % 64);
    }
}

int main(const int argc, char *args[]) {

    if (!init()) {
        return 1;
    }

    bool active = true;
    while (active) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                active = false;
            }
        }

        render();

        draw_.render_2x(screen_->pixels, screen_->pitch/4);
        SDL_Flip(screen_);

        SDL_Delay(1000/25);
    }

    return 0;
}
