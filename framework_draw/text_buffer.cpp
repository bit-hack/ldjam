#include <array>
#include <cstdarg>
#include "text_buffer.h"

namespace tengu {

text_buffer_t::text_buffer_t(const vec2i_t & size) {
    resize(size);
}

void text_buffer_t::resize(const vec2i_t & in) {
}

void text_buffer_t::putc(const vec2i_t & p, const uint8_t ch) {
}

void text_buffer_t::puts(const vec2i_t & p, const char * str, const uint32_t max) {
}

void text_buffer_t::put(const recti_t & rect, const uint8_t ch) {
}

void text_buffer_t::print(const vec2i_t & p, const char * fmt, ...) {
}

void text_buffer_t::putc(const uint8_t ch) {
}

void text_buffer_t::puts(const char * str, const uint32_t max) {
}

void text_buffer_t::print(const char * fmt, ...) {
    std::array<char, 512> buffer;
    va_list va;
    va_start(va, fmt);
    const int out = vsnprintf(buffer.data(), buffer.size(), fmt, va);
    assert(out >= 0 && out < buffer.size());
    this->puts(buffer.data(), out);
    va_end(va);
}

void text_buffer_t::copy(const recti_t & src, const recti_t & dst) {
}

void text_buffer_t::border(const recti_t & rect) {
}

void text_buffer_t::scroll(const vec2i_t & delta) {
}

} // namespace tengu
