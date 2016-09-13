#include <array>
#define _SDL_main_h
#include <SDL/SDL.h>

#include "../../framework_core/geometry.h"
#include "../../framework_core/random.h"
#include "../../framework_draw/draw.h"

namespace {
    SDL_Surface * screen_ = nullptr;
    draw_t draw_;
    bitmap_t target_;
    random_t random_(0x12345);
    vec2f_t * picked_ = nullptr;
    std::array<vec2f_t, 4> point_;
} // namespace {}

void seed() {
    for (auto & p : point_) {
        p = vec2f_t {
            32.f + random_.randfu() * (320.f - 64.f),
            32.f + random_.randfu() * (240.f - 64.f) };
    }
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    screen_ = SDL_SetVideoMode(320*2, 240*2, 32, 0);
    if (!screen_) {
        return false;
    }
    if (!bitmap_t::create(320, 240, target_)) {
        return false;
    }
    if (!target_.valid()) {
        return false;
    }
    draw_.set_target(target_);
    seed();
    return true;
}

void test_line_project() {
    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);

    geometry::line_t<vec2f_t> line {point_[0], point_[1]};
    vec2f_t proj = line.project(point_[2]);

    draw_.colour_ = 0x806040;
    draw_.line(point_[2], proj);
}

void test_pline_project() {
    using namespace geometry;

    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);

    pline_t<vec2f_t> pline {point_[0], point_[1]};
    vec2f_t proj = pline.project(point_[2]);

    draw_.colour_ = 0x806040;
    draw_.line(point_[2], proj);
}

void test_edge_project() {
    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);

    geometry::edge_t<vec2f_t> edge {point_[0], point_[1]};
    vec2f_t proj = edge.project(point_[2]);

    draw_.colour_ = 0x806040;
    draw_.line(point_[2], proj);
}

void test_intersect_edge_edge() {
    using namespace geometry;

    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);
    draw_.line(point_[2], point_[3]);

    vec2f_t i;
    if (intersect(
            edge_t<vec2f_t>{point_[0], point_[1]},
            edge_t<vec2f_t>{point_[2], point_[3]},
            i)) {
        draw_.colour_ = 0x806040;
        draw_.circle(vec2i_t::cast(i), 3);
    }
}

void test_intersect_line_edge() {
    using namespace geometry;

    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);
    draw_.line(point_[2], point_[3]);

    vec2f_t i;
    if (intersect(
            line_t<vec2f_t>{point_[0], point_[1]},
            edge_t<vec2f_t>{point_[2], point_[3]},
            i)) {
        draw_.colour_ = 0x806040;
        draw_.circle(vec2i_t::cast(i), 3);
    }
}

void test_intersect_line_line() {
    using namespace geometry;

    draw_.colour_ = 0x406080;
    draw_.line(point_[0], point_[1]);
    draw_.line(point_[2], point_[3]);

    vec2f_t i;
    if (intersect(
            line_t<vec2f_t>{point_[0], point_[1]},
            line_t<vec2f_t>{point_[2], point_[3]},
            i)) {
        draw_.colour_ = 0x806040;
        draw_.circle(vec2i_t::cast(i), 3);
    }
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 6> tests = {{
    TEST(test_line_project),
    TEST(test_edge_project),
    TEST(test_pline_project),
    TEST(test_intersect_edge_edge),
    TEST(test_intersect_line_edge),
    TEST(test_intersect_line_line),
}};

int main(const int argc, char *args[]) {
    if (!init()) {
        return 1;
    }
    uint32_t test_index = 0;
    bool active = true;
    while (active) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    --test_index;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    ++test_index;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active = false;
                }
            }
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }

        // clear the screen
        draw_.colour_ = 0x202020;
        draw_.clear();

        // run this specific test
        const auto test = tests[test_index % tests.size()];
        test.func_();

        // update points
        int32_t mx, my;
        int32_t b = SDL_GetMouseState(&mx, &my);
        if (b & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            vec2f_t mouse { mx / 2.f, my / 2.f };
            if (!picked_) {
                picked_ = &point_[0];
                for (auto &p : point_) {
                    if (vec2f_t::distance(mouse, p) <
                        vec2f_t::distance(mouse, *picked_)) {
                        picked_ = &p;
                    }
                }
                if (vec2f_t::distance(mouse, *picked_) > 32.f) {
                    picked_ = nullptr;
                }
            }
            if (picked_) {
                *picked_ = mouse;
            }
        }
        else {
            picked_ = nullptr;
        }

        // draw the points
        for (const auto & p : point_) {
            draw_.colour_ = 0x204060;
            draw_.circle(vec2i_t{int32_t(p.x), int32_t(p.y)}, 4);
        }

        // upscale and flip target to screen
        draw_.render_2x(screen_->pixels, screen_->pitch / 4);
        SDL_Flip(screen_);

        SDL_Delay(1000/25);
    }
    return 0;
}
