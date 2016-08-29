#pragma once

#include <vector>

template <typename type_t>
struct fsm_t {

    typedef void (type_t::*fsm_func_t)();

    fsm_t(type_t* self)
        : self_(self)
    {
    }

    struct fsm_state_t {

        fsm_state_t() = default;

        fsm_state_t(fsm_func_t tick,
            fsm_func_t enter = nullptr,
            fsm_func_t leave = nullptr)
            : on_enter_(enter)
            , on_tick_(tick)
            , on_leave_(leave)
        {
        }

        fsm_func_t on_enter_;
        fsm_func_t on_tick_;
        fsm_func_t on_leave_;
    };

    void tick()
    {
        if (stack_.size()) {
            const auto state = stack_.back();
            if (state.on_tick_) {
                (*self_.*(state.on_tick_))();
            }
        }
    }

    void state_push(const fsm_state_t& state)
    {
        stack_.push_back(state);
        if (state.on_enter_) {
            (*self_.*(state.on_enter_))();
        }
    }

    void state_pop()
    {
        if (stack_.size()) {
            const auto state = stack_.back();
            if (state.on_leave_) {
                (*self_.*(state.on_leave_))();
            }
            stack_.pop_back();
        }
    }

protected:
    std::vector<fsm_state_t> stack_;
    type_t* self_;
};
