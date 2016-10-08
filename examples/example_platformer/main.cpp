#include "common.h"
#include "camera.h"
#include "player.h"
#include "particles.h"
#include "map.h"

using namespace tengu;

struct app_t {

    SDL_Surface * screen_;
    bitmap_t target_;

    bool active_;

    timing_t<uint32_t> timer_;
    random_t rand_;

    draw_t draw_;
    object_factory_t factory_;
    gamepad_key_t input_;
    gamepad_joy_t joystick_;

    service_t service_;

    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , rand_(0x12342)
        , factory_(&service_)
        , joystick_()
        , service_{draw_ex_t(draw_),
                   factory_,
                   &input_}
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

        // open the joystick
        if (joystick_.open(0)) {
            service_.gamepad_ = &joystick_;
        }
        return true;
    }

    bool app_quit() {
        if (screen_) {
            SDL_FreeSurface(screen_);
        }
        joystick_.close();
        SDL_Quit();
        return true;
    }

    bool poll_input() {
        gamepad_t & gamepad = *service_.gamepad_;
        if (service_.objects_["player"].valid()) {
            player_t &player = service_.objects_["player"]->cast<player_t>();
            player.move(gamepad.axis_.x);
            if (gamepad.delta_[input_.e_button_x] &&
                gamepad.button_[input_.e_button_x]) {
                player.cast<player_t>().jump();
            }
        }
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
                case (SDLK_s):
                    service_.objects_["camera"]->cast<camera_t>().shake(1.f);
                    break;
                default:
                    break;
                }
            }
        }
        return true;
    }

    bool tick() {
        draw_.colour_ = 0x305192;
        draw_.clear();

        service_.gamepad_->tick();
        poll_input();

        factory_.tick();
        factory_.sort();
        factory_.collect();

        return true;
    }

    bool main(const int argc, char *args[]) {
        if (!app_init()) {
            return false;
        }

        factory_.add_creator<dust_t>();
        factory_.add_creator<player_shadow_t>();
        factory_.add_creator<player_t>();
        factory_.add_creator<camera_t>();
        factory_.add_creator<player_splash_t>();
        factory_.add_creator<obj_map_t>();

        service_.objects_.insert("map", factory_.create(e_object_map));
        service_.objects_.insert("player", factory_.create(e_object_player));
        service_.objects_.insert("camera", factory_.create(e_object_camera));

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
