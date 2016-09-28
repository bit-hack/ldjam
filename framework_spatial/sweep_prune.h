#pragma once

#include <cstdint>
#include <set>
#include <vector>

template <typename type_t>
struct node_t {
    enum { e_next, e_prev };
    enum { e_start, e_end };
    uint32_t type_;
    type_t & object_;
    const float & value_;
};

template <typename type_t>
struct sweep_prune_t {

    /* type_t contract:
     * - rectf_t & type_t::get_pos()
     */

    typedef std::set<type_pair_t> type_pair_set_t;
    std::pair<const type_t*, const type_t*> type_pair_t;

    void insert(type_t *object) {
    }

    void remove(type_t *object) {
    }

    void tick(type_t *object) {
    }

    void get_pairs(type_pair_set_t & pairs) {
    }

protected:
    std::vector<node_t> x_, y_;
    type_pair_set_t pairs_;
};