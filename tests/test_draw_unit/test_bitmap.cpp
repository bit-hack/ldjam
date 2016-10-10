#include "../../framework_draw/bitmap.h"

using namespace tengu;

bool bitmap_test_1() {
    bitmap_t bmp;
    if (!bmp.load("d:/models/mini.bmp")) {
        return false;
    }
    return true;
}
