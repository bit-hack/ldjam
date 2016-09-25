#include <algorithm>
#include "objects.h"

object_ref_t object_t::get_ref() {
    return object_ref_t(this);
}

void object_factory_t::add_creator(
    object_type_t type,
    object_factory_t::creator_t* creator)
{
    creator_[type] = std::unique_ptr<creator_t>(creator);
}

object_ref_t object_factory_t::create(object_type_t type) {
    auto itt = creator_.find(type);
    if (itt == creator_.end()) {
        return object_ref_t();
    }
    else {
        assert(itt->second);
        object_t* obj = itt->second->create(type, service_);
        stage_.push_back(obj);
        return object_ref_t(obj);
    }
}

void object_factory_t::sort() {
    const size_t count = obj_.size();
    // shuffle into the correct position.  while not a great sort
    // its reasonable for the general case where the list is mostly
    // ordered.
    for (size_t i = 1; i<count; ++i) {
        size_t j = i;
        while (j>=1 && object_t::compare(obj_[j], obj_[j-1])) {
            std::swap(obj_[j-1], obj_[j]);
            --j;
        }
    }
#if 1
    // assert that orders are upheld
    for (size_t i = 1; i<count; ++i) {
        assert(!object_t::compare(obj_[i], obj_[i-1]));
    }
#endif
}

void object_factory_t::collect() {
    for (auto itt = obj_.begin(); itt != obj_.end();) {
        // deref to get our object
        object_t* obj = *itt;
        // check if this object is disposed
        if (obj->is_disposed()) {
            // find the creator for this object
            auto c_itt = creator_.find(obj->type_);
            assert(c_itt != creator_.end());
            // deref to get the creator object
            const up_object_creator_t& creator = c_itt->second;
            // use the creator to destroy this object
            creator->destroy(obj);
            // remove this object from the list
            itt = obj_.erase(itt);
        }
        // object has referenced then it is alive
        else {
            ++itt;
        }
    }
}

void object_factory_t::tick() {
    // itterate over all active objects
    for (auto itt = obj_.begin(); itt != obj_.end(); ++itt) {
        // deref to get our object
        object_t* obj = *itt;
        assert(obj);
        // check if this object is disposed
        if (!obj->is_disposed() && obj->is_alive()) {
            obj->tick();
        }
    }
    // merge stage_ into obj_
    if (!stage_.empty()) {
        obj_.reserve(obj_.size() + stage_.size());
        obj_.insert(obj_.end(), stage_.begin(), stage_.end());
        stage_.clear();
    }
}

void object_ref_t::dec() {
    if (obj_) {
        obj_->ref_.dec();
    }
}

void object_ref_t::inc() {
    if (obj_) {
        obj_->ref_.inc();
    }
}
