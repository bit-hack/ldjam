#pragma once
#include <stdint.h>

#include "../external/stb_ttf/stb_truetype.h"

#include "../framework_core/buffer.h"
#include "../framework_core/vec2.h"
#include "../framework_core/rect.h"

namespace tengu {
struct ttf_font_t {

  ttf_font_t()
    : _valid(false)
  {}

  struct target_t {
    uint32_t *dst;
    recti_t viewport;
    uint32_t pitch;
  };

  static const uint32_t bitmap_w = 512;
  static const uint32_t bitmap_h = 512;

  stbtt_bakedchar glyph[128];
  uint8_t data[bitmap_w * bitmap_h];

  bool load(const char* path, float size);

  void render(const target_t &t,
              const vec2i_t &p,
              const std::string& out,
              uint32_t rgb);

  bool get_bound(const std::string& text, recti_t& bound) const;

  bool valid() const {
    return _valid;
  }

private:
  bool _valid;
};
} // namespace tengu
