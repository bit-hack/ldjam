#pragma once
#include <cassert>
#include <cstdint>
#include <memory>

struct bitmap_t {

    static bool load(const char * path,
                     bitmap_t & out);

    uint32_t width() const {
        return width_;
    }

    uint32_t height() const {
        return height_;
    }

    uint32_t * data() const {
        return pix_.get();
    }

protected:

    std::unique_ptr<uint32_t[]> pix_;
    uint32_t width_;
    uint32_t height_;
};
