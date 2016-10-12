/* Random Number Generation Library
 * Aidan Dodds 2016
 *
 * This library contains a bunch of random number routines with integer and
 * float output and a handful of different weightings.  These routines were
 * written for use in the context of game development and image processing.
 * These routines are in no way suitable for cryptographic purposes, or where
 * statistical accuracy is required.  Im a programmer, not a mathematician.
 *
 * I place this library in the public domain.
**/
#pragma once

#include <cmath>
#include <cstdint>

#include "common.h"

namespace tengu {
struct hash_t {

    // Thomas Wang 32bit
    static uint32_t wang_32(uint32_t z) {
        z = (z^61)^(z>>16);
        z *= 9;
        z = z^(z>>4);
        z *= 0x27d4eb2d;
        z = z^(z>>15);
        return z;
    }

    // Thomas Wang 64 bit
    static uint64_t wang_64(uint64_t key) {
        key += ~(key<<32);
        key ^= (key>>22);
        key += ~(key<<13);
        key ^= (key>>8);
        key += (key<<3);
        key ^= (key>>15);
        key += ~(key<<27);
        key ^= (key>>31);
        return key;
    }

    // Robert Jenkins
    static uint32_t jenkins_32(uint32_t a) {
        a = (a+0x7ed55d16)+(a<<12);
        a = (a^0xc761c23c)^(a>>19);
        a = (a+0x165667b1)+(a<<5);
        a = (a+0xd3a2646c)^(a<<9);
        a = (a+0xfd7046c5)+(a<<3);
        a = (a^0xb55a4f09)^(a>>16);
        return a;
    }

    static uint32_t string_1(const char *str) {
        uint32_t hash = 5381, c;
        for (; c = *str++; hash = hash * 33 + c);
        return hash;
    }
};

struct random_t {

    random_t(uint64_t seed)
        : x_(seed) {
    }

    /* random unsigned 64bit value (xorshift*) */
    template <typename type_t = uint64_t>
    type_t rand() {
        x_ ^= x_>>12;
        x_ ^= x_<<25;
        x_ ^= x_>>27;
        return type_t(x_*2685821657736338717ull);
    }

    /* random integer within a certain range */
    template <typename type_t>
    type_t rand_range(type_t min, type_t max) {
        const uint64_t out = rand();
        const type_t diff = max-min;
        return min+(diff!=0 ? out % diff : type_t(0));
    }

    /* return true with a certain probability */
    bool rand_chance(uint64_t chance) {
        return (rand()%chance)==0;
    }

    /* random value between 0.f and 1.f */
    float randfu() {
        union {
            float f;
            uint32_t i;
        } u;
        const uint32_t fmask = (1<<23)-1;
        u.i = (rand<uint32_t>() & fmask)|0x3f800000;
        return u.f-1.f;
    }

    /* random value between -1.f and 1.f */
    float randfs() {
        union {
            float f;
            uint32_t i;
        } u;
        const uint32_t fmask = (1<<23)-1;
        u.i = (rand<uint32_t>() & fmask)|0x40000000;
        return u.f-3.f;
    }

    /* triangular noise distribution [-1,+1] tending to 0 */
    float trandfs() {
        static const uint32_t fmask = (1<<23)-1;
        union {
            float f;
            uint32_t i;
        } u, v;
        u.i = (rand<uint32_t>() & fmask)|0x3f800000;
        v.i = (rand<uint32_t>() & fmask)|0x3f800000;
        float out = (u.f+v.f-3.f);
        return out;
    }

    /* gaussian signed random ~[-1,+1] tending to 0 */
    float gaussian() {
        // 1 / half_width
        static const float c_scale = 0.4246284f/1.5f;
        float sum = 0.f;
        sum += randfs();
        sum += randfs();
        sum += randfs();
        sum += randfs();
        return sum * c_scale;
    }

    /* pinch random ~[-1,+1] tending to 0 */
    float pinch() {
        return randfs() * randfu();
    }

    // random signed vec2
    template <typename type_t>
    void randfs_vec2(type_t & vec) {
        vec.x = randfs();
        vec.y = randfs();
    }

    // random signed vec3
    template <typename type_t>
    void randfs_vec3(type_t & vec) {
        vec.x = randfs();
        vec.y = randfs();
        vec.z = randfs();
    }

    /* circular rand
     *
     * this function generates a random 2d vector inside a unit circle.
     * |out| -> [0,1]
    **/
    void vrand2d(float(&out)[2]) {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            mag = out[0]*out[0]+out[1]*out[1];
        } while (mag>1.f);
    }

    /* spherical rand
     *
     * this function generates a random 3d vector inside a unit sphere.
     * |out| -> [0,1]
    **/
    void vrand3d(float(&out)[3]) {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            out[2] = randfs();
            mag = out[0]*out[0]+out[1]*out[1]+out[2]*out[2];
        } while (mag>1.f);
    }

    /* normalised circular rand
     *
     * this function generates a random 3d vector on the surface of a unit circle.
     * |out| = 1
    **/
    void nvrand2d(float(&out)[2]) {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            mag = out[0]*out[0]+out[1]*out[1];
        } while (mag>1.f);
        mag = 1.f/sqrtf(mag);
        out[0] *= mag;
        out[1] *= mag;
    }

    /* normalised spherical rand
     *
     * this function generates a random 3d vector on the surface of a unit sphere.
     * |out| = 1
    **/
    void nvrand3d(float(&out)[3]) {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            out[2] = randfs();
            mag = out[0]*out[0]+out[1]*out[1]+out[2]*out[2];
        } while (mag>1.f);
        mag = 1.f/sqrtf(mag);
        out[0] *= mag;
        out[1] *= mag;
        out[2] *= mag;
    }

protected:
    uint64_t x_;
};

struct perlin_t {

    // constructor
    perlin_t(uint32_t width, uint32_t height, uint32_t seed)
        : width_(width)
        , height_(height)
        , seed_(seed)
    {
    }

    // 2d unsigned float rand
    template <uint32_t(*hash)(uint32_t) = hash_t::wang_32>
    float rand2df(uint32_t x, uint32_t y) const {
        const uint32_t munge = (x+y * width_)+hash(seed_);
        union {
            float f;
            uint32_t i;
        } u;
        const uint32_t fmask = (1<<23)-1;
        u.i = (hash(munge) & fmask)|0x3f800000;
        return u.f-1.f;
    }

    // interpolated normalized rand2d (0.f -> 1.f)
    template <typename vec_t>
    float rand2df(const vec_t & v) const {
        // fractional position
        const std::array<float, 2> f = {
            v.x-quantize(v.x, 1),
            v.y-quantize(v.y, 1)
        };
        // find integer sample points
        const std::array<int32_t, 4> i = {
            quantize(v.x+0, 1), quantize(v.x+1, 1),
            quantize(v.y+0, 1), quantize(v.y+1, 1)
        };
        // sample rand at quadrant points
        const std::array<float, 4> q = {
            rand2df(i[0], i[2]), rand2df(i[1], i[2]),
            rand2df(i[0], i[3]), rand2df(i[1], i[3])
        };
        // x axis blend
        const std::array<float, 2> j = {
            slerp(q[0], q[1], f[0]),
            slerp(q[2], q[3], f[0])
        };
        // y axis blend
        return slerp(j[0], j[1], f[1]);
    }

    // perlin noise function
    template <typename vec_t>
    float perlin(const vec_t & v,
                 float smooth,
                 size_t octaves) const
    {
        float accum = 0.f, scale = 1.f;
        float x = v.x, y = v.y;
        for (size_t i = 0; i<octaves; ++i) {
            accum += rand2df(x, y) * scale;
            x *= .512367592345f;
            y *= .534968120897f;
            scale *= (smooth+1.f);
        }
        return accum;
    }

protected:
    uint32_t seed_;
    const uint32_t width_, height_;
};
} // namespace tengu
