#pragma once

#include <cstdint>
#include <set>
#include <vector>
#include <array>

#include "../framework_core/objects.h"
#include "../framework_core/rect.h"

namespace tengu {
struct sweep_prune_proxy_t {

    sweep_prune_proxy_t(object_t * obj, const rectf_t & aabb)
        : rect_(aabb)
        , object_(obj)
    {
    }

    const rectf_t & get_rect() const {
        return rect_;
    }

    const object_t * get_object() const {
        return object_;
    }

protected:
    friend struct sweep_prune_t;
    rectf_t rect_;
    object_t * const object_;

    struct axis_t {
        std::array<size_t, 2> index_;
    };

    std::array<axis_t, 2> axis_;
};

struct sweep_prune_t {

    typedef std::pair<const sweep_prune_proxy_t*,
        const sweep_prune_proxy_t*> proxy_pair_t;
    typedef std::set<proxy_pair_t> type_pair_set_t;

    void insert(sweep_prune_proxy_t & proxy) {
    }

    void remove(sweep_prune_proxy_t & proxy) {
    }

    void move(sweep_prune_proxy_t & proxy, const rectf_t & dest) {
    }

protected:

    enum marker_t {
        e_mark_start,
        e_mark_end
    };

    struct entry_t {
        float value_;
        sweep_prune_proxy_t * proxy_;
        marker_t marker_;
    };

    struct axis_t {

        // insert a fresh node into this axis
        void insert(sweep_prune_proxy_t*, size_t axis);

        // sort a specific node in this axis
        void sort(size_t index);

        // structure of arrays entry_t
        std::vector<float> value_;
        std::vector<sweep_prune_proxy_t*> Proxy_;
        std::vector<marker_t> marker_;
    };

    // the x and y axis
    std::array<axis_t, 2> axis_;

    // persistant pair object for better cache and alloc perf.
    proxy_pair_t pairs_;
};
} // namespace tengu
