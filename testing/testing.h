#pragma once

#include <queue>
#include <string>

#define TEST_ASSERT(X) { if (!(X)) {} }


struct testing_t {

    typedef void (*test_func_t)(testing_t & testing);

    enum state_t {
        e_pass,
        e_fail
    };

    struct test_t {
        std::string name_;
        test_func_t func_;
    };

    struct result_t {
        state_t result_;
        std::string test_name_;
        std::string msg_;
    };

    void add_test(const std::string & name,
                  test_func_t test);

    void push_result(result_t res);

    void start();

protected:
    std::queue<result_t> results_;
    std::vector<test_t> tests_;
};
