#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>

#include "../framework_core/common.h"

namespace tengu {
template <typename type_t>
struct rect_t {
    type_t x0, y0;
    type_t x1, y1;

    enum mode_t {
        e_relative, // x1 relative to x0, etc
        e_absolute // x1 not relative to x0, etc
    };

    rect_t() = default;

    rect_t(const std::initializer_list<type_t>& l)
    {
        assert(l.size() == 4);
        const type_t* a = l.begin();
        x0 = a[0];
        y0 = a[1];
        x1 = a[2];
        y1 = a[3];
    }

    rect_t(type_t a,
        type_t b,
        type_t c,
        type_t d,
        mode_t mode = e_absolute)
        : x0(a)
        , y0(b)
        , x1(mode == e_relative ? a + c : c)
        , y1(mode == e_relative ? b + d : d)
    {
    }

    template <typename other_t>
    explicit rect_t(const rect_t<other_t>& other)
        : x0(type_t(other.x0))
        , y0(type_t(other.y0))
        , x1(type_t(other.x1))
        , y1(type_t(other.y1))
    {
    }

    /* find a rect which bounds two points
     */
    template <typename vec_t>
    static rect_t bound(const vec_t& a, const vec_t& b)
    {
        return rect_t{
            minv(a.x, b.x),
            minv(a.y, b.y),
            maxv(a.x, b.x),
            maxv(a.y, b.y)
        };
    }

    /* find the intersection of two rects
     */
    static rect_t intersect(const rect_t& a,
        const rect_t& b)
    {
        return rect_t{
            maxv(a.x0, b.x0),
            maxv(a.y0, b.y0),
            minv(a.x1, b.x1),
            minv(a.y1, b.y1)
        };
    }

    /* find the combined size of two rects
     */
    static rect_t combine(const rect_t& a,
        const rect_t& b)
    {
        return rect_t{
            minv(a.x0, b.x0),
            minv(a.y0, b.y0),
            maxv(a.x1, b.x1),
            maxv(a.y1, b.y1)
        };
    }

    enum classify_t {
        e_rect_inside,
        e_rect_outside,
        e_rect_overlap
    };

    /* test for levels of intersection between two rects
     */
    classify_t classify(const rect_t& a) const
    {
        if (a.x0 >= x0 && a.x1 <= x1 && a.y0 >= y0 && a.y1 <= y1)
            return e_rect_inside;
        else if (a.x0 > x1 || a.x1 < x0 || a.y0 > y1 || a.y1 < y0)
            return e_rect_outside;
        else
            return e_rect_overlap;
    }

    /* check if a point is contained by this rect
     */
    template <typename vec_t>
    bool contains(const vec_t& v) const
    {
        return (v.x >= x0) && (v.x <= x1) && (v.y >= y0) && (v.y <= y1);
    }

    /* projected size on x axis
     */
    type_t dx() const
    {
        return x1 - x0;
    }

    /* projected size on y axis
     */
    type_t dy() const
    {
        return y1 - y0;
    }

    /* size of this rect as a vector
     */
    template <typename vec_t>
    vec_t size() const
    {
        return vec_t{ width(), height() };
    }

    /*
     */
    type_t width() const
    {
        // +1 because x1 is inclusive
        return (maxv(x1, x0) + 1) - minv(x1, x0);
    }

    /*
     */
    type_t height() const
    {
        // +1 because y1 is inclusive
        return (maxv(y1, y0) + 1) - minv(y1, y0);
    }

    /* return the area of this rect
     */
    type_t area() const
    {
        return width() * height();
    }

    /* scale this rect
     */
    rect_t operator*(const type_t scale) const
    {
        return rect_t{
            x0 * scale,
            y0 * scale,
            x1 * scale,
            y1 * scale
        };
    }

    /* scale this rect
     */
    rect_t operator/(const type_t scale) const
    {
        return rect_t{
            x0 / scale,
            y0 / scale,
            x1 / scale,
            y1 / scale
        };
    }

    /* sum the components of two rects
     */
    rect_t operator+(const rect_t& rhs) const
    {
        return rect_t{
            x0 + rhs.x0,
            y0 + rhs.y0,
            x1 + rhs.x1,
            y1 + rhs.y1
        };
    }

    /* move rect via vector
     */
    template <typename vec_t>
    rect_t operator+(const vec_t& rhs) const
    {
        return rect_t{
            x0 + rhs.x,
            y0 + rhs.y,
            x1 + rhs.x,
            y1 + rhs.y
        };
    }
};

typedef rect_t<int32_t> recti_t;
typedef rect_t<float> rectf_t;
} // namespace tengu
