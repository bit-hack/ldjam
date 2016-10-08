#include <array>
#include "../test_lib/test_lib.h"
#include "../../framework_core/buffer.h"

using namespace test_lib;

#define TEST_ASSERT(X) {if (!(X)) { return false; }}

struct test_buffer_t: public test_t {

    test_buffer_t()
        : test_t("test_buffer_t")
    {
    }

    bool run() {
        using namespace tengu;

        buffer_t b1(32);
        TEST_ASSERT(b1.get());

        b1.fill(0);

        for (int i = 0; i<10; ++i) {
            *b1.get(i) = i;
        }

        for (int i = 0; i<10; ++i) {
            TEST_ASSERT(b1.get()[i]==i);
        }

        b1.resize(64);

        for (int i = 0; i<10; ++i) {
            *b1.get<uint16_t>(i*sizeof(uint16_t)) = i;
        }

        for (int i = 0; i<10; ++i) {
            TEST_ASSERT(b1.get<uint16_t>()[i]==i);
        }

        {
            buffer_t b2 = b1;
            for (int i = 0; i<10; ++i) {
                TEST_ASSERT(b2.get<uint16_t>()[i]==i);
            }
        }

        return true;
    }
};

static std::array<test_lib::register_t*, 1> reg_test = {
    test_lib::register_t::test<test_buffer_t>()
};
