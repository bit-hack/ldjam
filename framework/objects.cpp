#include "objects.h"

object_ref_t object_t::get_ref()
{
    return object_ref_t(this);
}

void object_factory_t::add_creator(
    object_type_t type,
    object_factory_t::creator_t * creator)
{
    creator_[type] = std::unique_ptr<creator_t>(creator);
}

object_ref_t object_factory_t::create(object_type_t type,
                                      object_service_t service)
{
    auto itt = creator_.find(type);
    if (itt == creator_.end()) {
        return object_ref_t();
    }
    else {
        assert(itt->second);
        object_t * obj = itt->second->create(type, service);
        obj_.push_front(obj);
        return object_ref_t(obj);
    }
}

void object_factory_t::collect()
{
    for (auto itt = obj_.begin(); itt!=obj_.end();) {
        // deref to get our object
        object_t * obj = *itt;
        // check if this object is disposed
        if (obj->is_disposed()) {
            // find the creator for this object
            auto c_itt = creator_.find(obj->type_);
            assert(c_itt != creator_.end());
            // deref to get the creator object
            const up_object_creator_t & creator = c_itt->second;
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

void object_factory_t::tick()
{
    for (auto itt = obj_.begin(); itt!=obj_.end(); ++itt) {
        // deref to get our object
        object_t * obj = *itt;
        // check if this object is disposed
        if (!obj->is_disposed()) {
            obj->tick();
        }
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
