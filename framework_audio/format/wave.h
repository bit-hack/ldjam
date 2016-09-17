#pragma once

#include <cstdint>
#include <memory>

#include "../../framework_core/buffer.h"

// todo:
// - add buffer space for interpolation

struct wave_t {

    static bool load_wav(const char * path, wave_t & out);

    uint32_t num_frames() const {
        const uint32_t sample_size = bit_depth_ / 8;
        const uint32_t num_samples = uint32_t(samples_.size()) / sample_size;
        const uint32_t num_frames = num_samples / channels_;
        return num_frames;
    }

    uint32_t num_channels() const {
        return channels_;
    }

    uint32_t bit_depth() const {
        return bit_depth_;
    }

    uint32_t sample_rate() const {
        return sample_rate_;
    }

    template <typename type_t>
    type_t * get() {
        return reinterpret_cast<type_t*>(
                samples_.get());
    }

    template <typename type_t>
    const type_t * get() const {
        return reinterpret_cast<const type_t*>(
                samples_.get());
    }

    uint32_t length() const {
        return uint32_t(samples_.size());
    }

protected:
    buffer_t samples_;
    uint32_t sample_rate_;
    uint32_t bit_depth_;
    uint32_t channels_;
};
