#pragma once
#include <cassert>

#include "common.h"

namespace geometry {

    enum intersect_mode_t {
        e_edge_edge,
        e_line_edge,
        e_line_line,
    };

    // edge/line intersection
    template<typename vec_t, intersect_mode_t C_MODE>
    bool intersect(
            const vec_t &a1,
            const vec_t &a2,
            const vec_t &b1,
            const vec_t &b2,
            vec_t &out) {
        static_assert(
                C_MODE == e_edge_edge ||
                C_MODE == e_line_edge ||
                C_MODE == e_line_line,
                "unsupported intersection mode");

        const float x1 = a1.x, x2 = a2.x, x3 = b1.x, x4 = b2.x;
        const float y1 = a1.y, y2 = a2.y, y3 = b1.y, y4 = b2.y;

        // check for parallel lines
        const float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (d == 0.f) {
            return false;
        }

        // Get the x and y for the intersection
        const float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
        const float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
        const float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

        // copy out intersection point
        out = vec_t {x, y};

        switch (C_MODE) {
        case (e_edge_edge):
            if (x < min2(x1, x2) ||
                x > max2(x1, x2) ||
                y < min2(y1, y2) ||
                y > max2(y1, y2)) {
                return false;
            }
            if (x < min2(y3, y4) ||
                x > max2(y3, y4) ||
                y < min2(y3, y4) ||
                y > max2(y3, y4)) {
                return false;
            }
            return true;

        case (e_line_edge):
            if (x < min2(x3, x4) ||
                x > max2(x3, x4) ||
                y < min2(y3, y4) ||
                y > max2(y3, y4)) {
                return false;
            }
            return true;

        case (e_line_line):
        default:
            return true;
        }
    }

    // point vs line side determination
    template<typename vec_t>
    float side(
            const vec_t &a1,
            const vec_t &a2,
            const vec_t &p) {
        // find normal and dot with point vector
        return vec_t::cross(a2 - a1) * (p - a1);
    }

    // circle vs line distance
    template<typename vec_t>
    float distance(
            const vec_t & a1,
            const vec_t & a2,
            const vec_t & c,
            const float r) {
        return 0.f;
    }

} // geometry
