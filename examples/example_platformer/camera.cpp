#include "camera.h"
#include "player.h"

camera_t::camera_t(object_service_t s)
    : object_ex_t()
    , service_(*reinterpret_cast<service_t*>(s))
    , pos_{32, 32}
    , target_{0, 0}
{
    order_ = _ORDER;
}

void camera_t::shake(float magnitude) {
    shake_.mag_ = max2(shake_.mag_, magnitude);
}

void camera_t::tick() {
    // smoothign factors
    const float C_SMOOTH_1 = 0.5f;
    const float C_SMOOTH_2 = 0.1f;
    // if we have a valid player
    if (service_.objects_["player"].valid()) {
        // get player state
        player_t & player = service_.objects_["player"]->cast<player_t>();
        const vec2f_t player_pos = player.get_pos();
        const vec2f_t player_vel = player.get_velocity();
        // calculate an deadzone factor
        const float deadzone = max2(falloff(6.f, vec2f_t::distance(player_pos, pos_), 8.f), 
                                    falloff(1.f, vec2f_t::length(player_vel), 2.f));
#if 0
        // debug show movement factor
        service_.draw_.colour_ = speed*255;
        service_.draw_.circle<false>(vec2i_t{8, 8}, 3);
#endif
        // project where the player is going
        const vec2f_t proj = player_pos + player_vel * 12.f;
        // raycast from player to future point
        vec2f_t hit;
        if (service_.map_.raycast(player_pos + vec2f_t{0, -8}, proj, hit)) {
            hit = vec2f_t::nearest(player_pos, hit, proj);
            // ease towards map intersection point
            target_ = vec2f_t::lerp(target_, hit, C_SMOOTH_1);
        }
        else {
            // ease towards future projection
            target_ = vec2f_t::lerp(target_, proj, C_SMOOTH_1);
        }
        // smooth out camera position
        const vec2f_t new_pos = vec2f_t::lerp(pos_, target_, C_SMOOTH_2);
        // final update based on deadzone
        pos_ = vec2f_t::lerp(pos_, new_pos, deadzone);
    }
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
    {
        shake_.mag_ *= 0.8f;


        // update camera shake
        if (shake_.counter_.frame()) {
            shake_.offset_[0] = shake_.offset_[1];
            shake_.offset_[1];
            shake_.rand_.randfs_vec2(shake_.offset_[1]);
        }
        const float dt = shake_.counter_.deltaf();
        vec2f_t rumble = vec2f_t::lerp(shake_.offset_[0], shake_.offset_[1], dt) * shake_.mag_;
        service_.draw_.offset_ = vec2i_t(-pos_) + vec2i_t{ 80, 60 } + vec2i_t(rumble);
    }
}
