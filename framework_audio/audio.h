#pragma once
#include <memory>
#include <cstdint>

#include "wave.h"
#include "vorbis.h"

struct audio_t {

    struct play_wave_t {
        const struct wave_t *wave_;
        float volume_;
        float rate_;
        bool looping_;
        bool retrigger_;
    };

    struct play_vorbis_t {
        const char * name_;
        const struct vorbis_t * vorbis_;
        bool loop_;
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
