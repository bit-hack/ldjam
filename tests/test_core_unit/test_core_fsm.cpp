#include <array>
#include "../test_lib/test_lib.h"
#include "../../framework_core/fsm.h"

using namespace test_lib;
    
struct test_fsm_1_t: public test_t {

    test_fsm_1_t()
        : test_t("test_fsm_1_t")
    {
    }

    virtual bool run() override
    {
        using namespace tengu;

        struct test_t {
            fsm_t<test_t>::fsm_state_t state_a_;
            fsm_t<test_t>::fsm_state_t state_b_;

            uint32_t did_a_enter_;
            uint32_t did_a_tick_;
            uint32_t did_b_tick_;
            uint32_t did_b_leave_;

            test_t()
                : fsm_(this)
                , state_a_(&test_t::fsm_state_a_tick,
                           &test_t::fsm_state_a_enter,
                           nullptr)
                , state_b_(&test_t::fsm_state_b_tick,
                           nullptr,
                           &test_t::fsm_state_b_leave)
                , did_a_enter_(0)
                , did_a_tick_(0)
                , did_b_tick_(0)
                , did_b_leave_(0)
            {
                fsm_.state_push(state_a_);
            }

            void fsm_state_a_enter()
            {
                ++did_a_enter_;
            }

            void fsm_state_a_tick()
            {
                ++did_a_tick_;
                fsm_.state_push(state_b_);
            }

            void fsm_state_b_tick()
            {
                ++did_b_tick_;
                fsm_.state_pop();
            }

            void fsm_state_b_leave()
            {
                ++did_b_leave_;
            }

            void tick()
            {
                fsm_.tick();
            }

            fsm_t<test_t> fsm_;
        };

        test_t test_1;
        test_1.tick();
        test_1.tick();
        test_1.tick();

        TEST_ASSERT(test_1.did_a_enter_==1);
        TEST_ASSERT(test_1.did_a_tick_==2);
        TEST_ASSERT(test_1.did_b_tick_==1);
        TEST_ASSERT(test_1.did_b_leave_==1);

        return true;
    }
};

static std::array<test_lib::register_t*, 1> reg_test = {
    test_lib::register_t::test<test_fsm_1_t>()
};
