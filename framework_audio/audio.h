#pragma once
#include <memory>
#include <cstdint>

#include "queue.h"
#include "format/wave.h"
#include "format/vorbis.h"

struct mix_out_t {
    int32_t * left_;
    int32_t * right_;
    const size_t count_;
};

struct audio_source_t {

    virtual void render(const mix_out_t &) = 0;
};

struct audio_t {

    // must be done before we start rendering
    void add(audio_source_t * src) {
        source_.push_back(src);
    }

    bool render(int16_t *, uint32_t count);

protected:

    // clip sample to signed 16bit integer range
    static constexpr int16_t clip(int32_t x) {
        return int16_t(x >  0x7fff ?  0x7fff :
                      (x < -0x7fff ? -0x7fff : x));
    }

    void _mixdown(int16_t * out, const uint32_t count);

    static const uint32_t C_BUFFER_SIZE = 1024;

    struct {
        std::array<int32_t, C_BUFFER_SIZE> left_;
        std::array<int32_t, C_BUFFER_SIZE> right_;
    } mix_;

    std::vector<audio_source_t*> source_;
};
