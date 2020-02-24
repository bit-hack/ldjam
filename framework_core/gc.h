#pragma once

#include <cassert>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace tengu {

struct gc_object_t {

  // static void gc_enum_t(const gc_object_t *obj, std::vector<const gc_object_t *> &live);

  virtual ~gc_object_t() {}
};


typedef void(*gc_enum_t)(
  const gc_object_t *,
  std::vector<const gc_object_t *> &);


struct gc_t {

  ~gc_t() {
    reset();
  }

  void reset() {
    for (auto *p : allocs_) {
      delete p;
    }
    allocs_.clear();
    pending_.clear();
    marked_.clear();
    enums_.clear();
  }

  template <class T, class... Types> T *alloc(Types &&... args) {
    // check allocation derives from gc_object_t
    static_assert(std::is_base_of<gc_object_t, T>::value,
                  "type must derive from gc_object_t");
    // perform the allocation and call constructor
    T *obj = new T(std::forward<Types>(args)...);
    // add the garbage collector enumerator
    enums_[obj] = T::gc_enum;
    // add to 'allocated' list
    allocs_.push_back(obj);
    return obj;
  }

  // perform a full sweep and collection
  void collect() {
    // visit everything
    while (!pending_.empty()) {
      // pop from the end of the pending list
      const gc_object_t *obj = pending_.back();
      pending_.pop_back();
      if (obj) {
        // if we have not visited this yet
        if (marked_.find(obj) == marked_.end()) {
          // mark it
          marked_.insert(obj);
          // enumerate its children
          auto itt = enums_.find(obj);
          assert(itt != enums_.end());
          if (itt != enums_.end()) {
            itt->second(obj, pending_);
          }
        }
      }
    }
    // delete redundant allocs
    for (auto itt = allocs_.begin(); itt != allocs_.end();) {
      if (marked_.find(*itt) == marked_.end()) {
        // erase the enumerator
        auto e = enums_.find(*itt);
        assert(e != enums_.end());
        enums_.erase(e);
        // delete the object
        assert(*itt);
        delete *itt;
        itt = allocs_.erase(itt);
      } else {
        ++itt;
      }
    }
    // clear ready for next time
    pending_.clear();
    marked_.clear();
  }

  // check in a known live object
  void check_in(const gc_object_t *obj) {
    pending_.push_back(obj);
  }

  // return the list of allocations
  const std::vector<gc_object_t *> &allocs() const {
    return allocs_;
  }

protected:
  // object enumeration functions
  std::unordered_map<const gc_object_t *, gc_enum_t> enums_;
  // all objects that have been allocated
  std::vector<gc_object_t *> allocs_;
  // objects we have visited
  std::unordered_set<const gc_object_t *> marked_;
  // objects we need to visit
  std::vector<const gc_object_t *> pending_;
};

} // namespace tengu
