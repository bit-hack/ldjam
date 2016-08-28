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

#include <math.h>
#include <stdint.h>

namespace prng {

    typedef uint64_t seed_t;

    namespace {

/* random unsigned 64bit value (xorshift*) */
        uint64_t randllu(seed_t &x) {
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;
            return uint64_t(x * 2685821657736338717ull);
        }

/*  */
        bool rand_chance(seed_t &x, uint64_t chance) {
            return (randllu(x)%chance)==0;
        }

/* random value between 0.f and 1.f */
        float randfu(seed_t &seed) {
            union {
                float f;
                uint32_t i;
            } u;
            const uint32_t fmask = (1 << 23) - 1;
            u.i = (randllu(seed) & fmask) | 0x3f800000;
            return u.f - 1.f;
        }

/* random value between -1.f and 1.f */
        float randfs(seed_t &seed) {
            union {
                float f;
                uint32_t i;
            } u;
            const uint32_t fmask = (1 << 23) - 1;
            u.i = (randllu(seed) & fmask) | 0x40000000;
            return u.f - 3.f;
        }

/* triangular noise distribution [-1,+1] tending to 0 */
        float trandfs(seed_t &x) {
            static const uint32_t fmask = (1 << 23) - 1;
            union {
                float f;
                uint32_t i;
            } u, v;
            u.i = (uint32_t(randllu(x)) & fmask) | 0x3f800000;
            v.i = (uint32_t(randllu(x)) & fmask) | 0x3f800000;
            float out = (u.f + v.f - 3.f);
            return out;
        }

/* gaussian signed random ~[-1,+1] tending to 0 */
        float grandfs(seed_t &x) {
            // 1 / half_width
            static const float c_scale = 0.4246284f / 2.f;
            float sum = 0.f;
            sum += randfs(x);
            sum += randfs(x);
            sum += randfs(x);
            sum += randfs(x);
            return sum * c_scale;
        }

/* circular rand
 *
 * this function generates a random 2d vector inside a unit circle.
 * |out| -> [0,1]
**/
        void vrand2d(seed_t &x, float (&out)[2]) {
            float mag;
            do {
                out[0] = randfs(x);
                out[1] = randfs(x);
                mag = out[0] * out[0] + out[1] * out[1];
            } while (mag > 1.f);
        }

/* spherical rand
 *
 * this function generates a random 3d vector inside a unit sphere.
 * |out| -> [0,1]
**/
        void vrand3d(seed_t &x, float (&out)[3]) {
            float mag;
            do {
                out[0] = randfs(x);
                out[1] = randfs(x);
                out[2] = randfs(x);
                mag = out[0] * out[0] + out[1] * out[1] + out[2] * out[2];
            } while (mag > 1.f);
        }

/* normalised circular rand
 *
 * this function generates a random 3d vector on the surface of a unit circle.
 * |out| = 1
**/
        void nvrand2d(seed_t &x, float (&out)[2]) {
            float mag;
            do {
                out[0] = randfs(x);
                out[1] = randfs(x);
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
        void nvrand3d(seed_t &x, float (&out)[3]) {
            float mag;
            do {
                out[0] = randfs(x);
                out[1] = randfs(x);
                out[2] = randfs(x);
                mag = out[0] * out[0] + out[1] * out[1] + out[2] * out[2];
            } while (mag > 1.f);
            mag = 1.f / sqrtf(mag);
            out[0] *= mag;
            out[1] *= mag;
            out[2] *= mag;
        }

/* 64bit hash function
 *
 * Thomas Wang's 64 bit Mix Function
**/
        uint64_t bitmix(seed_t key) {
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

    } // namespace {}
} // namespace prng
