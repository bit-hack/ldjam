#include "../framework/fsm.h"

#include "testing.h"

namespace {
bool fsm_test_1(testing_t & test) {

    return false;
}
};

extern
void register_test_fsm(testing_t & testing)
{
    testing.add_test("fsm test 1", fsm_test_1);
}
