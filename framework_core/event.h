#pragma once

#include <cassert>
#include <cstdint>
#include <deque>
#include <set>
#include <vector>
#include <unordered_map>
#include <array>

namespace tengu {
struct event_t {

    const uint32_t type_;

    // <---- ---- ---- ---- ---- ---- ---- ---- ---- todo: add safe casts
};

struct event_listener_t {

    virtual void recieve_event(const event_t *) = 0;
};

struct event_stream_t {

    // add a filtered listener
    template <int SIZE>
    void add(event_listener_t * l, const uint32_t(&filter)[SIZE]) {
        for (int i = 0; i<SIZE; ++i) {
            const uint32_t type = filter[i];
            // push back this type
            local_[type].insert(l);
        }
    }

    // add a global listener
    void add(event_listener_t * l) {
        global_.insert(l);
    }

    // remove a filtered listener
    template <int SIZE>
    void remove(event_listener_t * l, const uint32_t(&filter)[SIZE]) {
        // for each type being filtered
        for (int i = 0; i<SIZE; ++i) {
            const uint32_t type = filter[i];
            // find it in the local map
            auto itt = local_.find(type);
            if (itt==local_.end()) {
                continue;
            }
            // get the set for this type
            event_set_t & vec = itt->second;
            if (vec.find(l)!=vec.end()) {
                // erase the entry
                vec.erase(l);
            }
        }
    }

    // remove a global listener
    void remove(event_listener_t * l) {
        if (global_.find(l)!=global_.end()) {
            global_.erase(l);
        }
    }

    // send a message to all eligible listeners
    void send(const event_t * event) {
        assert(event);
        // send to all global listeners first
        for (auto * l:global_) {
            l->recieve_event(event);
        }
        // send to any local listeners
        auto itt = local_.find(event->type_);
        if (itt!=local_.end()) {
            const auto & vec = itt->second;
            for (auto & l:vec) {
                l->recieve_event(event);
            }
        }
    }

protected:

    typedef std::set<event_listener_t*> event_set_t;

    std::unordered_map<uint32_t, event_set_t> local_;
    event_set_t global_;
};
} // namespace tengu
