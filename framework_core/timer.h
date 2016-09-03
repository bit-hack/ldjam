#pragma once
#include <cassert>
#include <cstdint>

struct delta_time_t {
    typedef uint64_t (*millisec_t)();

    delta_time_t(millisec_t mills, uint64_t interval)
        : get_time_(mills)
        , interval_(interval)
    {
        reset();
    }

    float deltaf() const
    {
        assert(get_time_);
        uint64_t nval = get_time_();
        if (old_ > nval)
            return 0.f;
        const uint64_t diff = nval - old_;
        return float(diff) / float(interval_ ? interval_ : 1);
    }

    void reset()
    {
        assert(get_time_);
        old_ = get_time_();
    }

    void step()
    {
        old_ += interval_;
    }

    uint64_t deltai() const
    {
        assert(get_time_);
        uint64_t nval = get_time_();
        if (old_ > nval)
            return 0ull;
        const uint64_t diff = nval - old_;
        return interval_ ? (diff / interval_) : 0ull;
    }

    uint64_t interval_;

protected:
    uint64_t old_;
    uint64_t (*get_time_)();
};
