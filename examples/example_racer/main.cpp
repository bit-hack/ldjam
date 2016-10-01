#include "common.h"

struct object_car_t : object_ex_t<e_object_car, object_car_t> {

    object_car_t(object_service_t service)
        : object_ex_t()
        , pos_(80, 60)
        , draw_(static_cast<service_t*>(service)->draw_)
        , rot_(0.f)
    {
    }

    draw_ex_t & draw_;
    bitmap_t sprite_1_;
    bitmap_t sprite_2_;
    vec2f_t pos_;

    float rot_;

    void draw_car(bitmap_t & sprite, const vec2f_t & pos) {
        assert(sprite.valid());
        blit_info_ex_t info;
        info.bitmap_ = &sprite;
        info.dst_pos_ = pos;
        info.src_rect_ = recti_t{0, 0, 15, 15};
        info.type_ = e_blit_key;
        float scale = 0.8f;
        auto & m = info.matrix_;
        m[0] = scale * cosf(rot_) * 1.2;
        m[1] = scale *-sinf(rot_);
        m[2] = scale * sinf(rot_) * 2.f * 1.2;
        m[3] = scale * cosf(rot_) * 2.f;
        for (int i = 0; i<8; ++i) {
            draw_.key_ = 0x00D77BBA;
            draw_.blit<true>(info);
            info.dst_pos_ -= vec2f_t{0, 1.f/scale};
            info.src_rect_ = info.src_rect_ + vec2i_t{ 16, 0 };
        }
    }

    virtual void tick() override {
        draw_car(sprite_1_, vec2f_t{ 50, 60});
        draw_car(sprite_2_, vec2f_t{110, 60});
        // advance the rotation
        rot_ += 0.1f;
        if (rot_>C_2PI) rot_ -= C_2PI;
    }

    void init(int dummy) {
        bitmap_t::load("assets/car1.bmp", sprite_1_);
        bitmap_t::load("assets/car2.bmp", sprite_2_);
    }
};

struct app_t {

    SDL_Surface * screen_;
    bitmap_t target_;
    bool active_;
    timing_t<uint32_t> timer_;
    random_t rand_;
    draw_t draw_;
    object_factory_t factory_;

    service_t service_;

    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , rand_(0x12342)
        , factory_(&service_)
        , service_{draw_ex_t(draw_), factory_}
    {
    }

    bool app_init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
            return false;
        }

        const int32_t C_WIDTH = 160;
        const int32_t C_HEIGHT = 120;

        const bool fullscreen = false;
        screen_ = SDL_SetVideoMode(
               C_WIDTH * 3,
               C_HEIGHT * 3,
               32,
               fullscreen ? SDL_FULLSCREEN : 0);
        if (!screen_) {
            return false;
        }
        if (!bitmap_t::create(C_WIDTH, C_HEIGHT, target_)) {
            return false;
        }
        if (!target_.valid()) {
            return false;
        }
        // setup display target
        draw_.set_target(target_);
        // setup frame timer
        timer_.period_ = 1000 / 30;
        timer_.func_ = SDL_GetTicks;
        timer_.reset();
        return true;
    }

    bool app_quit() {
        if (screen_) {
            SDL_FreeSurface(screen_);
        }
        SDL_Quit();
        return true;
    }

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
                case (SDLK_F11):
                    SDL_WM_ToggleFullScreen(screen_);
                    break;
                default:
                    break;
                }
            }
        }
        return true;
    }

    bool tick() {
        draw_.colour_ = 0x202020;
        draw_.clear();
        factory_.tick();
        factory_.sort();
        factory_.collect();
        return true;
    }

    bool main(const int argc, char *args[]) {
        if (!app_init()) {
            return false;
        }

        factory_.add_creator<object_car_t>();
        service_.objects_.insert("car", factory_.create<object_car_t>(0));

        draw_.viewport(recti_t{0, 0, 160, 120});

        while (active_) {
            if (!poll_events()) {
                return false;
            }
            if (timer_.frame()) {
                if (!tick()) {
                    return false;
                }
                assert(screen_);
                draw_.render_3x(screen_->pixels,
                                screen_->pitch / sizeof(uint32_t));
                SDL_Flip(screen_);
            }
            else {
                SDL_Delay(1);
            }
        }

        app_quit();
        return true;
    }
};

int main(const int argc, char *args[]) {
    app_t * app = new app_t;
    return app->main(argc, args) ? 0 : 1;
}