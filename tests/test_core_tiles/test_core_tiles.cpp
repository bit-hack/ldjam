#include <array>
#define _SDL_main_h
#include <SDL/SDL.h>

#include "../../framework_core/tiles.h"
#include "../../framework_core/random.h"

#include "../../framework_draw/draw.h"

namespace {
    const int32_t _TILE_SIZE = 32;
    const int32_t _MAP_WIDTH = 640 / _TILE_SIZE;
    const int32_t _MAP_HEIGHT = 480 / _TILE_SIZE;

    collision_map_t map_ =
            collision_map_t(
                    vec2i_t{_MAP_WIDTH, _MAP_HEIGHT},
                    vec2i_t{_TILE_SIZE, _TILE_SIZE});
    SDL_Surface * screen_ = nullptr;
    draw_t draw_;
    bitmap_t target_;
    random_t random_(0x12345);
} // namespace {}

void seed() {
    const int32_t count = (_MAP_WIDTH * _MAP_HEIGHT) / 10;
    map_.clear(0);
    for (int32_t i=0; i<count; ++i) {
        map_.get(vec2i_t{
            random_.rand_range(0, _MAP_WIDTH),
            random_.rand_range(0, _MAP_HEIGHT)}) = e_tile_solid;
    }
    map_.preprocess();
}

void draw_map() {
    draw_.colour_ = 0x204060;
    draw_.clear();
    for (int32_t y=0; y<_MAP_HEIGHT; ++y) {
        for (int32_t x=0; x<_MAP_WIDTH; ++x) {
            vec2i_t tp = (vec2i_t{x, y}) * _TILE_SIZE;

            if (map_.is_solid(vec2i_t{x, y})) {
                draw_.colour_ = 0x4060A0;
                draw_.rect(
                        recti_t(tp.x,
                                tp.y,
                                _TILE_SIZE,
                                _TILE_SIZE,
                                recti_t::e_relative));
            }

            draw_.colour_ = 0x202020;
            uint8_t tile = map_.get(vec2i_t{x,y});
            if (tile & e_tile_push_up)
                draw_.plot(tp + vec2i_t { 16, 2 });
            if (tile & e_tile_push_down)
                draw_.plot(tp + vec2i_t { 16, 30 });
            if (tile & e_tile_push_left)
                draw_.plot(tp + vec2i_t { 2,  16 });
            if (tile & e_tile_push_right)
                draw_.plot(tp + vec2i_t { 30, 16 });
        }
    }
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    screen_ = SDL_SetVideoMode(640, 480, 32, 0);
    if (!screen_) {
        return false;
    }
    if (!bitmap_t::create(640, 480, target_)) {
        return false;
    }
    if (!target_.valid()) {
        return false;
    }
    draw_.set_target(target_);
    seed();
    return true;
}

void test_collision() {

    int32_t mx, my;
    SDL_GetMouseState(&mx, &my);

    rectf_t rect {
        mx - 24.f, my - 24.f,
        mx + 24.f, my + 24.f
    };

    draw_.colour_ = 0xD08040;
    draw_.rect(recti_t(rect));

    vec2f_t res{0.f, 0.f};
    if (map_.collide(rect, res)) {
        rect.x0 += res.x;
        rect.y0 += res.y;
        rect.x1 += res.x;
        rect.y1 += res.y;

        draw_.colour_ = 0x004488;
        draw_.rect(recti_t{16, 16, 32, 32});
    }

    draw_.colour_ = 0x40A070;
    draw_.rect(recti_t(rect));
}

void test_collision_alt() {

    int32_t mx, my;
    SDL_GetMouseState(&mx, &my);

    rectf_t rect {
        mx - 24.f, my - 24.f,
        mx + 24.f, my + 24.f
    };

    draw_.colour_ = 0xD08040;
    draw_.rect(recti_t(rect));

    vec2f_t res{0.f, 0.f};
    if (map_.collide_alt(rect, res)) {
        rect.x0 += res.x;
        rect.y0 += res.y;
        rect.x1 += res.x;
        rect.y1 += res.y;

        draw_.colour_ = 0x004488;
        draw_.rect(recti_t{16, 16, 32, 32});
    }

    draw_.colour_ = 0x40A070;
    draw_.rect(recti_t(rect));
}

void test_collision_point() {
    int32_t mx, my;
    SDL_GetMouseState(&mx, &my);

    draw_.colour_ = 0x00ff00;

    vec2f_t res;
    if (map_.collide(vec2f_t{float(mx), float(my)}, res)) {
        mx += res.x;
        my += res.y;
        draw_.rect(recti_t{mx-1, my-1, mx+1, my+1});
    }
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 3> tests = {{
    TEST(test_collision),
    TEST(test_collision_alt),
    TEST(test_collision_point)
}};

int main(const int argc, char *args[]) {
    if (!init()) {
        return 1;
    }
    int32_t test_index = 0;
    bool active = true;
    while (active) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    seed();
                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    if (--test_index<0) {
                        test_index += int32_t(tests.size());
                    }
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    if (++test_index >= int32_t(tests.size())) {
                        test_index -= int32_t(tests.size());
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active = false;
                }
            }
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }

        // draw the base map
        draw_map();

        // run this specific test
        test_index %= tests.size();
        const auto test = tests[test_index];
        test.func_();

        for (int32_t i = 0; i<int32_t(tests.size()); ++i) {
            draw_.colour_ = i==test_index ? 0xffffff : 0x909090;
            draw_.rect(recti_t(1 + 4 * i, 1, 3, 4, recti_t::e_relative));
        }

        // upscale and flip target to screen
        draw_.render_1x(screen_->pixels, screen_->pitch / 4);
        SDL_Flip(screen_);

        SDL_Delay(1000/25);
    }
    return 0;
}
