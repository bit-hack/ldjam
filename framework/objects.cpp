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

object_ref_t object_factory_t::create(object_type_t type)
{
    auto itt = creator_.find(type);
    if (itt == creator_.end()) {
        return object_ref_t();
    }
    else {
        assert(itt->second);
        return object_ref_t(
                itt->second->create(type)
        );
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
