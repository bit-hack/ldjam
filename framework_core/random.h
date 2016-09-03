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

struct random_t {

    random_t(uint64_t seed)
        : x_(seed)
    {
    }

    /* random unsigned 64bit value (xorshift*) */
    uint64_t randllu()
    {
        x_ ^= x_ >> 12;
        x_ ^= x_ << 25;
        x_ ^= x_ >> 27;
        return uint64_t(x_ * 2685821657736338717ull);
    }

    /* random integer within a certain range */
    template <typename type_t>
    type_t rand_range(type_t min, type_t max)
    {
        const uint64_t out = randllu();
        const type_t diff = max - min;
        return min + (diff != 0 ? out % diff : type_t(0));
    }

    /* return true with a certain probability */
    bool rand_chance(uint64_t chance)
    {
        return (randllu() % chance) == 0;
    }

    /* random value between 0.f and 1.f */
    float randfu()
    {
        union {
            float f;
            uint32_t i;
        } u;
        const uint32_t fmask = (1 << 23) - 1;
        u.i = (randllu() & fmask) | 0x3f800000;
        return u.f - 1.f;
    }

    /* random value between -1.f and 1.f */
    float randfs()
    {
        union {
            float f;
            uint32_t i;
        } u;
        const uint32_t fmask = (1 << 23) - 1;
        u.i = (randllu() & fmask) | 0x40000000;
        return u.f - 3.f;
    }

    /* triangular noise distribution [-1,+1] tending to 0 */
    float trandfs()
    {
        static const uint32_t fmask = (1 << 23) - 1;
        union {
            float f;
            uint32_t i;
        } u, v;
        u.i = (uint32_t(randllu()) & fmask) | 0x3f800000;
        v.i = (uint32_t(randllu()) & fmask) | 0x3f800000;
        float out = (u.f + v.f - 3.f);
        return out;
    }

    /* gaussian signed random ~[-1,+1] tending to 0 */
    float grandfs()
    {
        // 1 / half_width
        static const float c_scale = 0.4246284f / 2.f;
        float sum = 0.f;
        sum += randfs();
        sum += randfs();
        sum += randfs();
        sum += randfs();
        return sum * c_scale;
    }

    /* circular rand
     *
     * this function generates a random 2d vector inside a unit circle.
     * |out| -> [0,1]
    **/
    void vrand2d(float(&out)[2])
    {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            mag = out[0] * out[0] + out[1] * out[1];
        } while (mag > 1.f);
    }

    /* spherical rand
     *
     * this function generates a random 3d vector inside a unit sphere.
     * |out| -> [0,1]
    **/
    void vrand3d(float(&out)[3])
    {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            out[2] = randfs();
            mag = out[0] * out[0] + out[1] * out[1] + out[2] * out[2];
        } while (mag > 1.f);
    }

    /* normalised circular rand
     *
     * this function generates a random 3d vector on the surface of a unit circle.
     * |out| = 1
    **/
    void nvrand2d(float(&out)[2])
    {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            mag = out[0] * out[0] + out[1] * out[1];
        } while (mag > 1.f);
        mag = 1.f / sqrtf(mag);
        out[0] *= mag;
        out[1] *= mag;
    }

    /* normalised spherical rand
     *
     * this function generates a random 3d vector on the surface of a unit sphere.
     * |out| = 1
    **/
    void nvrand3d(float(&out)[3])
    {
        float mag;
        do {
            out[0] = randfs();
            out[1] = randfs();
            out[2] = randfs();
            mag = out[0] * out[0] + out[1] * out[1] + out[2] * out[2];
        } while (mag > 1.f);
        mag = 1.f / sqrtf(mag);
        out[0] *= mag;
        out[1] *= mag;
        out[2] *= mag;
    }

    /* 64bit hash function
     * Thomas Wang's 64 bit Mix Function
    **/
    static uint64_t hash64(uint64_t key)
    {
        key += ~(key << 32);
        key ^= (key >> 22);
        key += ~(key << 13);
        key ^= (key >> 8);
        key += (key << 3);
        key ^= (key >> 15);
        key += ~(key << 27);
        key ^= (key >> 31);
        return key;
    }

    /* 32bit hash function
     * Robert Jenkins
    **/
    static uint32_t hash32(uint32_t a) {
        a = (a+0x7ed55d16) + (a<<12);
        a = (a^0xc761c23c) ^ (a>>19);
        a = (a+0x165667b1) + (a<<5);
        a = (a+0xd3a2646c) ^ (a<<9);
        a = (a+0xfd7046c5) + (a<<3);
        a = (a^0xb55a4f09) ^ (a>>16);
        return a;
    }

protected:
    uint64_t x_;
};
