#include <array>
#include <cstdarg>
#include <cstdio>

#include "../framework_core/common.h"
#include "draw.h"

namespace tengu {

void draw_t::printf(const font_t& font,
    const vec2i_t& pos,
    const char* fmt,
    ...)
{
    const uint32_t xfit = font.bitmap_->width() / font.cellw_;
    std::array<char, 1024> temp;
    blit_info_t info;
    info.type_ = e_blit_mask;
    info.bitmap_ = font.bitmap_;
    info.h_flip_ = false;
    info.dst_pos_ = vec2i_t{ pos.x, pos.y };
    va_list vl;
    va_start(vl, fmt);
    if (int j = vsnprintf(temp.data(), temp.size(), fmt, vl)) {
        if (j <= 0) {
            return;
        }
        j = minv<int32_t>(int32_t(temp.size()), j);
        for (int i = 0; i < j; i++) {
            const uint8_t ch = temp[i];
            info.src_rect_ = recti_t(
                int32_t((ch % xfit) * font.cellw_),
                int32_t((ch / xfit) * font.cellh_),
                (font.cellw_ - 1),
                (font.cellh_ - 1),
                recti_t::e_relative);
            blit(info);
            info.dst_pos_.x += font.spacing_;
        }
    }
    va_end(vl);
}

} // namespace tengu
