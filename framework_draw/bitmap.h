#pragma once
#include <cassert>
#include <cstdint>
#include <memory>

#include "../framework_core/buffer.h"
#include "../framework_core/rect.h"
#include "../framework_core/vec2.h"

namespace tengu {
struct bitmap_t {

    bool create(const vec2i_t & size);

    bool load(const char * path);

    bool free();

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
        const uint32_t * pix = pix_.get<const uint32_t>();
        assert(pix);
        return pix;
    }

    void colour_key(const uint32_t key);

    operator bool () const {
        return valid();
    }

    bool valid() const {
        return pix_.size()&&pix_.get()!=nullptr;
    }

    recti_t rect() const {
        return recti_t{0, 0, int32_t(width_)-1, int32_t(height_)-1};
    }

protected:
    buffer_t pix_;
    uint32_t width_;
    uint32_t height_;
};
} // namespace tengu
