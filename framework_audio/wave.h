#pragma once

#include <cstdint>
#include <memory>


struct wave_t {

    static bool load_wav(const char * path, wave_t & out);

    uint32_t num_samples() const {
        return length_ / ((bit_depth_ * channels_) / 8);
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
        return reinterpret_cast<type_t*>(samples_.get());
    }

    template <typename type_t>
    const type_t * get() const {
        return reinterpret_cast<const type_t*>(samples_.get());
    }

    uint32_t length() const {
        return length_;
    }

protected:
    uint32_t length_; // in bytes
    std::unique_ptr<int8_t[]> samples_;
    uint32_t sample_rate_;
    uint32_t bit_depth_;
    uint32_t channels_;
};
