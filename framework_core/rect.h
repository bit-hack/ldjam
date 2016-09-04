#pragma once

#include <cstdint>
#include "../framework_core/common.h"

template <typename type_t>
struct rect_t {
    type_t x0, y0;
    type_t x1, y1;

    static rect_t intersect(const rect_t & a,
                            const rect_t & b) {
        return rect_t{
            maxv(a.x0, b.x0),
            maxv(a.y0, b.y0),
            minv(a.x1, b.x1),
            minv(a.y1, b.y1)
        };
    }

    static rect_t combine(const rect_t & a,
                          const rect_t & b) {
        return rect_t{
                minv(a.x0, b.x0),
                minv(a.y0, b.y0),
                maxv(a.x1, b.x1),
                maxv(a.y1, b.y1)
        };
    }

    template <typename vec_t>
    bool contains(const vec_t & v) {
        return (v.x>=x0) && (v.x<=x1) && (v.y>=y0) && (v.y<=y1);
    }
};

typedef rect_t<int32_t> recti_t;
