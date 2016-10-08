#pragma once
#include <cassert>
#include <vector>
#include <memory>
#include <cstdio>
#include <string>

#define TEST_ASSERT(X) {if (!(X)) { return false; }}

namespace test_lib {
struct test_t {

    test_t(const std::string & name)
        : name_(name)
        , pass_(true)
    {
    }

    virtual ~test_t() {
    }

    virtual bool pre() {
        return true;
    }

    virtual bool run() = 0;

    virtual bool post() {
        return true;
    }

    const std::string & name() const {
        return name_;
    }

    bool pass() const {
        return pass_;
    }

    const std::string & message() const {
        return msg_;
    }

protected:
    bool pass_;
    const std::string name_;
    std::string msg_;
};

struct executor_t {

    template <typename type_t>
    void add() {
        std::unique_ptr<test_t> temp(new type_t);
        tests_.push_back(std::move(temp));
    }

    int32_t run() {
        int32_t fails = 0;
        for (up_test_t & test:tests_) {
            assert(test.get());
            bool pass = true;
            pass &= test->pre();
            pass &= test->run();
            pass &= test->post();
            pass &= test->pass();
            const std::string & name = test->name();
            const std::string & msg = test->message();
            assert(!name.empty());
            printf("%s - %s\n", name.c_str(), pass ? "pass": "fail");
            if (!pass) {
                if (!msg.empty()) {
                    printf("  - \"%s\"\n", msg.c_str());
                }
            }
            fails += pass ? 0 : 1;
        }
        return fails;
    }

    static executor_t & inst() {
        static executor_t * inst_;
        if (!inst_) {
            inst_ = new executor_t;
        }
        return *inst_;
    }

protected:

    executor_t() {}

    typedef std::unique_ptr<test_t> up_test_t;
    std::vector<up_test_t> tests_;
};

struct register_t {

    typedef volatile void(*ftype_t)();

    template <typename type_t>
    static register_t * test() {
        executor_t::inst().add<type_t>();
        return new register_t(ftype_t(dummy));
    }

protected:
    register_t(ftype_t f): func_(f) {
        func_();
    }

    // dummy to avoid DCE
    static void dummy() {}
    ftype_t func_;
};
} // namespace test_lib
