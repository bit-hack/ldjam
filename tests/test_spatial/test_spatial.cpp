#include <SDL/SDL.h>
#include <array>
#include <list>
#include <set>

#include "../../framework_draw/draw.h"
#include "../../framework_core/random.h"
#include "../../framework_spatial/space_hash.h"

using namespace tengu;

struct app_t {
    SDL_Surface* window_;
    int32_t w_, h_;

    draw_t draw_;
    bitmap_t target_;

    bool init(uint32_t w, uint32_t h) {
        if (SDL_Init(SDL_INIT_VIDEO))
            return false;
        window_ = SDL_SetVideoMode(w, h, 32, 0);
        if (!window_)
            return false;
        w_ = w;
        h_ = h;
        if (!target_.create(vec2i_t{int32_t(w), int32_t(h)})) {
            return false;
        }
        draw_.set_target(target_);
        return true;
    }

    bool tick() {
        draw_.render_1x(window_->pixels, window_->pitch/4);
        SDL_Flip(window_);
        draw_.colour_ = 0x202020;
        draw_.clear();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
            }
        }
        return true;
    }
};

struct test_t {
    virtual void tick() = 0;
    virtual void init(draw_t & draw) = 0;
};

struct test_shash_t : test_t {

    static const uint32_t num_objects = 200;
    std::list<body_t*> list_;

    random_t rand_;
    spatial_t hash_;
    body_pair_set_t set_;
    tengu::draw_t * draw_;

    test_shash_t()
        : rand_(SDL_GetTicks())
        , hash_()
    {
    }

    virtual void init(draw_t & draw) override {
        draw_ = &draw;
        for (int i = 0; i < num_objects; ++i) {
            float out[2];
            rand_.vrand2d(out);
            const vec2f_t p = { rand_.randfu() * 512,
                                rand_.randfu() * 512 };
            const float r = rand_.randfu() * 16 + 4;
            body_t* obj = new body_t(p, r, nullptr);
            obj->vel_.x = out[0];
            obj->vel_.y = out[1];
            list_.push_front(obj);
            hash_.insert(obj);
        }
    }

    void draw_occupancy() {
        for (int y = 0; y < 512 / spatial_t::width; ++y) {
            for (int x = 0; x < 512 / spatial_t::width; ++x) {
                draw_->colour_ = uint32_t(hash_.dbg_ocupancy(x, y) * 8);
                draw_->rect(recti_t(x * 32, y * 32, 32, 32,
                                    recti_t::e_relative));
            }
        }
    }

    void resolve_overlap() {
        hash_.query_collisions(set_);
        for (auto pair : set_) {
            body_t* a = pair.first;
            body_t* b = pair.second;
            float nx = b->pos().x - a->pos().x;
            float ny = b->pos().y - a->pos().y;
            float nl = sqrtf(nx * nx + ny * ny);
            if (nl < 0.0001f)
                continue;
            nx /= nl;
            ny /= nl;
            const float os = ((b->radius() + a->radius()) - nl) * .5f;
            vec2f_t v1{ -nx * os, -ny * os };
            hash_.move(a, a->pos() + v1);
            vec2f_t v2{ +nx * os, +ny * os };
            hash_.move(b, b->pos() + v2);
        }
    }

    void draw_agents() {
        body_set_t obj_set;
        float p1 = 128, p2 = 128 + 256;
        hash_.query_rect(vec2f_t{ p1, p1 },
                         vec2f_t{ p2, p2 }, obj_set);
        for (auto obj : obj_set) {
            vec2f_t p = obj->pos();
            draw_->colour_ = 0x3377aa;
            draw_->circle(vec2i_t{int32_t(p.x),
                                  int32_t(p.y)},
                          int32_t(obj->radius()));
        }
        for (auto obj : list_) {
            float speed = 0.5f;
            uint32_t colour = 0x404040;
            if (obj_set[obj]) {
                speed = 1.f;
                colour = 0x00ff00;
            }
            draw_->colour_ = colour;
            draw_->circle(vec2i_t{int32_t(obj->pos().x),
                                  int32_t(obj->pos().y)},
                          int32_t(obj->radius()));
            vec2f_t v1 = { obj->vel_.x * speed,
                           obj->vel_.y * speed };
            hash_.move(obj, obj->pos() + v1);

            // bound the object to the world
            obj->vel_.x *= (obj->pos().x < 000 && obj->vel_.x < 0) ? -1 : 1;
            obj->vel_.x *= (obj->pos().x > 512 && obj->vel_.x > 0) ? -1 : 1;
            obj->vel_.y *= (obj->pos().y < 000 && obj->vel_.y < 0) ? -1 : 1;
            obj->vel_.y *= (obj->pos().y > 512 && obj->vel_.y > 0) ? -1 : 1;
        }
    }

    void draw_world() {
        for (int i = 0; i < 512; i += spatial_t::width) {
            draw_->colour_ = 0x202020;
            float j = i;
            draw_->line(vec2f_t{0.f, j}, vec2f_t{512.f, j});
            draw_->line(vec2f_t{j, 0.f}, vec2f_t{j, 512.f});
        }
        draw_->colour_ = 0x203090;
        draw_->line(vec2f_t{0, 128}, vec2f_t{512, 128});
        draw_->line(vec2f_t{0, 384}, vec2f_t{512, 384});
        draw_->line(vec2f_t{128, 0}, vec2f_t{128, 512});
        draw_->line(vec2f_t{384, 0}, vec2f_t{384, 512});
    }

    virtual void tick() override {
        draw_occupancy();
        resolve_overlap();
        draw_agents();
        draw_world();
        set_.clear();
    }
};

// test list
std::array<test_t *, 1> s_tests = {
    new test_shash_t
};

int main(int argc, char *args[]) {
    // setup app
    app_t app;
    if (!app.init(512, 512)) {
        return false;
    }
    // init all tests
    for (test_t * test : s_tests) {
        test->init(app.draw_);
    }
    // main loop
    while (app.tick()) {
        s_tests[0]->tick();
        SDL_Delay(0);
    }
    return 0;
}
