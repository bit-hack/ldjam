#pragma once

#include <array>
#include <cstdint>

#include "../framework_core/rect.h"
#include "../framework_core/objects.h"
#include "../framework_core/vec2.h"
#include "../framework_core/random.h"

/* axis aligned bounding box proxy object
 */
struct aabb_proxy_t {

    aabb_proxy_t(object_t * obj,
                 const rectf_t & aabb)
        : obj_(obj)
        , aabb_(aabb)
    {}

    /* get the axis aligned bounding box of this proxy
     */
    const rectf_t & get_aabb() const {
        return aabb_;
    }

    /* get the object associated with this proxy
     */
    object_t * get_object() const {
        return obj_;
    }

protected:

    friend struct aabb_tree_t;
    object_t * const obj_;
    rectf_t aabb_;
};

/* axis aligned bounding box tree object
 */
struct aabb_tree_t {

    aabb_tree_t();

    /* clear the aabb tree entirely
     */
    void clear();

    /* insert a proxy object into the tree
     */
    void insert(const aabb_proxy_t & proxy) {
        index_t ix = new_node();
        map_[&proxy] = ix;
        aabb_[ix] = proxy.aabb_;
        fatten(aabb_[ix]);
        insert(ix, 0);
    }

    /* remove a proxy object from the tree
     */
    void remove(const aabb_proxy_t & proxy);

    /* move a proxy object within the tree
     */
    void move(aabb_proxy_t & proxy, rectf_t & aabb);

    // todo: add raycast and spatial queries

protected:
    typedef int16_t index_t;

    /*
     */
    void fatten(rectf_t & rect) {
        // todo
    }

    /* re-balance a specific nodes immediate children
     */
    void rebalance(index_t);

    /* compute the aabb for a given node
     */
    void compute_aabb(index_t, rectf_t & out);

    /* insert one node into a given node
     */
    void insert(index_t new_node, index_t index);

    /* check if a node is a leaf
     */
    bool is_leaf(index_t index) const {
        return type_[index] != nullptr;
    }

    index_t new_node() {
        assert(!free_.empty());
        index_t out = free_.back();
        free_.pop_back();
        return out;
    }

protected:
    // todo: _SIZE can be template param along with object_t + proxy
    static const size_t _SIZE = 1024;
    static const index_t _NO_INDEX = -1;

    template <typename type_t>
    using array_t = std::array<type_t, _SIZE>;

    //
    random_t rand_;

    // index -> (fat) axis aligned bounding box
    array_t<rectf_t> aabb_;
    // index -> proxy object
    array_t<aabb_proxy_t*> type_;
    // index -> node edges
    array_t<index_t> parent_;
    array_t<index_t> child_a_;
    array_t<index_t> child_b_;
    // proxy -> index
    std::map<const aabb_proxy_t*, index_t> map_;
    // list of free nodes
    std::vector<index_t> free_;
};
