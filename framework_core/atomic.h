#pragma once

#if defined (_MSC_VER)
#include <intrin.h>
#endif

namespace tengu {
struct atomic_t {

    atomic_t(long val)
        : v_(val)
    {
    }

    atomic_t()
        : v_(0)
    {
    }
    
#if defined(_MSC_VER)
    long inc() {
        return _InterlockedIncrement(&v_);
    }

    long dec() {
        return _InterlockedDecrement(&v_);
    }

    long xchg(const long x) {
        return _InterlockedExchange(&v_, x);
    }
#else
    long inc() {
        return __sync_fetch_and_add(&v_, 1);
    }

    long dec() {
        return __sync_fetch_and_sub(&v_, 1);
    }

    long xchg(const long x) {
        long o = __sync_lock_test_and_set(&v_, x);
        __sync_synchronize();
        return o;
    }
#endif

    long operator () () const {
        return v_;
    }

protected:
    volatile long v_;
};
} // namespace {}
