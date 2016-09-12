#pragma once
#include <cassert>

#include "common.h"
#include "vec2.h"

namespace geometry {

    // edge
    template <typename vec_t>
    struct edge_t {
        const vec_t p0, p1;

        float distance(const vec_t & p) const {
            return 0;
        }
    };

    // inferred line
    template <typename vec_t>
    struct line_t {
        const vec_t p0, p1;
    };

    // parametric line
    template <typename vec_t>
    struct pline_t {
        const vec_t n_;
        const float d_;

        pline_t(const vec_t & a,
                const vec_t & b)
            : n_(vec_t::normalize(vec_t::cross(b-a)))
            , d_(-n_ * a)
        {
        }

        float distance(const vec_t & p) const {
            return p.x * n_.x + p.y * n_.y + d_;
        }
    };

    // pline -> pline intersection
    template <typename vec_t>
    bool intersect(
            const pline_t<vec_t> & a,
            const pline_t<vec_t> & b,
            vec_t & out) {
        return false;
    }

    // pline -> edge intersection
    template <typename vec_t>
    bool intersect(
            const pline_t<vec_t> & l, 
            const edge_t<vec_t> & e,
            vec_t & out) {
        return false;
    }

    // edge -> edge intersection
    template <typename vec_t>
    bool intersect(
            const edge_t<vec_t> & a,
            const edge_t<vec_t> & b,
            vec_t &out) {

        const float x1 = a.p0.x, x2 = a.p1.x, x3 = b.p0.x, x4 = b.p1.x;
        const float y1 = a.p0.y, y2 = a.p1.y, y3 = b.p0.y, y4 = b.p1.y;

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
    }

    // line -> edge intersection
    template <typename vec_t>
    bool intersect(
            const line_t<vec_t> &a,
            const edge_t<vec_t> &b,
            vec_t &out) {

        const float x1 = a.p0.x, x2 = a.p1.x, x3 = b.p0.x, x4 = b.p1.x;
        const float y1 = a.p0.y, y2 = a.p1.y, y3 = b.p0.y, y4 = b.p1.y;

        // check for parallel lines
        const float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (d == 0.f) {
            return false;
        }

        // find intersection point
        const float pre  = (x1 * y2 - y1 * x2);
        const float post = (x3 * y4 - y3 * x4);
        out = vec_t {
            (pre * (x3 - x4) - (x1 - x2) * post) / d,
            (pre * (y3 - y4) - (y1 - y2) * post) / d };

        if (out.x < min2(x3, x4) ||
            out.x > max2(x3, x4) ||
            out.y < min2(y3, y4) ||
            out.y > max2(y3, y4)) {
            return false;
        }
        return true;
    }
    
    // line -> line intersection
    template <typename vec_t>
    bool intersect(
            const line_t<vec_t> &a,
            const line_t<vec_t> &b,
            vec_t &out) {

        const float x1 = a.p0.x, x2 = a.p1.x, x3 = b.p0.x, x4 = b.p1.x;
        const float y1 = a.p0.y, y2 = a.p1.y, y3 = b.p0.y, y4 = b.p1.y;

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
        return true;
    }

} // geometry
