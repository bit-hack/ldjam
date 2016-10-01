#pragma once

#include <cstdint>
#include <array>
#include <cmath>

template <typename type_t, 
          float (*sin_t)(float), 
          float (*cos_t)(float)>
struct mat2_t {

    type_t e[4];

    // return matrix inverse
    mat2_t invert() const {
        const float det = 1.f/(mat[0]*mat[3]-mat[1]*mat[2]);
        return mat2_t {
             det * mat[3], -det * mat[1],
            -det * mat[2],  det * mat[0]
        };
    }

    // return matrix transpose
    mat2_t transpose() const {
        return mat2_t{
            mat[0], mat[2]
            mat[1], mat[3]
        };
    }

    // index matrix array
    type_t & operator [] (size_t index) {
        assert(index<4);
        return e[index];
    }

    // index matrix array
    const type_t & operator [] (size_t index) const {
        assert(index<4);
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
        }
    }

    // transform vector
    template <typename vec_t>
    vec_t transform(const vec_t & v) const {
        return vec_t{
            v.x * e[0] + v.y * e[1],
            v.x * e[2] + v.y * e[3]
        };
    }
};

typedef mat2_t<float, sinf, cosf> mat2f_t;
