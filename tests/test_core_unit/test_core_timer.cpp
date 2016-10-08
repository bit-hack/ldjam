#include <array>
#include "../test_lib/test_lib.h"
#include "../../framework_core/timer.h"

using namespace test_lib;

namespace {
uint64_t thing_ = 0;
uint64_t get_time() {
    return thing_;
}
} // namespace {}

struct test_timer_1_t: public test_t {

    test_timer_1_t()
        : test_t("test_timer_1_t")
    {
    }

    virtual bool run() override {
        using namespace tengu;

        timing_t<uint64_t> timer = timing_t<uint64_t>(get_time, 4);
        thing_ = 0;
        TEST_ASSERT(timer.deltaf()==0.00f);
        thing_ = 1;
        TEST_ASSERT(timer.deltaf()==0.25f);
        thing_ = 2;
        TEST_ASSERT(timer.deltaf()==0.50f);
        thing_ = 3;
        TEST_ASSERT(timer.deltaf()==0.75f);
        thing_ = 4;
        TEST_ASSERT(timer.deltaf()==1.00f);
        thing_ = 5;
        TEST_ASSERT(timer.deltaf()==1.25f);
        timer.reset();
        TEST_ASSERT(timer.deltaf()==0.00f);
        thing_ = 6;
        TEST_ASSERT(timer.deltaf()==0.25f);
        thing_ = 9;
        TEST_ASSERT(timer.deltaf()==1.00f);
        thing_ = 10;
        TEST_ASSERT(timer.deltaf()==1.25f);
        timer.step();
        TEST_ASSERT(timer.deltaf()==0.25f);
        return true;
    }
};

struct test_timer_2_t: public test_t {

    test_timer_2_t()
        : test_t("test_timer_2_t")
    {
    }

    virtual bool run() override {
        using namespace tengu;
        {
            thing_ = 1ull;
            timing_t<uint64_t> timer = timing_t<uint64_t>(get_time, 0);
            timer.period_ = 0;
            uint64_t di = timer.deltai();
            TEST_ASSERT(di==0ull);
            float df = timer.deltaf();
            TEST_ASSERT(df==0.f);
        }
        {
            thing_ = 0ull;
            timing_t<uint64_t> timer = timing_t<uint64_t>(get_time, 0);
            timer.period_ = 1;
            uint64_t di = timer.deltai();
            TEST_ASSERT(di==0ull);
            float df = timer.deltaf();
            TEST_ASSERT(df==0.f);
        }
        {
            thing_ = 0ull;
            timing_t<uint64_t> timer = timing_t<uint64_t>(get_time, 0);
            timer.period_ = 0ull;
            uint64_t di = timer.deltai();
            TEST_ASSERT(di==0ull);
            float df = timer.deltaf();
            TEST_ASSERT(df==0.f);
        }
        return true;
    }
};

static std::array<register_t*, 2> reg_test = {
    register_t::test<test_timer_1_t>(),
    register_t::test<test_timer_2_t>(),
};
