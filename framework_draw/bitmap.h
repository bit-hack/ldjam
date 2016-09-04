#pragma once
#include <cassert>
#include <cstdint>
#include <memory>

struct bitmap_t {

    static bool create(const uint32_t width,
                       const uint32_t height,
                       bitmap_t & out);

    static bool load(const char * path,
                     bitmap_t & out);

    int32_t width() const {
        return int32_t(width_);
    }

    int32_t height() const {
        return int32_t(height_);
    }

    uint32_t * data() const {
        return pix_.get();
    }

    void colour_key(uint32_t key);

    bool valid() const {
        return pix_.get() != nullptr;
    }

protected:

    std::unique_ptr<uint32_t[]> pix_;
    uint32_t width_;
    uint32_t height_;
};
