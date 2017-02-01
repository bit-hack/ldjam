#pragma once

#include <cassert>
#include <stdint.h>
#include <vector>

namespace tengu {
template <typename type_t>
struct fsm_t {

    typedef void (type_t::*fsm_func_t)();

    fsm_t(type_t* self)
        : self_(self)
        , visible_state_(0)
    {
        assert(self);
    }

    struct fsm_state_t {

        fsm_state_t()
            : on_enter_(nullptr)
            , on_tick_(nullptr)
            , on_leave_(nullptr)
        {
        }

        fsm_state_t(
            fsm_func_t tick,
            fsm_func_t enter = nullptr,
            fsm_func_t leave = nullptr)
            : on_enter_(enter)
            , on_tick_(tick)
            , on_leave_(leave)
        {
        }

        bool operator==(const fsm_state_t& other) const
        {
            return 
                on_enter_ == other.on_enter_ &&
                on_tick_ == other.on_tick_ &&
                on_leave_ == other.on_leave_;
        }

        fsm_func_t on_enter_;
        fsm_func_t on_tick_;
        fsm_func_t on_leave_;
    };

    void tick()
    {
        if (!stack_.empty()) {
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
        if (!stack_.empty()) {
            const auto state = stack_.back();
            if (state.on_leave_) {
                (*self_.*(state.on_leave_))();
            }
            stack_.pop_back();
        }
    }

    void state_change(const fsm_state_t& state)
    {
        state_pop();
        state_push(state);
    }

    bool empty() const
    {
        return stack_.empty();
    }

    uint32_t visible_state() const
    {
        return visible_state_;
    }

    const fsm_state_t& state() const
    {
        assert(!stack_.empty());
        return stack_.back();
    }

protected:
    uint32_t visible_state_;
    std::vector<fsm_state_t> stack_;
    type_t* const self_;
};
} // namespace tengu
