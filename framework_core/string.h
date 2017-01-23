#pragma once
#include "random.h"
#include <cstdint>
#include <cstring>

namespace tengu {
struct string_t {

    string_t(const char* str)
        : str_(str)
        , hash_(tengu::hash_t::string_1(str))
    {
    }

    string_t(string_t&& temp)
        : str_(std::move(temp.str_))
        , hash_(temp.hash_)
    {
    }

    size_t size() const
    {
        return str_.size();
    }

    bool operator==(const string_t& rhs) const
    {
        if (hash_ == rhs.hash_) {
            return str_ == rhs.str_;
        }
        return false;
    }

    struct hash_t {
        size_t operator()(const string_t& in) const
        {
            return in.hash_;
        }
    };

    struct compare_t {
        size_t operator()(const string_t& a, const string_t& b) const
        {
            return a.hash_ < b.hash_;
        }
    };

protected:
    friend struct hash_t;
    friend struct compare_t;
    uint32_t hash_;
    const std::string str_;
};

struct const_str_t {

    const_str_t(const char* str)
        : str_(str)
        , hash_(tengu::hash_t::string_1(str))
    {
    }

    bool operator==(const const_str_t& rhs) const
    {
        if (hash_ == rhs.hash_) {
            return strcmp(str_, rhs.str_) == 0;
        }
        return false;
    }

    struct hash_t {
        size_t operator()(const const_str_t& in) const
        {
            return in.hash_;
        }
    };

protected:
    friend struct hash_t;
    const uint32_t hash_;
    const char* str_;
};
} // namespace tengu