#pragma once

#include <cstdint>
#include <array>
#include <cmath>

namespace tengu {
    template<typename type_t,
            float (*sin_t)(float),
            float (*cos_t)(float)>
    struct mat2_t {

        type_t e[4];

        // scale in principal axies
        mat2_t scale(float x, float y) const {
            return mat2_t {
                e[0] * x, e[1] * y,
                e[2] * x, e[3] * y
            };
        }

        // return matrix inverse
        mat2_t invert() const {
            const float denom = e[0] * e[3] - e[1] * e[2];
            const float det = 1.f / denom;
            return mat2_t {
                 det * e[3], -det * e[1],
                -det * e[2],  det * e[0]
            };
        }

        // return matrix transpose
        mat2_t transpose() const {
            return mat2_t{
                e[0], e[2],
                e[1], e[3]
            };
        }

        // index matrix array
        type_t &operator[](size_t index) {
            assert(index < 4);
            return e[index];
        }

        // index matrix array
        const type_t &operator[](size_t index) const {
            assert(index < 4);
            return e[index];
        }

        // identity matrix
        static mat2_t identity() {
            return mat2_t{
                1, 0,
                0, 1
            };
        }

        // make rotation matrix
        static mat2_t rotate(type_t angle) {
            const type_t s = sin_t(angle), c = cos_t(angle);
            return mat2_t{
                 s, c,
                -c, s
            };
        }

        // matrix matrix multiply
        static mat2_t multiply(const mat2_t & a, const mat2_t & b) {
            const float *ae = a.e, *be = b.e;
            return mat2_t {
                ae[0]*be[0]+ae[1]*be[2], ae[0]*be[1]+ae[1]*be[3],
                ae[1]*be[0]+ae[3]*be[2], ae[1]*be[1]+ae[3]*be[3],
            };
        }

        // transform vector
        template<typename vec_t>
        vec_t transform(const vec_t &v) const {
            return vec_t{
                v.x * e[0] + v.y * e[1],
                v.x * e[2] + v.y * e[3]
            };
        }
    };

    typedef mat2_t<float, sinf, cosf> mat2f_t;

} // namespace tengu
