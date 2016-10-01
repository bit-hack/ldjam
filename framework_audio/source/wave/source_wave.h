#pragma once
#include <map>

#include "../../audio.h"

struct audio_source_wave_t :
        public audio_source_t {

    virtual void render(const mix_out_t &) override;

    struct play_wave_t {
        const struct wave_t *wave_;
        float volume_;
        float rate_;
        bool looping_;
        bool retrigger_;
    };

    bool play(const play_wave_t & info) {
        queue_.push(info);
        return true;
    }

protected:
    struct info_wave_t {
        const wave_t * wave_;
        uint64_t frame_;    // 0xffff fixed point
        uint64_t delta_;    // 0xffff fixed point
        uint16_t volume_;   // 0x00ff fixed point
        bool looping_;
    };

    bool _render(const mix_out_t & mix, info_wave_t & wave);
    void _check_messages();

    bool _render_8_1(
            const mix_out_t & mix,
            info_wave_t & info);

    bool _render_8_2(
            const mix_out_t & mix,
            info_wave_t & info);

    bool _render_16_1(
            const mix_out_t & mix,
            info_wave_t & info);

    bool _render_16_2(
            const mix_out_t & mix,
            info_wave_t & info);

    std::map<const wave_t*, info_wave_t> waves_;
    queue_t<play_wave_t> queue_;
};
