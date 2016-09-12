#pragma once

#include <cstdint>
#include "../framework_core/common.h"

template <typename type_t>
struct rect_t {
    type_t x0, y0;
    type_t x1, y1;

    template <typename vec_t>
    static rect_t bound(const vec_t & a, const vec_t & b) {
        return rect_t{
            min2(a.x, b.x),
            min2(a.y, b.y),
            max2(a.x, b.x),
            max2(a.y, b.y)
        };
    }

    static rect_t intersect(const rect_t & a,
                            const rect_t & b) {
        return rect_t{
            max2(a.x0, b.x0),
            max2(a.y0, b.y0),
            min2(a.x1, b.x1),
            min2(a.y1, b.y1)
        };
    }

    static rect_t combine(const rect_t & a,
                          const rect_t & b) {
        return rect_t{
            min2(a.x0, b.x0),
            min2(a.y0, b.y0),
            max2(a.x1, b.x1),
            max2(a.y1, b.y1)
        };
    }

    enum classify_t {
        e_rect_inside,
        e_rect_outside,
        e_rect_overlap
    };

    classify_t classify(const rect_t & a) const {
        if (a.x0 >= x0 && a.x1 <= x1 && a.y0 >= y0 && a.y1 <= y1)
            return e_rect_inside;
        else if (a.x0 > x1 || a.x1 < x0 || a.y0 > y1 || a.y1 < y0)
            return e_rect_outside;
        else
            return e_rect_overlap;
    }

    template <typename vec_t>
    bool contains(const vec_t & v) {
        return (v.x>=x0) && (v.x<=x1) && (v.y>=y0) && (v.y<=y1);
    }

    type_t dx() const {
        return x1 - x0;
    }

    type_t dy() const {
        return y1 - y0;
    }

    template <typename vec_t>
    vec_t size() const {
        return vec_t{dx(), dy()};
    }
};

typedef rect_t<int32_t> recti_t;
typedef rect_t<float> rectf_t;
