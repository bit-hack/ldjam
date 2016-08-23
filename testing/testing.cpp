#include "testing.h"

extern bool test_objects(testing_t & testing);

void register_tests(testing_t & testing) {
    test_objects(testing);
}

void testing_t::push_result(result_t res) {

}

void testing_t::add_test(const std::string & name, test_func_t test) {

    test_t test_obj {
        name,
        test
    };

    tests_.push_back(test_obj);
}

void testing_t::start() {
    for (const auto & test : tests_) {
        test.func_(*this);
    }
}

int main(const int argc, char *args[]) {

    testing_t testing;
    register_tests(testing);

    testing.start();

    return 0;
}
