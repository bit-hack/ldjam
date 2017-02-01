#pragma once
#include <cassert>
#if defined(_MSC_VER)
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
    long inc()
    {
        return _InterlockedIncrement(&v_);
    }

    long dec()
    {
        return _InterlockedDecrement(&v_);
    }

    long xchg(const long x)
    {
        return _InterlockedExchange(&v_, x);
    }
#else
    long inc()
    {
        return __sync_fetch_and_add(&v_, 1);
    }

    long dec()
    {
        return __sync_fetch_and_sub(&v_, 1);
    }

    long xchg(const long x)
    {
        long o = __sync_lock_test_and_set(&v_, x);
        __sync_synchronize();
        return o;
    }
#endif

    long operator()() const
    {
        return v_;
    }

protected:
    volatile long v_;
};

extern void yield();

struct spinlock_t {

    spinlock_t()
        : atom_(c_unlocked)
    {
    }

    ~spinlock_t()
    {
        assert(atom_() == c_unlocked);
    }

    bool try_lock()
    {
        return atom_.xchg(c_locked) == c_unlocked;
    }

    void lock()
    {
        while (!try_lock()) {
            yield();
        }
        assert(atom_() == c_locked);
    }

    void unlock()
    {
        assert(atom_() == c_locked);
        atom_.xchg(c_unlocked);
    }

protected:
    atomic_t atom_;

    enum : long {
        c_unlocked = 0,
        c_locked
    };
};

template <typename lock_t>
struct scope_lock_t {

    scope_lock_t(lock_t& sl)
        : lock_(sl)
    {
        lock_.lock();
    }

    scope_lock_t(const scope_lock_t&) = delete;
    void operator=(const scope_lock_t&) = delete;

    ~scope_lock_t()
    {
        lock_.unlock();
    }

protected:
    lock_t& lock_;
};
} // namespace tengu
