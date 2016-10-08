#include <array>
#include "../test_lib/test_lib.h"

#include "../../framework_core/anim.h"

using namespace test_lib;

struct test_anim_t: public test_t {

    test_anim_t()
        : test_t("test_anim_t")
    {
    }

    enum {
        e_foot_fall
    };

    virtual bool run() override {
        using namespace tengu;

        anim::sheet_t sheet(128, 128);
        sheet.add_grid(32, 32);

        anim::controller_t control;
        control.set_sheet(&sheet);

        anim::sequence_t seq1("walk",
                              anim::sequence_t::e_end_loop);
        seq1.op_interval(1).
            op_event(e_foot_fall).
            op_frame(0).
            op_delay(2).
            op_frame(1).
            op_event(e_foot_fall).
            op_frame(2).
            op_delay(2).
            op_frame(3);

        anim::sequence_t seq2("impact",
                              anim::sequence_t::e_end_pop);
        seq2.op_interval(1).
            op_event(e_foot_fall).
            op_frame(0).
            op_frame(1).
            op_frame(2);

        control.tick(1);

        control.push_sequence(&seq1);
        control.push_sequence(&seq2);

        for (int i = 0; i<10; ++i) {
            control.tick(1);
            recti_t frame;
            control.get_frame(frame);
        }

        return true;
    }
};

static std::array<register_t*, 1> reg_test = {
    register_t::test<test_anim_t>()
};
