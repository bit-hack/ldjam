#pragma once

#include <mutex>
#include <queue>

template <typename type_t>
struct queue_t {

    void push(const type_t & in) {
        std::lock_guard<std::mutex> guard(mutex_);
        queue_.push(in);
    }

    bool pop(type_t & out) {
        std::lock_guard<std::mutex> guard(mutex_);
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
    std::queue<type_t> queue_;
    std::mutex mutex_;
};
