#pragma once

#include <vector>

template <typename type_t>
struct fsm_t {

    typedef void(type_t::*fsm_func_t)();

    struct fsm_state_t {
        fsm_func_t on_enter_;
        fsm_func_t on_tick_;
        fsm_func_t on_leave_;
    };

    void tick() {
    }

    void state_push(const fsm_state_t & state) {
    }

    void state_pop() {
    }

protected:
    std::vector<fsm_state_t> states_;
};
