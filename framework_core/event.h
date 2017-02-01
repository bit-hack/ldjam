#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <set>
#include <unordered_map>
#include <vector>

namespace tengu {

typedef uint32_t event_type_t;

struct event_t {

    event_t(event_type_t type)
        : type_(type)
    {
    }

    template <typename type_t>
    type_t& cast()
    {
        assert(type_t::type() == type_);
        return *static_cast<type_t*>(this);
    }

    template <typename type_t>
    type_t& cast() const
    {
        assert(type_t::type() == type_);
        return *static_cast<const type_t*>(this);
    }

    template <typename type_t>
    bool is_a() const
    {
        return type_t::type() == type_;
    }

    event_type_t type() const
    {
        return type_;
    }

protected:
    const event_type_t type_;
};

struct event_listener_t {

    virtual void recieve_event(const event_t*) = 0;
};

struct event_stream_t {

    // add a filtered listener
    template <int SIZE>
    void add(event_listener_t* l, const std::array<event_type_t, SIZE>& filter)
    {
        for (int i = 0; i < SIZE; ++i) {
            const event_type_t type = filter[i];
            // push back this type
            local_[type].insert(l);
        }
    }

    // add a global listener
    void add(event_listener_t* l)
    {
        global_.insert(l);
    }

    // remove a filtered listener
    template <int SIZE>
    void remove(event_listener_t* l, const std::array<event_type_t, SIZE>& filter)
    {
        // for each type being filtered
        for (int i = 0; i < SIZE; ++i) {
            const event_type_t type = filter[i];
            // find it in the local map
            auto itt = local_.find(type);
            if (itt == local_.end()) {
                continue;
            }
            // get the set for this type
            event_set_t& vec = itt->second;
            if (vec.find(l) != vec.end()) {
                // erase the entry
                vec.erase(l);
            }
        }
    }

    // remove a global listener
    void remove(event_listener_t* l)
    {
        if (global_.find(l) != global_.end()) {
            global_.erase(l);
        }

        //todo: remove from all locals?
    }

    // send a message to all eligible listeners
    void send(const event_t* event)
    {
        assert(event);
        // send to all global listeners first
        for (auto* l : global_) {
            l->recieve_event(event);
        }
        // send to any local listeners
        auto itt = local_.find(event->type());
        if (itt != local_.end()) {
            const auto& vec = itt->second;
            for (auto& l : vec) {
                l->recieve_event(event);
            }
        }
    }

protected:
    typedef std::set<event_listener_t*> event_set_t;

    std::unordered_map<event_type_t, event_set_t> local_;
    event_set_t global_;
};
} // namespace tengu
