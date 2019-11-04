#include <SDL/SDL.h>

#include "../../external/stb_ttf/stb_truetype.h"

#include "../../framework_core/objects.h"
#include "../../framework_core/timer.h"
#include "../../framework_draw/draw.h"

#include "../../framework_gui/framework_gui.h"
#include "../../framework_gui/framework_gui_widgets.h"

using namespace tengu;

// draw scale
static const int32_t _SCALE = 3;

struct gui_impl_render_t : public gui_extern_render_t {

    gui_impl_render_t(draw_t& d)
        : draw(d)
    {
        origin.x = 0;
        origin.y = 0;
    }

    draw_t& draw;

    // draw outline rectangle
    void draw_rect_outline(int32_t x0, int32_t y0, int32_t x1, int32_t y1) override
    {
        recti_t r = recti_t{ x0, y0, x1, y1 } + recti_t{ origin.x, origin.y, origin.x, origin.y };
        draw.colour_ = rgb;
        draw.outline(r);
    }

    // draw filled rectangle
    void draw_rect_fill(int32_t x0, int32_t y0, int32_t x1, int32_t y1) override
    {
        recti_t r = recti_t{ x0, y0, x1, y1 } + recti_t{ origin.x, origin.y, origin.x, origin.y };
        draw.colour_ = rgb;
        draw.rect(r);
    }

    // draw transparent rectangle
    void draw_shadow(int32_t x0, int32_t y0, int32_t x1, int32_t y1) override
    {
        const uint32_t drop = 4;
        draw.colour_ = rgb;
        {
            int32_t sx0 = x0 + drop;
            int32_t sx1 = x1 + drop;
            int32_t sy0 = y1;
            int32_t sy1 = y1 + drop;
            draw.rect_gliss(recti_t{ sx0, sy0, sx1, sy1 } + recti_t{ origin.x, origin.y, origin.x, origin.y });
        }
        {
            int32_t sx0 = x1;
            int32_t sx1 = x1 + drop;
            int32_t sy0 = y0 + drop;
            int32_t sy1 = y1;
            draw.rect_gliss(recti_t{ sx0, sy0, sx1, sy1 } + recti_t{ origin.x, origin.y, origin.x, origin.y });
        }
    }

    // draw rounded rectangle
    void draw_circle(int32_t x0, int32_t y0, int32_t r) override
    {
        draw.colour_ = rgb;
        draw.circleAA(vec2i_t{ x0, y0 } + origin, r);
    }

    void draw_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const std::string& txt) override
    {
        // todo
    }
};

struct gui_impl_io_t : public gui_extern_io_t {

    gui_impl_io_t()
        : mouse_x(0)
        , mouse_y(0)
        , mouse_button_new(0)
        , mouse_button_old(0)
    {
    }

    // previous mouse button state
    int32_t mouse_x, mouse_y;
    uint8_t mouse_button_old;
    uint8_t mouse_button_new;

    void update()
    {
        mouse_button_old = mouse_button_new;
        mouse_button_new = SDL_GetMouseState(&mouse_x, &mouse_y);
    }

    void mouse_get(mouse_state_t& out) override
    {
        const uint32_t n = mouse_button_new;
        const int32_t n_lmb = (SDL_BUTTON_LMASK & n) ? 2 : 0;
        const int32_t n_mmb = (SDL_BUTTON_MMASK & n) ? 2 : 0;
        const int32_t n_rmb = (SDL_BUTTON_RMASK & n) ? 2 : 0;

        const uint32_t o = mouse_button_old;
        const int32_t o_lmb = (SDL_BUTTON_LMASK & o) ? 1 : 0;
        const int32_t o_mmb = (SDL_BUTTON_MMASK & o) ? 1 : 0;
        const int32_t o_rmb = (SDL_BUTTON_RMASK & o) ? 1 : 0;

        // old  new  out
        // 0    0     0
        // 1    0    -1
        // 0    2     2
        // 1    2     1

        out.button[0] = n_lmb - o_lmb;
        out.button[1] = n_mmb - o_mmb;
        out.button[2] = n_rmb - o_rmb;
        out.x = mouse_x / _SCALE;
        out.y = mouse_y / _SCALE;
    };

    void key_get(key_state_t& key) override
    {
    }
};

// application framework
struct app_t {
    // frames per second
    static const int32_t _FPS = 30;
    // effective screen size
    static const int32_t _WIDTH = 320, _HEIGHT = 240;

    SDL_Surface* screen_;
    bitmap_t target_;
    bool active_;
    timing_t<uint32_t> timer_;
    draw_t draw_;
    gui_t gui_;
    gui_impl_io_t gui_io_;
    gui_impl_render_t gui_render_;

    // ctor
    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , gui_render_(draw_)
    {
    }

    // initalize framework
    bool app_init()
    {
        // init SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
            return false;
        }
        // create SDL window
        const bool fullscreen = false;
        screen_ = SDL_SetVideoMode(_WIDTH * _SCALE, _HEIGHT * _SCALE, 32,
            fullscreen ? SDL_FULLSCREEN : 0);
        if (!screen_) {
            return false;
        }
        // create a render target
        if (!target_.create(vec2i_t{ _WIDTH, _HEIGHT })) {
            return false;
        }
        if (!target_.valid()) {
            return false;
        }
        // pass render target to drawing library
        draw_.set_target(target_);
        // setup framerate timer
        timer_.period_ = 1000 / _FPS;
        timer_.func_ = SDL_GetTicks;
        timer_.reset();
        return true;
    }

    // teardown framework
    bool app_quit()
    {
        if (screen_) {
            SDL_FreeSurface(screen_);
        }
        SDL_Quit();
        return true;
    }

    // monitor SDL event stream
    bool poll_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case (SDLK_ESCAPE):
                    return false;
                default:
                    break;
                }
            }
        }
        return true;
    }

    // per frame update function
    bool tick()
    {
        gui_io_.update();

        draw_.colour_ = 0x202020;
        draw_.clear();
        gui_.tick(gui_io_, gui_render_);

        gui_event_t event;
        while (gui_.event_pop(event)) {
            printf("event %d\n", event.type);
        }

        return true;
    }

    void init_gui()
    {
        auto z = gui_.alloc.frame_create<gui_widget_frame_t>();
        gui_.root = z;
        z->x0 = 32;
        z->y0 = 32;
        z->x1 = 32 + 96;
        z->y1 = 32 + 128;

        auto b = gui_.alloc.frame_create<gui_widget_button_t>();
        z->child_add(b);
        b->x0 = 10;
        b->y0 = 10;
        b->x1 = 80;
        b->y1 = 25;

        auto c = gui_.alloc.frame_create<gui_widget_check_box_t>();
        z->child_add(c);
        c->x0 = 20;
        c->y0 = 60;
        c->x1 = 30;
        c->y1 = 70;

        auto d = gui_.alloc.frame_create<gui_widget_hslider_t>();
        z->child_add(d);
        d->x0 = 15;
        d->y0 = 100;
        d->x1 = 90;
        d->y1 = 115;

        auto e = gui_.alloc.frame_create<gui_widget_vslider_t>();
        z->child_add(e);
        e->x0 = 2;
        e->y0 = 50;
        e->x1 = 12;
        e->y1 = 100;
        e->user_tag = 0x1234;

        auto f = gui_.alloc.frame_create<gui_widget_progress_bar_t>();
        z->child_add(f);
        f->x0 = 15;
        f->y0 = 110;
        f->x1 = 90;
        f->y1 = 120;
        f->user_tag = 0x4321;
    }

    // framework entry point
    bool main(const int argc, char* args[])
    {
        // initalize the framework
        if (!app_init()) {
            return false;
        }

        init_gui();

        // main game loop
        while (active_) {
            // monitor sdl events
            if (!poll_events()) {
                return false;
            }
            // if its time the next frame
            if (timer_.frame()) {
                // update main logic
                if (!tick()) {
                    return false;
                }
                // display draw buffer to screen
                assert(screen_);
                if (_SCALE == 1) {
                    draw_.render_1x(screen_->pixels, screen_->pitch / sizeof(uint32_t));
                }
                if (_SCALE == 2) {
                    draw_.render_2x(screen_->pixels, screen_->pitch / sizeof(uint32_t));
                }
                if (_SCALE == 3) {
                    draw_.render_3x(screen_->pixels, screen_->pitch / sizeof(uint32_t));
                }
                SDL_Flip(screen_);
            } else {
                // dont burn the cpu
                SDL_Delay(1);
            }
        }
        // teardown application
        app_quit();
        return true;
    }
};

// c main function
int main(const int argc, char* args[])
{
    app_t* app = new app_t;
    return app->main(argc, args) ? 0 : 1;
}
