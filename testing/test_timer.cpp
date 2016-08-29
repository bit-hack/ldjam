#include "testing.h"
#include "../framework/timer.h"

namespace {
uint64_t thing_ = 0;
uint64_t get_time()
{
    return thing_;
}
} // namespace {}

bool timer_test_1(testing_t& test)
{
    delta_time_t timer = delta_time_t(get_time, 4);
    thing_ = 0;
    TEST_ASSERT(timer.deltaf() == 0.00f);
    thing_ = 1;
    TEST_ASSERT(timer.deltaf() == 0.25f);
    thing_ = 2;
    TEST_ASSERT(timer.deltaf() == 0.50f);
    thing_ = 3;
    TEST_ASSERT(timer.deltaf() == 0.75f);
    thing_ = 4;
    TEST_ASSERT(timer.deltaf() == 1.00f);
    thing_ = 5;
    TEST_ASSERT(timer.deltaf() == 1.25f);
    timer.reset();
    TEST_ASSERT(timer.deltaf() == 0.00f);
    thing_ = 6;
    TEST_ASSERT(timer.deltaf() == 0.25f);
    thing_ = 9;
    TEST_ASSERT(timer.deltaf() == 1.00f);
    thing_ = 10;
    TEST_ASSERT(timer.deltaf() == 1.25f);
    timer.step();
    TEST_ASSERT(timer.deltaf() == 0.25f);
    return true;
}

bool timer_test_2(testing_t& test)
{
    {
        thing_ = 1ull;
        delta_time_t timer = delta_time_t(get_time, 0);
        timer.interval_ = 0;
        uint64_t di = timer.deltai();
        TEST_ASSERT(di == 0ull);
        float df = timer.deltaf();
        TEST_ASSERT(df == 0.f);
    }
    {
        thing_ = 0ull;
        delta_time_t timer = delta_time_t(get_time, 0);
        timer.interval_ = 1;
        uint64_t di = timer.deltai();
        TEST_ASSERT(di == 0ull);
        float df = timer.deltaf();
        TEST_ASSERT(df == 0.f);
    }
    {
        thing_ = 0ull;
        delta_time_t timer = delta_time_t(get_time, 0);
        timer.interval_ = 0ull;
        uint64_t di = timer.deltai();
        TEST_ASSERT(di == 0ull);
        float df = timer.deltaf();
        TEST_ASSERT(df == 0.f);
    }
    return true;
}

extern void register_test_timer(testing_t& testing)
{
    testing.add_test("timer test 1", timer_test_1);
    testing.add_test("timer test 2", timer_test_2);
}
