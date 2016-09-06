#pragma once

#include <cstdint>

#include "wave.h"

struct audio_t {

    struct play_wave_t {
        const wave_t *wave_;
        float volume_;
        float rate_;
        bool looping_;
        bool retrigger_;
    };

    struct play_vorbis_t {
        int dummy_;
    };

    audio_t();
    ~audio_t();

    bool play(const play_wave_t &);
    bool play(const play_vorbis_t &);

    bool render(int16_t *, uint32_t count);

protected:

    struct detail_t;
    std::unique_ptr<detail_t> detail_;
};
