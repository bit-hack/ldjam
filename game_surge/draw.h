#pragma once

#include <cassert>
#include <cstdint>
#include <array>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "../framework/vec2.h"
#include "../framework/random.h"
#include "../framework/timer.h"

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
extern uint64_t time_func();

enum sprite_t {
    e_sprite_player_c,
    e_sprite_player_l,
    e_sprite_player_r,
    e_sprite_bullet,
    e_sprite_bomb,
    e_sprite_thrust_1,
    e_sprite_thrust_2,
    e_sprite_powerup_1,
    e_sprite_powerup_2,
    e_sprite_unknown_1,
    e_sprite_smoke_1,
    e_sprite_smoke_2,
    e_sprite_smoke_3,
    e_sprite_smoke_4,
    e_sprite_smoke_5,
    E_SPRITE_COUNT__
};

extern
std::array<SDL_Rect, E_SPRITE_COUNT__> s_sprite_offset;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
struct draw_t
{
    SDL_Window * window_;
    SDL_Renderer * render_;
    delta_time_t shake_time_;
    prng::seed_t seed_;
    SDL_Surface * image_;
    SDL_Texture * texture_;

    float shake_;
    std::array<vec2f_t, 2> shake_offset_;

    draw_t()
        : window_(nullptr)
        , render_(nullptr)
        , shake_(0.f)
        , shake_time_(time_func, 30)
        , seed_(0xbeef)
        , image_(nullptr)
        , texture_(nullptr)
    {
        shake_offset_[0] = vec2f_t{
            prng::randfs(seed_),
            prng::randfs(seed_)};
        shake_offset_[1] = vec2f_t{
            prng::randfs(seed_),
            prng::randfs(seed_)};
    }

    bool init(const uint32_t w, const uint32_t h, bool fullscreen=true) {

        const int C_FLAGS = fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0;

        assert(!(render_ || window_));
        if (SDL_CreateWindowAndRenderer(w*2, h*2, C_FLAGS, &window_, &render_)) {
            return false;
        }
        if (!(window_ && render_)) {
            return false;
        }
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_RenderSetLogicalSize(render_, w, h);

        image_ = SDL_LoadBMP("art/sprites.bmp");
        if (!image_) {
            printf("Unable to open sprites!!\n");
            return false;
        }
        SDL_SetColorKey(image_, SDL_TRUE, 0x0);

        texture_ = SDL_CreateTextureFromSurface(render_, image_);
        if (!texture_) {
            printf("Unable to create texture!!\n");
            return false;
        }

        return true;
    }

    void present() {
        assert(render_);
        SDL_RenderPresent(render_);
        SDL_SetRenderDrawColor(render_, 0x10, 0x10, 0x10, 0);
        SDL_RenderClear(render_);

        shake_ = shake_ <= 0.f ? 0.f : (shake_ * 0.8f - 0.05f);

        while (shake_time_.deltai()) {
            shake_time_.step();
            shake_offset_[0] = shake_offset_[1];
            shake_offset_[1] = vec2f_t{
                prng::randfs(seed_),
                prng::randfs(seed_)};
        }

    }

    void draw_sprite(sprite_t sprite, const vec2f_t & pos) {

        switch (sprite) {
        case (e_sprite_smoke_1):
        case (e_sprite_smoke_2):
        case (e_sprite_smoke_3):
        case (e_sprite_smoke_4):
        case (e_sprite_smoke_5):
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_ADD);
            break;
        default:
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            break;
        }

        vec2f_t offs = shake_offset_[0] + (shake_offset_[1]-shake_offset_[0]) * shake_time_.deltaf();
        offs *= shake_;
        offs += pos;
        const SDL_Rect src = s_sprite_offset[sprite];
        const SDL_Rect dst = SDL_Rect{int32_t(offs.x - src.w/2), int32_t(offs.y - src.h/2), src.w, src.h};
        SDL_RenderCopy(render_, texture_, &src, &dst);
    }

    void draw_colour(uint8_t r, uint8_t g, uint8_t b) {
        SDL_SetRenderDrawColor(render_, r, g, b, 0);
    }

    void draw_rect(int x, int y, int w, int h) {

        vec2f_t offs = shake_offset_[0] + (shake_offset_[1]-shake_offset_[0]) * shake_time_.deltaf();
        offs *= shake_;

        SDL_Rect r = { x + offs.x, y + offs.y, w, h };
        SDL_RenderDrawRect(render_, &r);
    }
};
