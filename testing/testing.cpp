#include <string>
#include <map>

#include "testing.h"


extern void register_test_objects(testing_t & testing);
extern void register_test_timer(testing_t & testing);
extern int run_spatial_tests();

typedef std::map<const std::string, int (*)()> test_set_t;


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

int run_unit_tests()
{
    testing_t testing;
    register_test_objects(testing);
    register_test_timer(testing);
    testing.start();
}


namespace {
    int help(test_set_t tests) {
        for (const auto &test : tests) {
            printf(" - '%s'\n", test.first.c_str());
        }
        return 1;
    }
} // namespace {}

int main(const int argc, char *args[])
{
    test_set_t tests;
    tests["unit_tests"] = run_unit_tests;
    tests["spatial"] = run_spatial_tests;

    if (argc != 2) {
        printf("argument required\n");
        return help(tests);
    }

    const std::string arg = args[1];

    auto itt = tests.find(arg);
    if (itt == tests.end()) {
        printf("unknown testing type\n");
        return help(tests);
    }

    return (*itt).second();
}
