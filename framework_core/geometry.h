#pragma once
#include <cassert>

#include "common.h"
#include "rect.h"
#include "vec2.h"

namespace tengu {
namespace geometry {

// circle encapsulation
template <typename vec_t>
struct circle_t {
    vec_t p_;
    float r_;
};

// edge
template <typename vec_t>
struct edge_t {
    vec_t p0, p1;

    float distance(const vec_t& p) const
    {
        return vec_t::distance(p, project(p));
    }

    vec_t project(const vec_t& p) const
    {
        // project onto edge plane
        const vec_t delta = vec_t::normalize(p1 - p0);
        const float p_delta = delta * p;
        const float a_delta = delta * p0;
        const vec2f_t i = p0 + delta * (p_delta - a_delta);
        // find edge bounding box
        const vec_t bb_min = vec_t{ minv(p0.x, p1.x), minv(p0.y, p1.y) };
        const vec_t bb_max = vec_t{ maxv(p0.x, p1.x), maxv(p0.y, p1.y) };
        // clip to edge bounds
        return vec_t{
            minv(maxv(bb_min.x, i.x), bb_max.x),
            minv(maxv(bb_min.y, i.y), bb_max.y),
        };
    }
};

// inferred line
template <typename vec_t>
struct line_t {
    vec_t p0, p1;

    vec_t project(const vec_t& p) const
    {
        const vec_t delta = vec_t::normalize(p1 - p0);
        const float p_delta = delta * p;
        const float a_delta = delta * p0;
        return p0 + delta * (p_delta - a_delta);
    }

    float sideval(const vec_t& p) const
    {
        const vec_t norm = vec_t::cross(p1 - p0);
        return norm * (p - p0);
    }

    float distance(const vec_t& p) const
    {
        return vec_t::distance(p, project(p));
    }
};

// parametric line
template <typename vec_t>
struct pline_t {
    vec_t n_;
    float d_;

    pline_t(const vec_t& a,
        const vec_t& b)
        : n_(vec_t::normalize(vec_t::cross(b - a)))
        , d_(n_ * a)
    {
    }

    vec_t project(const vec_t& p) const
    {
        const vec_t t = vec_t::project(vec_t::cross(n_), p);
        return t + n_ * d_;
    }

    float distance(const vec_t& p) const
    {
        return p.x * n_.x + p.y * n_.y + d_;
    }
};

// pline -> pline intersection
template <typename vec_t>
bool intersect(
    const pline_t<vec_t>& a,
    const pline_t<vec_t>& b,
    vec_t& out)
{
    return false;
}

// pline -> edge intersection
template <typename vec_t>
bool intersect(
    const pline_t<vec_t>& l,
    const edge_t<vec_t>& e,
    vec_t& out)
{
    return false;
}

// edge -> edge intersection
template <typename vec_t>
bool intersect(
    const edge_t<vec_t>& a,
    const edge_t<vec_t>& b,
    vec_t& out)
{
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
    out = vec_t{ x, y };

    rectf_t ba = rectf_t::bound(a.p0, a.p1);
    rectf_t bb = rectf_t::bound(b.p0, b.p1);

    return ba.contains(out) && bb.contains(out);
}

// line -> edge intersection
template <typename vec_t>
bool intersect(
    const line_t<vec_t>& a,
    const edge_t<vec_t>& b,
    vec_t& out)
{

    const float x1 = a.p0.x, x2 = a.p1.x, x3 = b.p0.x, x4 = b.p1.x;
    const float y1 = a.p0.y, y2 = a.p1.y, y3 = b.p0.y, y4 = b.p1.y;

    // check for parallel lines
    const float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (d == 0.f) {
        return false;
    }

    // find intersection point
    const float pre = (x1 * y2 - y1 * x2);
    const float post = (x3 * y4 - y3 * x4);
    out = vec_t{
        (pre * (x3 - x4) - (x1 - x2) * post) / d,
        (pre * (y3 - y4) - (y1 - y2) * post) / d
    };

    if (out.x < minv(x3, x4) || out.x > maxv(x3, x4) || out.y < minv(y3, y4) || out.y > maxv(y3, y4)) {
        return false;
    }
    return true;
}

// line -> line intersection
template <typename vec_t>
bool intersect(
    const line_t<vec_t>& a,
    const line_t<vec_t>& b,
    vec_t& out)
{

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
    out = vec_t{ x, y };
    return true;
}

template <typename vec_t>
void split(
    const line_t<vec_t>& line,
    const edge_t<vec_t>& edge,
    valid_t<edge_t<vec_t>>& pos,
    valid_t<edge_t<vec_t>>& neg)
{

    const float dist_p0 = line.sideval(edge.p0);
    const float dist_p1 = line.sideval(edge.p1);

    // classify p0
    int32_t class_p0 = 0;
    if (!fp_cmp(dist_p0, 0.f)) {
        class_p0 = (dist_p0 > 0.f) ? 1 : -1;
    }

    // classify p1
    int32_t class_p1 = 0;
    if (!fp_cmp(dist_p1, 0.f)) {
        class_p1 = (dist_p1 > 0.f) ? 1 : -1;
    }

    // polarise coincident points
    if (class_p0 == 0)
        class_p0 = class_p1;
    if (class_p1 == 0)
        class_p1 = class_p0;

    // if edge is fully polarised
    if (class_p0 == class_p1) {
        switch (class_p0) {
        case (0):
        // both coincident
        case (1):
            // both positive
            pos = edge;
            return;
        case (-1):
            // both negative
            neg = edge;
            return;
        }
    } else {
        // edge is split
        vec_t mid;
        intersect(line, edge, mid);

        if (dist_p0 > 0.f) {
            pos = edge_t<vec_t>{ mid, edge.p0 };
            neg = edge_t<vec_t>{ edge.p1, mid };
        } else {
            pos = edge_t<vec_t>{ mid, edge.p1 };
            neg = edge_t<vec_t>{ edge.p0, mid };
        }
    }
}

typedef edge_t<vec2f_t> edgef_t;
typedef line_t<vec2f_t> linef_t;
typedef pline_t<vec2f_t> plinef_t;

} // geometry
} // namespace tengu
