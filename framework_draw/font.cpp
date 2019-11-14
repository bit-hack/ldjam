#include <intrin.h>

#include "font.h"

#include "../framework_core/common.h"

namespace {
#if 1
static inline uint32_t alpha_blend(
    const uint32_t a,
    const uint32_t b,
    const uint8_t i)
{

    __m128i a1 = _mm_set1_epi32(a);
    __m128i b1 = _mm_set1_epi32(b);

    __m128i a2 = _mm_unpacklo_epi8(a1, _mm_setzero_si128());
    __m128i b2 = _mm_unpacklo_epi8(b1, _mm_setzero_si128());

    __m128i i1 = _mm_broadcastw_epi16(_mm_set1_epi16(i));
    __m128i i2 = _mm_broadcastw_epi16(_mm_set1_epi16(255 - i));

    __m128i a3 = _mm_mulhi_epi16(i1, a2);
    __m128i b3 = _mm_mulhi_epi16(i2, b2);


    const uint8_t j = 255 - i;
    // pixel a
    const uint32_t a0 = ((a & 0xff00ff) * j) >> 8;
    const uint32_t a1 = ((a & 0x00ff00) * j) >> 8;
    // pixel b
    const uint32_t b0 = ((b & 0xff00ff) * i) >> 8;
    const uint32_t b1 = ((b & 0x00ff00) * i) >> 8;
    // mix results
    return ((a0 & 0xff00ff) | (a1 & 0xff00)) + ((b0 & 0xff00ff) | (b1 & 0xff00));
}
#else
static inline uint32_t alpha_blend(
    const uint32_t a,
    const uint32_t b,
    const uint8_t i)
{
    const uint8_t j = 255 - i;
    // pixel a
    const uint32_t a0 = ((a & 0xff00ff) * j) >> 8;
    const uint32_t a1 = ((a & 0x00ff00) * j) >> 8;
    // pixel b
    const uint32_t b0 = ((b & 0xff00ff) * i) >> 8;
    const uint32_t b1 = ((b & 0x00ff00) * i) >> 8;
    // mix results
    return ((a0 & 0xff00ff) | (a1 & 0xff00)) + ((b0 & 0xff00ff) | (b1 & 0xff00));
}
#endif
} // namespace {}

namespace tengu {

bool ttf_font_t::load(const char* path, float size)
{
    buffer_t buffer;
    if (!buffer.load(path)) {
        return false;
    }
    stbtt_BakeFontBitmap(buffer.data(), 0, size, data, bitmap_w, bitmap_h, 0, 128, glyph);
    _valid = true;
    return true;
}

void ttf_font_t::render(const target_t& t,
    const vec2i_t& p,
    const std::string& out,
    uint32_t rgb)
{
    if (!_valid) {
        return;
    }
    float xpos = p.x;
    for (uint32_t i = 0; i < out.length(); ++i) {
        const char ch = out[i];
        const auto& g = glyph[ch & 0x7f];

        int32_t x0 = xpos + g.xoff;
        int32_t y0 = p.y + g.yoff;
        int32_t x1 = std::min(x0 + g.x1 - g.x0, t.viewport.x1);
        int32_t y1 = std::min(y0 + g.y1 - g.y0, t.viewport.y1);

        const int32_t nx = std::max<int32_t>(0, t.viewport.x0 - x0);
        const int32_t ny = std::max<int32_t>(0, t.viewport.y0 - y0);

        uint8_t* src = data + g.x0 + g.y0 * bitmap_w;
        src += nx + ny * bitmap_w;

        uint32_t* dst = t.dst;
        dst += (y0 + ny) * t.pitch;

        x0 += nx;
        y0 += ny;

        for (uint32_t y = y0; y < y1; ++y) {
            uint32_t sx = 0;
            for (uint32_t x = x0; x < x1; ++x, ++sx) {
                dst[x] = alpha_blend(dst[x], rgb, src[sx]);
            }
            src += bitmap_w;
            dst += t.pitch;
        }

        xpos += g.xadvance;
    }
}

bool ttf_font_t::get_bound(const std::string& text, recti_t& bound) const
{
    if (!_valid) {
        return false;
    }
    bound = recti_t{ 0, 0, 0, 0 };

    float xpos = 0;
    for (uint32_t i = 0; i < text.length(); ++i) {
        const char ch = text[i];
        const auto& g = glyph[ch & 0x7f];

        const int32_t dx = g.y1 - g.y0;
        const int32_t dy = g.x1 - g.x0;

        bound = recti_t::combine(bound,
            recti_t{ int32_t(g.xoff + xpos),
                int32_t(g.yoff),
                int32_t(g.xoff + dx + xpos),
                int32_t(g.yoff + dy) });

        xpos += g.xadvance;
    }
    return true;
}
} // namespace tengu
