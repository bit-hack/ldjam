#pragma once
#include <cmath>

namespace
{
    float sqrt(const float & x)
    {
        return sqrtf(x);
    }

    float isqrt(const float & val)
    {
        const float threehalfs = 1.5f;
        float x2 = val * 0.5f;
        float y  = val;
        long i  = *(long*) &y;
        i  = 0x5f3759df - (i >> 1);
        y  = * (float *) &i;
        y  = y * (threehalfs - (x2 * y * y));
        return y;
    }
}

template <typename type_t>
struct vec3_t
{
    type_t x, y, z;

    void operator += (const vec3_t & v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }

    void operator -= (const vec3_t & v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }

    void operator *= (const type_t v)
    {
        x *= v;
        y *= v;
        z *= v;
    }

    void operator /= (const type_t v)
    {
        x /= v;
        y /= v;
        z /= v;
    }

    static type_t length(const vec3_t & v)
    {
        return sqrt(v.x*v.x + v.y*v.y  + v.z*v.z);
    }

    static vec3_t normalize(const vec3_t & v)
    {
        const type_t l = length(v);
        return vec3_t {
            v.x / l,
            v.y / l,
            v.z / l
        };
    }

    static vec3_t cross(
        const vec3_t & a,
        const vec3_t & b)
    {
        return vec3_t {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static type_t distance(
        const vec3_t & a,
        const vec3_t & b)
    {
        return length(b - a);
    }

    static vec3_t zero() {
        return vec3_t {
            0, 0, 0
        };
    }

    // project v onto u
    static vec3_t project(
        const vec3_t & u,
        const vec3_t & v)
    {
        const type_t vu = v * u;
        const type_t uu = u * u;
        return u * (vu / uu);
    }

    static vec3_t rotate_z(
        const vec3_t & x,
        const type_t angle)
    {
        const type_t s = sinf(angle);
        const type_t c = cosf(angle);
        return vec3_t {
            c * x.x + s * x.y,
            c * x.y - s * x.x,
            x.z
        };
    }

    static vec3_t lerp(
        const vec3_t & a,
        const vec3_t & b,
        const type_t i
    )
    {
        return a + (b - a) * i;
    }
};

namespace {

    template <typename type_t>
    vec3_t<type_t> operator + (
        const vec3_t<type_t> & a,
        const vec3_t<type_t> & b)
    {
        return vec3_t<type_t> {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        };
    }

    template <typename type_t>
    vec3_t<type_t> operator - (
        const vec3_t<type_t> & a,
        const vec3_t<type_t> & b)
    {
        return vec3_t<type_t> {
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
        };
    }

    template <typename type_t>
    type_t operator * (
        const vec3_t<type_t> & a,
        const vec3_t<type_t> & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename type_t>
    vec3_t<type_t> operator * (
        const vec3_t<type_t> & a,
        const type_t s)
    {
        return vec3_t<type_t> {
            a.x * s,
            a.y * s,
            a.z * s
        };
    }

    template <typename type_t>
    vec3_t<type_t> operator * (
        const type_t s,
        const vec3_t<type_t> & a)
    {
        return vec3_t<type_t> {
            a.x * s,
            a.y * s,
            a.z * s
        };
    }

    template <typename type_t>
    vec3_t<type_t> operator / (
        const vec3_t<type_t> & a,
        const type_t s)
    {
        return vec3_t<type_t> {
            a.x / s,
            a.y / s,
            a.z / s
        };
    }

    template <typename type_t>
    vec3_t<type_t> operator - (
        const vec3_t<type_t> & a)
    {
        return vec3_t<type_t> {
            -a.x,
            -a.y,
            -a.z
        };
    }

    typedef vec3_t<float> vec3f_t;

} // namespace {}
