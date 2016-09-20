#include <SDL/SDL.h>

#include "../../framework_core/fsm.h"
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
    vec2f_t pos_[2];
    float dx_;

    typedef fsm_t<player_t> player_fsm_t;
    typedef player_fsm_t::fsm_state_t player_state_t;

    player_fsm_t fsm_;
    player_state_t fsm_state_air_; // falling jumping
    player_state_t fsm_state_run_; // running on ground
    player_state_t fsm_state_slide_; // wall slide

    player_t(object_service_t s)
        : object_ex_t()
        , service_(static_cast<service_t*>(s))
        , fsm_(this)
        , fsm_state_air_(&player_t::tick_air)
        , fsm_state_run_(&player_t::tick_run)
        , fsm_state_slide_(&player_t::tick_slide)
        , dx_(0.f)
    {
        pos_[0] = pos_[1] = vec2f_t{64, 64};
    }

    rectf_t bound() const {
        return rectf_t{
            pos_[1].x-4,
            pos_[1].y-10,
            pos_[1].x+4,
            pos_[1].y };
    }

    void tick_run() {
        vec2f_t res;
        if (!service_->map_->collide(bound(), res)) {
            fsm_.state_change(fsm_state_air_);
            return;
        }
        else {
            // reduce size slightly to ensure continuous collision
//            res.y -= signv(res.y);
            // apply collision response as impulse
            pos_[1] += res;
            // find velocity (verlet)
            const vec2f_t vel = pos_[1] - pos_[0];
            // integrate
            pos_[0] = pos_[1];
            pos_[1] += vel + vec2f_t {dx_, 0.f};
            // ground friction
            pos_[0].x = lerp(pos_[0].x, pos_[1].x, 0.4f);
        }
    }

    void tick_air() {
        const float _GRAVITY = 0.4f;

        vec2f_t res;
        if (service_->map_->collide(bound(), res)) {
            const vec2f_t vel = pos_[1] - pos_[0];
            const bool falling = vel.y > 0.f;
            // feet landed on something
            if (res.y < 0.f && falling) {
                fsm_.state_change(fsm_state_run_);
                return;
            }
            // being pushed in the x axis
            if (absv(res.x) >= 0.f && (res.y == 0.f)) {
                if (signv(res.x) != signv(vel.x)) {
                    fsm_.state_change(fsm_state_slide_);
                    return;
                }
            }
            // apply collision as impulse
            pos_[1] += res;
            //
            if (res.y > 0.f) {
                pos_[0].y = pos_[1].y;
            }
        }
        // glide threw the air
        {
            // find velocity (verlet)
            const vec2f_t vel = pos_[1] - pos_[0];
            // integrate
            pos_[0] = pos_[1];
            pos_[1] += vel + vec2f_t{dx_, _GRAVITY};
        }
        // air resistance
        {
            pos_[0].x = lerp(pos_[0].x, pos_[1].x, 0.2f);
        }
    }

    void tick_slide() {
        const float _GRAVITY = 0.4f;

        const vec2f_t vel = pos_[1] - pos_[0];
        const bool falling = vel.y > 0.f;
        vec2f_t res;
        if (service_->map_->collide(bound(), res)) {
            // feet landed on something
            if (res.y < 0.f && falling) {
                fsm_.state_change(fsm_state_run_);
                return;
            }
            // reduce size slightly to ensure continuous collision
            res.x -= signv(res.x);
            // apply resolution as simple translate
            pos_[1].x += res.x;
            pos_[0].x = pos_[1].x;
            // find velocity (verlet)
            const vec2f_t vel = pos_[1] - pos_[0];
            // integrate
            pos_[0] = pos_[1];
            pos_[1] += vel + vec2f_t{0.f, _GRAVITY};
            //
            pos_[0].y = lerp(pos_[0].y, pos_[1].y, 0.4f);
        }
        else {
            // no collision so falling
            fsm_.state_change(fsm_state_air_);
            return;
        }
    }

    void jump() {
        if (fsm_.state() == fsm_state_run_) {
            pos_[0].y += 6.f;
        }
        if (fsm_.state() == fsm_state_slide_) {

            rectf_t b = bound();
            b.x0 -= 2;
            b.x1 += 2;

            vec2f_t res;
            if (service_->map_->collide(b, res)) {
                if (res.x > 0.f) {
                    pos_[1] += vec2f_t {3,-3};
                }
                if (res.x < 0.f) {
                    pos_[1] += vec2f_t {-3,-3};
                }
            }
        }
    }

    void move(float dx) {
        dx_ = dx;
    }

    virtual void tick() override {
        // tick the fsm
        if (fsm_.empty()) {
            fsm_.state_push(fsm_state_air_);
        }
        fsm_.tick();

        if (fsm_.state() == fsm_state_run_) {
            service_->draw_->colour_ = 0xff0000;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }
        if (fsm_.state() == fsm_state_air_) {
            service_->draw_->colour_ = 0x00ff00;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }
        if (fsm_.state() == fsm_state_slide_) {
            service_->draw_->colour_ = 0x0000ff;
            service_->draw_->rect(recti_t{8, 8, 16, 16});
        }

        // draw the player
        service_->draw_->colour_ = 0x406080;
        service_->draw_->rect(recti_t(bound()));
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
                    player_->cast<player_t>().jump();
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

        float dx = 0.f;
        {
            const uint8_t *key = SDL_GetKeyState(nullptr);
            dx += key[SDLK_LEFT] ? -1.f : 0.f;
            dx += key[SDLK_RIGHT] ? 1.f : 0.f;
        }

        player_t & player = player_->cast<player_t>();
        player.move(dx);

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
