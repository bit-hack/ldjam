#pragma once
#include <memory>
#include <array>

#include "../framework_core/vec2.h"
#include "../framework_core/rect.h"

namespace tengu {
struct text_buffer_t {

    text_buffer_t(const vec2i_t & size);

    void resize(const vec2i_t & in);

    void putc(const vec2i_t & p, const uint8_t ch);

    void puts(const vec2i_t & p, const char * str, const uint32_t max);

    void put(const recti_t & rect, const uint8_t ch);

    void print(const vec2i_t & p, const char * fmt, ...);

    void putc(const uint8_t ch);

    void puts(const char * str, const uint32_t max);

    void print(const char * fmt, ...);

    void copy(const recti_t & src, const recti_t & dst);

    void border(const recti_t & rect);

    void scroll(const vec2i_t & delta);

    const vec2i_t & size() const {
        return size_;
    }

    vec2i_t carret_;

protected:
    vec2i_t size_;
    std::unique_ptr<uint8_t[]> char_, attr_;
    std::array<uint32_t, 16> palette_;
};
} // tengu
