#pragma once

#include <queue>
#include <string>

#define TEST_ASSERT(X) { if (!(X)) { return false; } }


struct testing_t
{
    typedef bool (*test_func_t)(testing_t & testing);

    struct test_t {
        std::string name_;
        test_func_t func_;
    };

    void add_test(const std::string & name,
                  test_func_t test);

    void start();

protected:
    std::vector<test_t> tests_;
};
