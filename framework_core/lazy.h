#pragma once
#include <cassert>

namespace tengu {

template <typename type_t>
struct lazy_t {

    lazy_t()
        : valid_(false)
    {
    }

    lazy_t(const lazy_t& l)
        : valid_(l.valid_)
        , value_(l.value_)
    {
    }

    lazy_t(lazy_t&& rval)
        : valid_(rval.valid_)
        , value_(std::move(rval.value_))
    {
    }

    lazy_t(type_t& value)
        : valid_(true)
        , value_(value)
    {
    }

    lazy_t(const type_t& value)
        : valid_(true)
        , value_(value)
    {
    }

    void reset() { valid_ = false; }

    bool valid() const { return valid_; }

    type_t & get() { assert(valid_); return value_; }

    const type_t & get() const { assert(valid_); return value_; }

    lazy_t & operator=(const type_t& value)
    {
        valid_ = true;
        value_ = value;
        return *this;
    }

    lazy_t & operator=(const lazy_t& rhs)
    {
        if (valid_ = rhs.valid_) {
            value_ = rhs.value_;
        }
        return *this;
    }

    operator const type_t() const
    {
        assert(valid());
        return value_;
    }

    bool operator==(const lazy_t& rhs) const
    {
        return (valid_ && rhs.valid_) ? value_ == rhs.value_ : false;
    }

    bool operator!=(const lazy_t& rhs) const
    {
        return (valid_ && rhs.valid_) ? value_ != rhs.value_ : false;
    }

    operator bool() const {
        return valid_;
    }

#if 0
    lazy_t(type_t &&rval): valid_(true), value_(std::forward<type_t>(rval)) {}

    template <typename... Args>
    lazy_t(Args &&... Args) : valid_(true), value_(std::forward<Args>(args)...) {}
#endif

protected:
    bool valid_;
    type_t value_;
};

} // namespace tengu
