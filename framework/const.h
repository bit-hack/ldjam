#pragma once

namespace {
    const float C_PI = 3.14159265359f;
    const float C_2PI = 6.28318530718f;

    template <typename type_t>
    constexpr float lerp(type_t x, type_t y, type_t i) {
        return x+(y-x) * i;
    }

    constexpr float fpart(const float & in) {
        return in - float(int(in));
    }

} // namespace {}
