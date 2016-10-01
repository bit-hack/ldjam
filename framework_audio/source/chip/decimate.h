/*
 * http://www.musicdsp.org/showArchiveComment.php?ArchiveID=231
 *
 * Posted by Paul Sernine
 * Refactored by Aidan Dodds (2016)
 *
 * These are /2 decimators, just instanciate one of them and use the Calc
 * method to obtain one sample while inputing two. There is 5,7 and 9 tap
 * versions.  They are extracted/adapted from a tutorial code by Thierry
 * Rochebois. The optimal coefficients are excerpts of Traitement numérique
 * du signal, 5eme edition, M Bellanger, Masson pp. 339-346.
 */
#pragma once
#include <array>

// 5 tap decimation
class decimate_5_t {
private:
    std::array<float, 5> R;
    const float h0, h1, h3, h5;

public:
    decimate_5_t()
        : h0( 346/692.0f)
        , h1( 208/692.0f)
        , h3(-44 /692.0f)
        , h5( 9  /692.0f) {
        R.fill(0.f);
    }

    float operator () (const float x0, const float x1) {
        const float h5x0 = h5 * x0;
        const float h3x0 = h3 * x0;
        const float h1x0 = h1 * x0;
        const float R6 = R[4] + h5x0;
        R[4] = R[3] + h3x0;
        R[3] = R[2] + h1x0;
        R[2] = R[1] + h1x0 + h0 * x1;
        R[1] = R[0] + h3x0;
        R[0] = h5x0;
        return R6;
    }
};

// 7 tap decimation
class decimate_7_t {
private:
    std::array<float, 7> R;
    const float h0, h1, h3, h5, h7;

public:
    decimate_7_t()
        : h0( 802/1604.0f)
        , h1( 490/1604.0f)
        , h3(-116/1604.0f)
        , h5( 33 /1604.0f)
        , h7(-6  /1604.0f){
        R.fill(0.f);
    }

    float operator () (const float x0, const float x1) {
        const float h7x0 = h7 * x0;
        const float h5x0 = h5 * x0;
        const float h3x0 = h3 * x0;
        const float h1x0 = h1 * x0;
        const float R8 = R[6] + h7x0;
        R[6] = R[5] + h5x0;
        R[5] = R[4] + h3x0;
        R[4] = R[3] + h1x0;
        R[3] = R[2] + h1x0 + h0 * x1;
        R[2] = R[1] + h3x0;
        R[1] = R[0] + h5x0;
        R[0] = h7x0;
        return R8;
    }
};

// 9 tap decimation
class decimate_9_t {
private:
    std::array<float, 9> R;
    const float h0, h1, h3, h5, h7, h9;

public:
    decimate_9_t()
        : h0( 8192/16384.0f)
        , h1( 5042/16384.0f)
        , h3(-1277/16384.0f)
        , h5( 429 /16384.0f)
        , h7(-116 /16384.0f)
        , h9( 18  /16384.0f) {
        R.fill(0.f);
    }

    float operator () (const float x0, const float x1) {
        const float h9x0 = h9 * x0;
        const float h7x0 = h7 * x0;
        const float h5x0 = h5 * x0;
        const float h3x0 = h3 * x0;
        const float h1x0 = h1 * x0;
        const float R10  = R[8] + h9x0;
        R[8] = R[7] + h7x0;
        R[7] = R[6] + h5x0;
        R[6] = R[5] + h3x0;
        R[5] = R[4] + h1x0;
        R[4] = R[3] + h1x0 + h0 * x1;
        R[3] = R[2] + h3x0;
        R[2] = R[1] + h5x0;
        R[1] = R[0] + h7x0;
        R[0] = h9x0;
        return R10;
    }
};
