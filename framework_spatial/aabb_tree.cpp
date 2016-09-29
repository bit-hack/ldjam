#include "aabb_tree.h"

aabb_tree_t::aabb_tree_t()
    : rand_(0x12345) {
    clear();
}

void aabb_tree_t::clear() {
    map_.clear();
    parent_.fill(_NO_INDEX);
    child_a_.fill(_NO_INDEX);
    child_b_.fill(_NO_INDEX);
    type_.fill(nullptr);
    aabb_.fill(rectf_t{0, 0, 0, 0});
    free_.clear();
    free_.reserve(_SIZE);
    for (index_t i=0; i<_SIZE; ++i) {
        free_.push_back(i);
    }
}

void aabb_tree_t::remove(const aabb_proxy_t & proxy) {
    auto itt = map_.find(&proxy);
    if (itt == map_.end()) {
        return;
    }
    index_t ix = itt->second;
    if (ix == _NO_INDEX) {
        return;
    }
}

void aabb_tree_t::move(aabb_proxy_t & proxy, rectf_t & aabb) {
}

void aabb_tree_t::rebalance(index_t ix) {
}

void aabb_tree_t::compute_aabb(index_t ix, rectf_t & out) {
    assert(ix != _NO_INDEX);
    // if this is a leaf node
    if (aabb_proxy_t * proxy = type_[ix]) {
        // todo: fatten this aabb
        out = proxy->aabb_;
    }
    else {
        // pull out both children
        const index_t c1 = child_a_[ix];
        const index_t c2 = child_b_[ix];
        // non leaf must have two children
        assert (c1 != _NO_INDEX && c2 != _NO_INDEX);
        // take product of both children
        out = rectf_t::combine(aabb_[c1], aabb_[c2]);
    }
}

void aabb_tree_t::insert(index_t new_node, index_t ix) {
    if (is_leaf(ix)) {
        // todo
        // new_node is the thing being inserted
        // ix turns into the super node
        // we make a new node to become the old_ix
    }
    else {
        // pull out both children
        const index_t c1 = child_a_[ix];
        const index_t c2 = child_b_[ix];
        // non leaf must have two children
        assert (c1 != _NO_INDEX && c2 != _NO_INDEX);
        // find aabb expansion after possible insertion
        const rectf_t r1 = rectf_t::combine(aabb_[c1], aabb_[new_node]);
        const rectf_t r2 = rectf_t::combine(aabb_[c2], aabb_[new_node]);
        // find increase in area
        const float dr1 = r1.area() - aabb_[c1].area();
        const float dr2 = r2.area() - aabb_[c2].area();
        // find node to insert into
        bool insert_c1 = false;
        if (dr1 == dr2) {
            if (rand_.rand() & 1) {
                insert_c1 = true;
            }
        } else if (dr1 < dr2) {
            insert_c1 = true;
        }
        // perform insertion and adjustment
        if (insert_c1) {
            aabb_[ix] = r1;
            insert(new_node, c1);
        }
        else {
            aabb_[ix] = r2;
            insert(new_node, c2);
        }
    }
}
