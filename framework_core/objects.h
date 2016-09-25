#pragma once

#include <memory>
#include <cassert>
#include <cstdint>
#include <map>
#include <list>

struct ref_t;
struct object_t;
struct object_ref_t;

typedef uint32_t object_type_t;

typedef void* object_service_t;

struct ref_t {
    ref_t()
        : value_(0)
    {
    }

    uint32_t inc()
    {
        return ++value_;
    }

    uint32_t dec()
    {
        return --value_;
    }

    bool disposed() const
    {
        return value_ <= 0;
    }

    int32_t count() const
    {
        return value_;
    }

protected:
    int32_t value_;
};

struct object_ref_t {
    object_ref_t()
        : obj_(nullptr)
    {
    }

    object_ref_t(object_t* obj)
        : obj_(obj)
    {
        inc();
    }

    object_ref_t(const object_ref_t& copy)
        : obj_(copy.obj_)
    {
        inc();
    }

    object_ref_t(object_ref_t&& copy)
        : obj_(copy.obj_)
    {
        copy.obj_ = nullptr;
    }

    void operator=(const object_ref_t& rhs)
    {
        dec();
        obj_ = rhs.obj_;
        inc();
    }

    ~object_ref_t()
    {
        dec();
    }

    bool valid() const
    {
        return obj_ != nullptr;
    }

    object_t& get()
    {
        assert(obj_);
        return *obj_;
    }

    const object_t& get() const
    {
        assert(obj_);
        return *obj_;
    }

    object_t* operator->()
    {
        assert(obj_);
        return obj_;
    }

    const object_t* operator->() const
    {
        assert(obj_);
        return obj_;
    }

    bool operator==(const object_ref_t& ref) const
    {
        return obj_ == ref.obj_;
    }

    void dispose()
    {
        if (obj_) {
            dec();
            obj_ = nullptr;
        }
    }

protected:
    void dec();
    void inc();

    object_t* obj_;
};

struct object_t {

    object_t(object_type_t type)
        : type_(type)
        , alive_(true)
        , order_(0)
    {
        // retain reference to self
        ref_.inc();
    }

    virtual ~object_t()
    {
    }

    template <typename type_t>
    type_t& cast()
    {
        assert(is_a<type_t>());
        return *static_cast<type_t*>(this);
    }

    template <typename type_t>
    const type_t& cast() const
    {
        assert(is_a<type_t>());
        return *static_cast<const type_t*>(this);
    }

    template <typename type_t>
    type_t* try_cast()
    {
        return (is_a<type_t>()) ? *static_cast<type_t*>(this) : nullptr;
    }

    template <typename type_t>
    const type_t* try_cast() const
    {
        return (is_a<type_t>()) ? *static_cast<const type_t*>(this) : nullptr;
    }

    bool is_a(object_type_t type) const
    {
        return type_ == type;
    }

    template <typename type_t>
    bool is_a() const
    {
        return type_ == type_t::type();
    }

    bool is_a(const object_t& other) const
    {
        return type_ == other.type_;
    }

    object_ref_t get_ref();

    const object_type_t type_;

    uint32_t ref_count() const
    {
        return ref_.count();
    }

    bool is_disposed() const
    {
        return ref_.disposed();
    }

    virtual void tick() {};

    void destroy()
    {
        if (alive_) {
            ref_.dec();
            alive_ = false;
        }
    }

    bool is_alive() const
    {
        if (is_disposed()) {
            return false;
        }
        return alive_;
    }

    // std compare function for ordering
    static bool compare(const object_t * a, const object_t * b) {
        assert(a && b);
        return a->order_ < b->order_;
    }

protected:
    friend struct object_ref_t;

    int32_t order_;
    ref_t ref_;
    bool alive_;
};

struct object_factory_t {
    struct creator_t {
        virtual object_t* create(object_type_t,
            object_service_t) = 0;
        virtual void destroy(object_t*) = 0;
    };

    object_factory_t(object_service_t service)
        : service_(service)
    {
    }

    template <typename type_t>
    void add_creator()
    {
        add_creator(type_t::type(), type_t::creator());
    }

    // creator pointer will transfer ownership
    void add_creator(object_type_t type,
        creator_t* creator);

    template <typename type_t>
    object_ref_t create()
    {
        return create(type_t::type());
    }

    object_ref_t create(object_type_t type);

    template <typename type_t, typename... args_t>
    object_ref_t create(args_t&&... args)
    {
        object_ref_t ref = create<type_t>();
        ref->cast<type_t>().init(std::forward<args_t>(args)...);
        return ref;
    };

    // prune any dead objects
    void collect();

    // tick all objects
    void tick();

    // sort all objects according to their sort order
    void sort();

protected:
    // todo: <--- covert to vector
#if 1
    std::list<object_t*> obj_;
#else
    std::vector<object_t*> obj_;
#endif

    typedef std::unique_ptr<creator_t> up_object_creator_t;
    std::map<object_type_t, up_object_creator_t> creator_;

    // service object
    object_service_t service_;
};

template <typename type_t>
struct object_create_t : public object_factory_t::creator_t {
    virtual object_t* create(object_type_t, object_service_t service)
    {
        return new type_t(service);
    }

    virtual void destroy(object_t* obj)
    {
        delete static_cast<type_t*>(obj);
    }
};

template <object_type_t id_t, typename type_t>
struct object_ex_t : public object_t {
    object_ex_t()
        : object_t(type())
    {
    }

    static object_type_t type()
    {
        return id_t;
    }

    static object_factory_t::creator_t* creator()
    {
        return new object_create_t<type_t>();
    }
};
