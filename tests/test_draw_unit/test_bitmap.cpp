#include "../../framework_draw/bitmap.h"

bool bitmap_test_1()
{
    bitmap_t bmp;
    if (!bitmap_t::load("d:/models/mini.bmp", bmp)) {
        return false;
    }
    return true;
}
