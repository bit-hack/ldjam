#pragma once
#include "atomic.h"
#include <assert.h>

namespace tengu {

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
