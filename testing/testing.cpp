#include "testing.h"

extern void register_test_objects(testing_t & testing);

void register_tests(testing_t & testing)
{
    register_test_objects(testing);
}

void testing_t::add_test(const std::string & name, test_func_t test)
{
    test_t test_obj {
        name,
        test
    };
    tests_.push_back(test_obj);
}

void testing_t::start()
{
    for (const auto & test : tests_) {
        bool res = test.func_(*this);
        printf("%s - %s\n",
               test.name_.c_str(),
               (res ? "pass" : "fail"));
    }
}

int main(const int argc, char *args[])
{
    testing_t testing;
    register_tests(testing);
    testing.start();
    return 0;
}
