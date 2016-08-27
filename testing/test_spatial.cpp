#define _SDL_main_h
#include <SDL/SDL.h>
#include <array>
#include <list>
#include <set>

#include "../framework/random.h"
#include "../framework/spatial.h"

struct app_t
{
    SDL_Surface * window_;
    int32_t w_, h_;

    bool init(uint32_t w, uint32_t h)
    {
        if (SDL_Init(SDL_INIT_VIDEO))
            return false;

        window_ = SDL_SetVideoMode(w, h, 32, 0);
        if (!window_)
            return false;

        w_ = w; h_ = h;

        return true;
    }

    bool tick()
    {
        SDL_Flip(window_);
        SDL_FillRect(window_, nullptr, 0x101010);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type==SDL_QUIT) {
                return false;
            }
        }
        return true;
    }

    void plot(int x, int y, uint32_t rgb) {
        if (x>=0&&x<w_&&y>=0&&y<h_) {
            uint32_t * pix = (uint32_t*)window_->pixels;
            pix[x+y*w_] = rgb;
        }
    }

    void circle(int x0, int y0, int radius, uint32_t rgb);

    void line(int x, int y, int x2, int y2, uint32_t rgb);

    void rect(int x, int y, int w, int h, uint32_t rgb);
};

void app_t::line(int x, int y, int x2, int y2, uint32_t rgb)
{
    bool yLonger = false;
    int incrementVal, endVal;
    int shortLen = y2 - y, longLen  = x2 - x;
    if (abs(shortLen) > abs(longLen)) {
        //std::swap<int>(shortLen, longLen);
        int temp = longLen;
        longLen = shortLen;
        shortLen = temp;
        yLonger = true;
    }
    endVal = longLen;
    if (longLen < 0) {
        incrementVal = -1;
        longLen = -longLen;
    }
    else {
        incrementVal = 1;
    }
    int decInc = (longLen==0) ? 0 : (shortLen<<16)/longLen;
    if (yLonger) {
        for (int i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(x + (j >> 16), y + i, rgb);
        }
    } else {
        for (int i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(x + i, y + (j >> 16), rgb);
        }
    }
}

void app_t::circle(int x0, int y0, int radius, uint32_t rgb)
{
    int x = 0, y = radius;
    int dp = 1-radius;

    plot(x0+y, y0, rgb);
    plot(x0-y, y0, rgb);
    plot(x0, y0+y, rgb);
    plot(x0, y0-y, rgb);

    do {
        if (dp < 0)
            dp = dp+2*(++x)+3;
        else
            dp = dp+2*(++x)-2*(--y)+5;

        plot(x0+x, y0+y, rgb);
        plot(x0-x, y0+y, rgb);
        plot(x0+x, y0-y, rgb);
        plot(x0-x, y0-y, rgb);
        plot(x0+y, y0+x, rgb);
        plot(x0-y, y0+x, rgb);
        plot(x0+y, y0-x, rgb);
        plot(x0-y, y0-x, rgb);

    } while (x < y);
}

void app_t::rect(int x, int y, int w, int h, uint32_t rgb)
{
    for (int ty = y; ty<y+h; ++ty) {
        for (int tx = x; tx<x+w; ++tx) {
            plot(tx, ty, rgb);
        }
    }
}

extern
int run_spatial_tests()
{
    static const uint32_t num_objects = 200;
    std::list<body_t*> list_;

    app_t app;
    if (!app.init(512, 512)) {
        return false;
    }

    prng::seed_t seed = SDL_GetTicks();

    spatial_t hash;

    for (int i = 0; i<num_objects; ++i) {

        float out[2];
        prng::vrand2d(seed, out);
        const vec2f_t p = {prng::randfu(seed) * 512,
                           prng::randfu(seed) * 512};
        const float r = prng::randfu(seed) * 16 + 4;
        body_t * obj = new body_t(p, r, nullptr);
        obj->vel.x = out[0];
        obj->vel.y = out[1];
        list_.push_front(obj);
        hash.insert(obj);
    }

    body_pair_set_t set;

    while (app.tick()) {

        for (int y=0; y<512/spatial_t::width; ++y) {
            for (int x=0; x<512/spatial_t::width; ++x) {
                uint32_t clr = hash.dbg_ocupancy(x, y) * 8;
                app.rect(x*32, y*32, 32, 32, clr);
            }
        }

#if 1
        hash.query_collisions(set);
        body_set_t obj_set;

        {
            float p1 = 128;
            float p2 = 128+256;
            hash.query_rect(vec2f_t{p1, p1}, vec2f_t{p2, p2}, obj_set);
            for (auto obj : obj_set) {
                vec2f_t p = obj->pos();
                app.circle(p.x, p.y, obj->radius(), 0x3377aa);
            }
        }

        for (auto pair : set) {

            body_t * a = pair.first;
            body_t * b = pair.second;

            float nx = b->pos().x - a->pos().x;
            float ny = b->pos().y - a->pos().y;
            float nl = sqrtf(nx*nx + ny*ny);

            if (nl < 0.0001f)
                continue;

            nx /= nl;
            ny /= nl;

            float os = ((b->radius() + a->radius()) - nl) * .5f;

#if 0
            app.line(a->pos().x, a->pos().y, b->pos().x, b->pos().y, 0xff0000);
            app.line(a->pos().x, a->pos().y, a->pos().x-nx*os, a->pos().y-ny*os, 0x203060);
            app.line(b->pos().x, b->pos().y, b->pos().x+nx*os, b->pos().y+ny*os, 0x203060);
#endif
            vec2f_t v1 { -nx * os, -ny * os };
            hash.move(a, a->pos()+v1);

            vec2f_t v2 { +nx * os, +ny * os };
            hash.move(b, b->pos()+v2);
        }

        for (auto obj:list_) {

            float speed = 0.5f;
            uint32_t colour = 0x404040;

            if (obj_set[obj]) {
                speed = 1.f;
                colour = 0x00ff00;
            }

            app.circle(obj->pos().x, obj->pos().y, obj->radius(), colour);

            vec2f_t v1 = {obj->vel.x * speed,
                          obj->vel.y * speed};
            hash.move(obj, obj->pos()+v1);

            obj->vel.x *= (obj->pos().x<000&&obj->vel.x<0) ? -1 : 1;
            obj->vel.x *= (obj->pos().x>512&&obj->vel.x>0) ? -1 : 1;
            obj->vel.y *= (obj->pos().y<000&&obj->vel.y<0) ? -1 : 1;
            obj->vel.y *= (obj->pos().y>512&&obj->vel.y>0) ? -1 : 1;
        }

        for (int i = 0; i<512; i += spatial_t::width) {
            app.line(0, i, 512, i, 0x202020);
            app.line(i, 0, i, 512, 0x202020);
        }

        app.line(0, 128, 512, 128, 0x203090);
        app.line(0, 384, 512, 384, 0x203090);
        app.line(128, 0, 128, 512, 0x203090);
        app.line(384, 0, 384, 512, 0x203090);

        set.clear();
        obj_set.clear();
#endif

        SDL_Delay(0);
    }

    return 0;
}
