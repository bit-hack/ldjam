#include <SDL/SDL.h>

#include "../../framework_core/objects.h"
#include "../../framework_core/timer.h"
#include "../../framework_draw/draw.h"

using namespace tengu;

// object type enumeration
enum {
    e_object_editor = 0,
    e_object_computer,
};

// service locator given to all objects at spawn
struct service_t {
    draw_ex_t draw_;
    object_factory_t & factory_;
    bitmap_t font_;
};

// computer
struct object_computer_t:
    object_ex_t<e_object_computer, object_computer_t> {

    object_computer_t(object_service_t service)
        : object_ex_t()
    {
    }

    virtual void tick() override {
    }
};

// editor
struct object_editor_t:
    object_ex_t<e_object_editor, object_editor_t> {

    static const size_t _WIDTH = 40;
    static const size_t _HEIGHT = 30;

    object_ref_t comp_;
    service_t & service_;
    draw_ex_t & draw_;

    struct buffer_t {
        std::array<uint8_t, _WIDTH * _HEIGHT> cells_;
        std::array<uint8_t, _WIDTH * _HEIGHT> attrib_;
    } buffer_;

    void put(const vec2i_t & i, const char * a) {
        vec2i_t d = i;
        for (;*a; ++a) {
            put(d, *a);
            d.x++;
        }
    }

    void put(const vec2i_t & i, uint8_t ch) {
        buffer_.cells_[i.x + i.y * _WIDTH] = ch;
    }

    void fill(const recti_t & r, uint8_t ch) {
        for (auto y=r.y0; y<=r.y1; ++y) {
            for (auto x=r.x0; x<=r.x1; ++x) {
                buffer_.cells_[y * _WIDTH + x] = ch;
            }
        }
    }

    object_editor_t(object_service_t service)
        : object_ex_t()
        , service_(*static_cast<service_t*>(service))
        , draw_(service_.draw_)
    {
        fill(recti_t{0, 0, _WIDTH-1, _HEIGHT-1}, ' ');
        fill(recti_t{0, 0, _WIDTH-1, 0}, 'd' + 64 + 32);
        put(vec2i_t{0, 0}, "CODE");
        put(vec2i_t{21, 0}, "STK");
        put(vec2i_t{26, 0}, "MEM");
        put(vec2i_t{31, 0}, "IN");
        put(vec2i_t{31, 9}, "OUT");
        fill(recti_t{20, 1, 20, _HEIGHT-1}, 's'+64);
        fill(recti_t{25, 1, 25, _HEIGHT-1}, 's'+64);
        fill(recti_t{30, 1, 30, _HEIGHT-1}, 's'+64);
        put(vec2i_t{20, 0}, 'b' + 64 + 32);
        put(vec2i_t{25, 0}, 'b' + 64 + 32);
        put(vec2i_t{30, 0}, 'b' + 64 + 32);
    }


    void init(const object_ref_t & ref) {
        comp_ = ref;
    }

    virtual void tick() override {

        if (!service_.font_.valid()) {
            return;
        }

        tilemap_t tiles;
        tiles.bitmap_ = &service_.font_;
        tiles.type_ = e_blit_opaque;
        tiles.cell_size_ = vec2i_t{8, 8};
        tiles.attrib_ = buffer_.attrib_.data();
        tiles.cells_ = buffer_.cells_.data();
        tiles.map_size_ = vec2i_t { int32_t(_WIDTH), int32_t(_HEIGHT) };

        draw_.blit(tiles, vec2i_t{0, 0});
    }
};

// application framework
struct app_t {

    // draw scale
    static const int32_t _SCALE = 2;
    // frames per second
    static const int32_t _FPS = 30;
    // effective screen size
    static const int32_t _WIDTH = 320, _HEIGHT = 240;

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
        , service_{draw_ex_t(draw_),
                   factory_}
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
        screen_ = SDL_SetVideoMode(
               _WIDTH * _SCALE,
               _HEIGHT * _SCALE,
               32,
               fullscreen ? SDL_FULLSCREEN : 0);
        if (!screen_) {
            return false;
        }
        // create a render target
        if (!target_.create(vec2i_t{_WIDTH, _HEIGHT})) {
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
        //
        if (!service_.font_.load("assets/font.bmp")) {
            return false;
        }
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
        factory_.add_creator<object_editor_t>();
        factory_.add_creator<object_computer_t>();
        object_ref_t comp = factory_.create<object_computer_t>();
        factory_.create<object_editor_t>(comp);
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
                assert(screen_ && _SCALE==2);
                draw_.render_2x(screen_->pixels, screen_->pitch / sizeof(uint32_t));
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
