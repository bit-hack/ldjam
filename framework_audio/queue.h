#pragma once

#include <mutex>
#include <queue>
#include "../framework_core/thread.h"

namespace tengu {
template <typename type_t>
struct queue_t {

    void push(const type_t & in) {
        tengu::scope_lock_t<tengu::spinlock_t> guard(lock_);
        queue_.push(in);
    }

    bool pop(type_t & out) {
        tengu::scope_lock_t<tengu::spinlock_t> guard(lock_);
        if (queue_.empty()) {
            return false;
        }
        else {
            out = queue_.front();
            queue_.pop();
            return true;
        }
    }

protected:
    tengu::spinlock_t lock_;
    std::queue<type_t> queue_;
    std::mutex mutex_;
};
} // namespace tengu
