#include "../framework/bitmap.h"

#include "testing.h"

namespace {
bool bitmap_test_1(testing_t& test) {

    bitmap_t bmp;
    if (!bitmap_t::load("d:/models/mini.bmp", bmp)) {
        return false;
    }

    return true;
}
};

extern void register_test_bitmap(testing_t& testing) {
    testing.add_test("bitmap test 1", bitmap_test_1);
}
