#pragma once
#include <cassert>
#include <cstdint>
#include <memory>

#include "../framework_core/buffer.h"
#include "../framework_core/rect.h"

struct bitmap_t {

    static bool create(const uint32_t width,
                       const uint32_t height,
                       bitmap_t & out);

    static bool load(const char * path,
                     bitmap_t & out);

    // todo: load from memory

    int32_t width() const {
        return int32_t(width_);
    }

    int32_t height() const {
        return int32_t(height_);
    }

    uint32_t * data() {
        return pix_.get<uint32_t>(0);
    }

    const uint32_t * data() const {
        return pix_.get<const uint32_t>(0);
    }

    void colour_key(uint32_t key);

    bool valid() const {
        return pix_.size() && pix_.get() != nullptr;
    }

    recti_t rect() const {
        return recti_t{0, 0, int32_t(width_)-1, int32_t(height_)-1};
    }

protected:
    buffer_t pix_;
    uint32_t width_;
    uint32_t height_;
};
