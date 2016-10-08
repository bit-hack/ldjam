#include <SDL/SDL.h>

#include "../../framework_core/common.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/timer.h"
#include "../../framework_draw/draw.h"

using namespace tengu;

namespace {
// draw scale
static const int32_t _SCALE = 3;
// frames per second
static const int32_t _FPS = 30;
// effective screen size
static const int32_t _WIDTH = 160, _HEIGHT = 120;
}

struct camera_t {

    std::array<float, 6> mat_;
    vec2f_t scroll_, center_;
    const uint32_t width_, height_;

    camera_t(uint32_t width, uint32_t height)
        : width_(width)
        , height_(height)
    {
        init();
    }

    vec2f_t transform(const vec3f_t & in) const {
        return vec2f_t {
            // ---- ---- ---- ---- x
            in.x * mat_[0] +
            in.y * mat_[1] +
            in.z * mat_[2] - scroll_.x + center_.x,
            // ---- ---- ---- ---- y
            in.x * mat_[3] +
            in.y * mat_[4] +
            in.z * mat_[5] - scroll_.y + center_.y,
        };
    }

    void init() {
        const float RAD = C_2PI / 360.f;
        const float a = 30.f, b = 30.f;
        const float sx = 8.f, sy = 8.f, sz = 8.f;
        mat_[0] = cosf(a*RAD)*sx;
        mat_[1] =-cosf(b*RAD)*sy;
        mat_[2] = 0;
        mat_[3] = sinf(a*RAD)*sx;
        mat_[4] = sinf(b*RAD)*sy;
        mat_[5] =-sz;
        center_.x = float(_WIDTH/2);
        center_.y = float(_HEIGHT/2);
        scroll_.x = 0.f;
        scroll_.y = 0.f;
    }
};

namespace {
bool intersect(float a, float b,
               float c, float d,
               vec2f_t & out) {
    const float denom = (a - b - c + d);
    if (absv(denom) == 0.f)
        return false;
    out = vec2f_t {
        (a - c) / denom,
        (a * d - b * c) / denom
    };
    return true;
}
} // namespace {}

// object type enumeration
enum {
    e_object_simple = 0,
};

// service locator given to all objects at spawn
struct service_t {
    draw_ex_t draw_;
    object_factory_t & factory_;
};

// simple object type
struct object_simple_t : object_ex_t<e_object_simple, object_simple_t> {

    draw_ex_t & draw_;

    std::array<vec2f_t, 4> point_;

    vec2f_t * select_;

    object_simple_t(object_service_t service)
        : object_ex_t()
        , draw_(static_cast<service_t*>(service)->draw_)
        , point_{{
             vec2f_t{32, 32},
             vec2f_t{32, 60},
             vec2f_t{_WIDTH - 32, 80},
             vec2f_t{_WIDTH - 32, 40}
         }}
    {
    }

    void mouse() {
        int mx, my;
        int b = SDL_GetMouseState(&mx, &my);
        mx /= _SCALE;
        my /= _SCALE;
        if (b&SDL_BUTTON(SDL_BUTTON_LEFT)) {
            vec2f_t mp{float(mx), float(my)};
            if (select_==nullptr) {
                select_ = point_.data();
                for (auto & p : point_) {
                    if (vec2f_t::distance(mp, p) <
                        vec2f_t::distance(mp, *select_)) {
                        select_ = &p;
                    }
                }
            }
            else {
                if (vec2f_t::distance(mp, *select_) < 32) {
                    select_->y = float(clampv<int32_t>(32, my, _HEIGHT-32));
                }
            }
        }
        else {
            select_ = nullptr;
        }
    }

    virtual void tick() override {

        mouse();

        draw_.colour_ = 0x404040;
        draw_.line(vec2f_t{32, _HEIGHT-32}, vec2f_t{_WIDTH-32, _HEIGHT-32});
        draw_.line(vec2f_t{32, 32}, vec2f_t{32, _HEIGHT-32});
        draw_.line(vec2f_t{_WIDTH-32, 32}, vec2f_t{_WIDTH-32, _HEIGHT-32});

        // draw points
        draw_.colour_ = 0xff8833;
        draw_.circle(vec2i_t{32, int32_t(point_[0].y)}, 2);
        draw_.colour_ = 0x3388ff;
        draw_.circle(vec2i_t{32, int32_t(point_[1].y)}, 2);
        draw_.colour_ = 0xff8833;
        draw_.circle(vec2i_t{_WIDTH-32, int32_t(point_[2].y)}, 2);
        draw_.colour_ = 0x3388ff;
        draw_.circle(vec2i_t{_WIDTH-32, int32_t(point_[3].y)}, 2);

        // draw lines
        draw_.colour_ = 0xff8833;
        draw_.line(point_[0], point_[2]);
        draw_.colour_ = 0x3388ff;
        draw_.line(point_[1], point_[3]);

        vec2f_t ix {};
        intersect(point_[0].y, point_[2].y,
                  point_[1].y, point_[3].y, ix);
        draw_.colour_ = 0xff0000;
        if (ix.x >= 0.f && ix.x < 1.f)
            draw_.colour_ = 0xffffff;
        draw_.circle(vec2i_t{int32_t(lerp(32.f, _WIDTH-32.f, ix.x)), int32_t(ix.y)}, 2);
    }
};

// application framework
struct app_t {

    SDL_Surface * screen_;
    bitmap_t target_;
    bool active_;
    timing_t<uint32_t> timer_;
    draw_t draw_;
    object_factory_t factory_;
    service_t service_;

    // ctor
    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , factory_(&service_)
        , service_{draw_ex_t(draw_), factory_}
    {
    }

    // initalize framework
    bool app_init() {
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
        if (!bitmap_t::create(_WIDTH, _HEIGHT, target_)) {
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
    bool app_quit() {
        if (screen_) {
            SDL_FreeSurface(screen_);
        }
        SDL_Quit();
        return true;
    }

    // monitor SDL event stream
    bool poll_events() {
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
    bool tick() {
        draw_.colour_ = 0x202020;
        draw_.clear();
        factory_.tick();
        factory_.sort();
        factory_.collect();
        return true;
    }

    // framework entry point
    bool main(const int argc, char *args[]) {
        // initalize the framework
        if (!app_init()) {
            return false;
        }
        // reigster and create simple object
        factory_.add_creator<object_simple_t>();
        factory_.create<object_simple_t>();
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
                assert(screen_ && _SCALE==3);
                draw_.render_3x(screen_->pixels, screen_->pitch / sizeof(uint32_t));
                SDL_Flip(screen_);
            }
            else {
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
int main(const int argc, char *args[]) {
    app_t * app = new app_t;
    return app->main(argc, args) ? 0 : 1;
}
