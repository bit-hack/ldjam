#pragma once

#include <cstdint>
#include <memory>

struct sound_t {
    uint32_t length_; // in bytes
    std::unique_ptr<int8_t[]> samples_;
    uint32_t sample_rate_;
    uint32_t bit_depth_;
    uint32_t channels_;
};
