#pragma once
#include <cstdint>
#include <cmath>

namespace {
const float c_renorm    = 1.0E-26f;
const float c_pi        = 3.14159265359f;
const float c_2pi       = 3.14159265359f * 2.f;

inline float note_to_freq(int32_t note) {
    float x = float(note-69)/12.f;
    float y = powf(2.f, x);
    return 440.f * y;
}

inline float lerp(float x, float y, float i) {
    return x+(y-x)*i;
}

template <typename type_t>
inline type_t _clamp(type_t lo, type_t in, type_t hi) {
    if (in<lo) return lo;
    if (in>hi) return hi;
    return in;
}

template <typename type_t>
inline type_t _min(type_t a, type_t b) {
    return (a<b) ? a : b;
}

template <typename type_t>
inline type_t _max(type_t a, type_t b) {
    return (a>b) ? a : b;
}

inline float hermite(float frac_pos, float xm1, float x0, float x1, float x2) {
    const float    c = (x1-xm1) * 0.5f;
    const float    v = x0-x1;
    const float    w = c+v;
    const float    a = w+v+(x2-x0) * 0.5f;
    const float    b_neg = w+a;
    return ((((a * frac_pos)-b_neg) * frac_pos+c) * frac_pos+x0);
}

inline float _fpart(float f) {
    return f-float(int32_t(f));
}

inline int32_t _ipart(float f) {
    return int32_t(f);
}
} // namespace {}
