#pragma once
#include <cstdint>
#include <array>
#include <cassert>
#include <deque>
#include <bitset>

namespace pathfind {

template <typename type_t, size_t c_size>
struct frame_alloc_t {

    type_t * alloc() {
#if 0
        assert(index_<c_size);
#else
        if (index_>=c_size)
            return nullptr;
#endif
        return &list_[index_++];
    }

    void clear() {
        index_ = 0;
    }

protected:
    size_t index_;
    std::array<type_t, c_size> list_;
};

// Fixed size binary heap implementation
template <typename type_t,
          size_t c_size,
          bool compare(const type_t &a, const type_t &b)>
struct bin_heap_t {

    // index of the root node
    static const size_t c_root = 1;
    static const size_t capacity = c_size;

    // Note:
    //  To simplify the implementation, heap_ is base 1 (index 0 unused).

    bin_heap_t(): index_(c_root) {}

    // pop the current best node in the heap (root node)
    type_t pop() {
        assert(!empty());
        // save top most value for final return
        type_t out = heap_[c_root];
        // swap last and first and shrink by one
        index_ -= 1;
        std::swap(heap_[c_root], heap_[index_]);
        // push down to correct position
        bubble_down(c_root);
        return out;
    }

    // push a new node into the heap and make rebalanced tree
    void push(type_t node) {
        assert(!full());
        // insert node into end of list
        size_t i = index_++;
        heap_[i] = node;
        bubble_up(i);
    }

    // return true if the heap is empty
    bool empty() const {
        return index_<=1;
    }

    // return true if the heap is full
    bool full() const {
        return index_ > c_size;
    }

    // number of nodes currently in the heap
    size_t size() const {
        return index_-1;
    }

    // wipe the heap back to its empty state
    void clear() {
        index_ = c_root;
    }

    // test that we have a valid binary heap
    void validate() const {
        for (size_t i = 2; i<index_; ++i)
            assert(compare(heap_[parent(i)], heap_[i]));
    }

    // discard a number of items from the bottom of the heap
    void discard(size_t num) {
        assert(index_-num >= c_root);
        index_ -= num;
    }

protected:
    std::array<type_t, c_size+1> heap_;
    size_t index_;

    // check an index points to a valid node
    bool valid(size_t i) const {
        return i<index_;
    }

    // bubble an item down to its correct place in the tree
    inline void bubble_down(size_t i) {
        // while we are not at a leaf
        while (valid(child(i, 0))) {
            // get both children
            const size_t x = child(i, 0);
            const size_t y = child(i, 1);
            // select best child
            const bool select = valid(y)&&compare(heap_[y], heap_[x]);
            type_t & best = select ? heap_[y] : heap_[x];
            // quit if children are not better
            if (!compare(best, heap_[i]))
                break;
            // swap current and child
            std::swap(best, heap_[i]);
            // repeat from child node
            i = select ? y : x;
        }
    }

    // bubble and item up to its correct place in the tree
    inline void bubble_up(size_t i) {
        // while not at the root node
        while (i>c_root) {
            const size_t j = parent(i);
            // if node is better then parent
            if (!compare(heap_[i], heap_[j]))
                break;
            std::swap(heap_[i], heap_[j]);
            i = j;
        }
    }

    // given an index, return the parent index
    static inline size_t parent(size_t index) {
        return index/2;
    }

    // given an index, return one of the two child nodes (branch 0/1)
    static inline size_t child(size_t index, uint32_t branch) {
        assert(!(branch&~1u));
        return index*2+branch;
    }
};

template<size_t c_size>
struct bit_mask_t {

    void set(const size_t index) {
        set_[index] = true;
    }

    bool is_set(const size_t index) const {
        return set_[index];
    }

    void clear() {
        set_.reset();
    }

protected:
    std::bitset<c_size> set_;
};

template<typename type_t>
struct stack_t {

    void push(type_t in) {
        list_.push_back(in);
    }

    type_t pop() {
        assert(!list_.empty());
        type_t out = list_.back();
        list_.pop_back();
        return out;
    }

    bool empty() const {
        return list_.empty();
    }

    void clear() {
        list_.clear();
    }

    const type_t &top() const {
        return list_.back();
    }

protected:
    std::deque<type_t> list_;
};

template<typename waypoint_t,
         size_t c_frame_size,
         size_t c_heap_size,
         size_t c_mask_size,
         typename derived_t>
struct astar_t {

    // search node
    struct node_t {
        // parent node
        const node_t *parent_;
        // node location
        waypoint_t waypoint_;
        // path length + heuristic
        uint32_t cost_;
        // length of path so far
        uint32_t length_;
    };

    typedef node_t as_node_t;

    // node compare function for binary heap
    static constexpr bool compare(
            const as_node_t *const &a,
            const as_node_t *const &b) {
        return a->cost_ < b->cost_;
    }

    typedef frame_alloc_t<as_node_t, c_frame_size> as_alloc_t;
    typedef bin_heap_t<const as_node_t *, c_heap_size, compare> as_heap_t;
    typedef bit_mask_t<c_mask_size> as_mask_t;

    // ctor
    astar_t() : self_(*static_cast<derived_t *>(this)) {}

    // perform an A* search
    bool search(const waypoint_t &start,
                const waypoint_t &goal,
                stack_t<waypoint_t> &path) {
        // clear traces of previous search
        frame_.clear();
        heap_.clear();
        mask_.clear();
        path.clear();
        error_ = false;
        // add the start node to the heap
        {
            as_node_t *temp = node_alloc();
            temp->parent_ = nullptr;
            temp->waypoint_ = start;
            temp->cost_ = self_.node_cost(temp->waypoint_, goal);
            temp->length_ = 0;
            node_push(temp);
        }
        // our itteration node pointer
        const as_node_t *node = nullptr;
        // while we have nodes in the heap
        while (!heap_.empty()) {
            // pop best node off heap
            node = heap_.pop();
            // path found when cost is zero
            if (node->waypoint_ == goal) {
                break;
            }
            // collect all surrounding nodes
            if (!self_.node_expand(*node, goal) || error_) {
                return false;
            }
            // clear node for next pass
            node = nullptr;
        }
        // trace path from end to start
        while (node && !error_) {
            path.push(node->waypoint_);
            node = node->parent_;
        }
        // success if path is not empty
        return !path.empty();
    }

    // push a search node into the open list
    void node_push(as_node_t *n) {
        // if the heap is too full discard worst nodes.
        // this can lead to suboptimal paths or even failed pathfind results.
        // it is however one way to deal with a fixed open set size.
        if (heap_.full()) {
            heap_.discard(heap_.capacity / 4);
        }
        if (heap_.full()) {
            error_ = true;
            return;
        } else {
            // mark node as explored
            mask_.set(self_.linear_id(n->waypoint_));
            // push into open set
            heap_.push(n);
        }
    }

    // allocate a new search node
    // this function will always return a valid pointer
    as_node_t *node_alloc() {
        as_node_t *n = frame_.alloc();
        if (n == nullptr) {
            error_ = true;
            n = &dummy_;
        }
        return n;
    }

    // expand a node and add adjacent nodes to open set
    virtual bool node_expand(const as_node_t &in,
                             const waypoint_t &goal) = 0;

    // node heuristic function
    virtual uint32_t node_cost(const waypoint_t &dest,
                               const waypoint_t &pos) const = 0;

    // convert the waypoint into a unique linear address
    virtual size_t linear_id(const waypoint_t &a) const = 0;

    const as_mask_t & get_mask() const {
        return mask_;
    }

    bool error() const {
        return error_;
    }

protected:
    as_alloc_t frame_;
    as_mask_t mask_;
    as_heap_t heap_;
    derived_t & self_;
    bool error_;
    as_node_t dummy_;
};
} // pathfind
