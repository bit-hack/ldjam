#include "../../framework_core/timer.h"

#define TEST_ASSERT(X) {if (X) { return false; }}

uint64_t thing_ = 0;

uint64_t get_time()
{
    return thing_;
}

bool test_timer_1()
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

bool test_timer_2()
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
