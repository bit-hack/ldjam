#include <SDL/SDL.h>

#include "../../framework_core/random.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/tiles.h"
#include "../../framework_core/timer.h"
#include "../../framework_draw/draw.h"

enum {
    e_object_player
};

struct service_t {
    draw_t * draw_;
    collision_map_t * map_;
};

struct player_t : public object_ex_t<e_object_player, player_t> {

    service_t * service_;

    player_t(object_service_t s)
        : object_ex_t()
        , service_(static_cast<service_t*>(s))
    {
    }

    virtual void tick() override {
        service_->draw_->colour_ = 0x406080;
        service_->draw_->rect(recti_t{ 32, 32, 64, 64 });
    }
};

struct app_t {

    static const int32_t _MAP_WIDTH = 320 / 16;
    static const int32_t _MAP_HEIGHT = 240 / 16;

    SDL_Surface * screen_;
    draw_t draw_;
    timing_t<uint32_t> timer_;
    bitmap_t target_;
    bool active_;
    collision_map_t map_;
    random_t rand_;
    service_t service_;
    object_factory_t factory_;
    object_ref_t player_;

    app_t()
        : screen_(nullptr)
        , draw_()
        , timer_()
        , target_()
        , active_(true)
        , map_(vec2i_t{_MAP_WIDTH, _MAP_HEIGHT}, vec2i_t{16, 16})
        , rand_(0x12345)
        , factory_(&service_)
    {
        service_.draw_ = &draw_;
        service_.map_ = &map_;
    }

    bool app_init() {
        if (SDL_Init(SDL_INIT_VIDEO)) {
            return false;
        }
        screen_ = SDL_SetVideoMode(640, 480, 32, 0);
        if (!screen_) {
            return false;
        }
        if (!bitmap_t::create(320, 240, target_)) {
            return false;
        }
        if (!target_.valid()) {
            return false;
        }
        draw_.set_target(target_);
        // setup frame timer
        timer_.period_ = 1000 / 30;
        timer_.func_ = SDL_GetTicks;
        timer_.reset();
        return true;
    }

    bool map_init() {
        map_.clear(0);
        uint8_t * tile = map_.get();
        for (int32_t y=0; y<map_.size().y; ++y) {
            for (int32_t x=0; x<map_.size().x; ++x) {
                bool set = rand_.rand_chance(20) || y==(map_.size().y-1);
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
    }

    bool poll_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
        }
        return true;
    }

    bool tick() {
        draw_.colour_ = 0x202020;
        draw_.clear();
        map_draw();
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
        player_ = factory_.create(e_object_player);
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
        return true;
    }
};

int main(const int argc, char *args[]) {
    app_t app;
    return app.main(argc, args) ? 0 : 1;
}
