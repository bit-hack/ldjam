#include "camera.h"
#include "player.h"

camera_t::camera_t(object_service_t s)
    : object_ex_t()
    , service_(*reinterpret_cast<service_t*>(s))
    , pos_{32, 32}
{
}

void camera_t::tick() {
    // if we have a valid player
    if (service_.player_.valid()) {
        // get player state
        player_t & player = service_.player_->cast<player_t>();
        const vec2f_t pos = player.get_pos();
        const vec2f_t vel = player.get_velocity();
        // project where the player is going
        const vec2f_t proj = pos + vel * 12.f;
        // raycast from player to future point
        vec2f_t hit;
        if (service_.map_.raycast(pos + vec2f_t{0, -8}, proj, hit)) {
            hit = vec2f_t::nearest(pos, hit, proj);
            // ease towards map intersection
            target_ = vec2f_t::lerp(target_, hit, .5f);
        }
        else {
            // ease towards future projection
            target_ = vec2f_t::lerp(target_, proj, .5f);
        }
    }
    // smooth out camera position
    pos_ = vec2f_t::lerp(pos_, target_, 0.1f);
    service_.draw_.offset_ = vec2i_t(-pos_) + vec2i_t{ 80, 60 };
    // draw position and target
#if 0
    service_.draw_.colour_ = 0x509030;
    service_.draw_.plot<true>(vec2i_t(pos_));
    service_.draw_.plot<true>(vec2i_t(target_));
#endif
#if 0
    // draw screen frame
    service_.draw_.colour_ = 0x505050;
    service_.draw_.line<true>(pos_+vec2f_t{-64, -48}, pos_+vec2f_t{ 64, -48});
    service_.draw_.line<true>(pos_+vec2f_t{-64,  48}, pos_+vec2f_t{ 64,  48});
    service_.draw_.line<true>(pos_+vec2f_t{-64,  48}, pos_+vec2f_t{-64, -48});
    service_.draw_.line<true>(pos_+vec2f_t{ 64,  48}, pos_+vec2f_t{ 64, -48});
#endif
}
