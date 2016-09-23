#include "common.h"
#include "camera.h"
#include "player.h"

struct app_t {

    static const int32_t _MAP_WIDTH = 320 / 16;
    static const int32_t _MAP_HEIGHT = 240 / 16;

    SDL_Surface * screen_;
    bitmap_t target_;
    SDL_Joystick * joystick_;

    bool active_;

    timing_t<uint32_t> timer_;
    random_t rand_;

    draw_t draw_;
    collision_map_t map_;
    object_factory_t factory_;

    service_t service_;

    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , map_(vec2i_t{_MAP_WIDTH, _MAP_HEIGHT}, vec2i_t{16, 16})
        , rand_(0x12345)
        , factory_(&service_)
        , joystick_(nullptr)
    {
        service_.draw_ = &draw_;
        service_.map_ = &map_;
    }

    bool app_init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
            return false;
        }

        const bool fullscreen = false;
        screen_ = SDL_SetVideoMode(
               640,
               480,
               32,
               fullscreen ? SDL_FULLSCREEN : 0);
        if (!screen_) {
            return false;
        }
        if (!bitmap_t::create(320, 240, target_)) {
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
        // setup gamepad
        if (SDL_NumJoysticks()) {
            joystick_ = SDL_JoystickOpen(0);
            printf("using joystick %s\n", SDL_JoystickName(0));
        }
        return true;
    }

    bool app_quit() {
        if (joystick_) {
            SDL_JoystickClose(joystick_);
        }
        if (screen_) {
            SDL_FreeSurface(screen_);
        }
        SDL_Quit();
        return true;
    }

    bool map_init() {

        const int32_t _CHANCE = 10;

        map_.clear(0);
        uint8_t * tile = map_.get();
        for (int32_t y=0; y<map_.size().y; ++y) {
            for (int32_t x=0; x<map_.size().x; ++x) {
                bool set = rand_.rand_chance(_CHANCE) || y==(map_.size().y-1);
                tile[x + y * map_.size().x] = set ? e_tile_solid : 0;
            }
        }
        // preprocess map for collision data
        map_.preprocess();
        return true;
    }

    bool map_draw() {
        draw_.colour_ = 0x204060;
        uint8_t * tile = map_.get();
        for (int32_t y=0; y<map_.size().y; ++y) {
            for (int32_t x=0; x<map_.size().x; ++x) {
                const uint8_t t = tile[x + y * map_.size().x];
                if (!(t & e_tile_solid)) {
                    continue;
                }
                draw_.rect(recti_t(x * 16, y * 16, 16, 16, recti_t::e_relative));
            }
        }
        return true;
    }

    bool poll_joypad() {
        if (!joystick_) {
            return false;
        }
        if (!service_.player_.valid()) {
            return false;
        }
        player_t & player = service_.player_->cast<player_t>();

        int16_t xaxis = SDL_JoystickGetAxis(joystick_, 0);
        float dx = 0.f;
        dx += xaxis <-0x2000 ? -1.f : 0.f;
        dx += xaxis > 0x2000 ?  1.f : 0.f;
        player.move(dx);

        if (SDL_JoystickGetButton(joystick_, 2)) {
            player.jump();
        }

        return true;
    }

    bool poll_keyboard() {
        float dx = 0.f;
        {
            const uint8_t *key = SDL_GetKeyState(nullptr);
            dx += key[SDLK_LEFT] ? -1.f : 0.f;
            dx += key[SDLK_RIGHT] ? 1.f : 0.f;
        }
        player_t & player = service_.player_->cast<player_t>();
        player.move(dx);
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
                case (SDLK_UP):
                    service_.player_->cast<player_t>().jump();
                    break;
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
        map_draw();

        poll_keyboard();
        poll_joypad();

        factory_.tick();
        return true;
    }

    bool main(const int argc, char *args[]) {
        if (!app_init()) {
            return false;
        }
        if (!map_init()) {
            return false;
        }

        factory_.add_creator<player_t>();
        service_.player_ = factory_.create(e_object_player);

        factory_.add_creator<camera_t>();
        object_ref_t camera = factory_.create(e_object_camera);

        while (active_) {
            if (!poll_events()) {
                return false;
            }
            if (timer_.frame()) {
                if (!tick()) {
                    return false;
                }
                assert(screen_);
                draw_.render_2x(screen_->pixels,
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
    app_t app;
    return app.main(argc, args) ? 0 : 1;
}
