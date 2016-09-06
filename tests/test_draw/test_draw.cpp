#include <array>
#include <SDL/SDL.h>

#include "../../framework_core/random.h"
#include "../../framework_draw/draw.h"

namespace {
    SDL_Surface * screen_;
    draw_t draw_;
    bitmap_t bitmap_;
    bitmap_t font_;
    random_t random_(0x12345);
    bitmap_t sprite_;
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

void test_circles() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.circle(p, random_.rand<uint32_t>() % 64);
    }
}

void test_lines() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2f_t p0 = vec2f_t {
            float(random_.rand<uint32_t>() % 320u),
            float(random_.rand<uint32_t>() % 240u)};
        const vec2f_t p1 = vec2f_t {
            float(random_.rand<uint32_t>() % 320u),
            float(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.line(p0, p1);
    }
}

void test_rect() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p0 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        const vec2i_t p1 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.rect(recti_t{p0.x, p0.y, p1.x, p1.y});
    }
}

void test_plot() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<1000; ++i) {
        const vec2i_t p0 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.plot(p0);
    }
}

void test_blit() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    if (!sprite_.valid()) {
        if (!bitmap_t::load("assets/sprite1.bmp", sprite_)) {
            return;
        }
    }
    draw_.colour_ = 0x0;
    for (int i=0; i<100; ++i) {
        blit_info_t info;
        info.bitmap_ = &sprite_;
        info.dst_pos_ = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        info.src_rect_ = recti_t {0, 0, 31, 31};
        info.h_flip_ = false;
        info.type_ = e_blit_key;
        draw_.blit(info);
    }
}

void test_font() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    if (!font_.valid()) {
        if (!bitmap_t::load("assets/font.bmp", font_)) {
            return;
        }
    }
    font_t font;
    font.bitmap_ = &font_;
    font.cellw_ = 9;
    font.cellh_ = 14;
    font.spacing_ = 9;
    vec2i_t pos {32, 32};
    draw_.printf(font, pos, "Hello World");
}

void test_triangle() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2f_t p0 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        const vec2f_t p1 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        const vec2f_t p2 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        draw_.colour_ = random_.rand<uint32_t>();
        draw_.triangle(p0, p1, p2);
    }
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 7> tests = {{
    TEST(test_font),
    TEST(test_blit),
    TEST(test_circles),
    TEST(test_lines),
    TEST(test_plot),
    TEST(test_rect),
    TEST(test_triangle)
}};

int main(const int argc, char *args[]) {
    if (!init()) {
        return 1;
    }
    uint32_t test_index = 0;
    bool pause = false;
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
                if (event.key.keysym.sym == SDLK_SPACE) {
                    pause ^= true;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active = false;
                }
            }
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }
        if (!pause) {
            const auto test = tests[test_index % tests.size()];
            test.func_();
            draw_.render_2x(screen_->pixels, screen_->pitch / 4);
            SDL_Flip(screen_);
        }
        SDL_Delay(1000/25);
    }
    return 0;
}
