#include "../../framework_core/objects.h"

#define TEST_ASSERT(X) {if (X) { return false; }}

enum {
    e_type_obj_1,
    e_type_obj_2,
    e_type_obj_3,
};

struct test_obj_1_t : public object_t {
    test_obj_1_t(object_service_t)
        : object_t(type())
    {
        ref_.dec();
    }

    static object_type_t type()
    {
        return e_type_obj_1;
    }

    static object_factory_t::creator_t* creator()
    {
        return new object_create_t<test_obj_1_t>();
    }

    static uint32_t magic()
    {
        return 0x12345678;
    }
};

struct test_obj_2_t : public object_t {
    test_obj_2_t(object_service_t)
        : object_t(type())
    {
    }

    static object_type_t type()
    {
        return e_type_obj_2;
    }

    static object_factory_t::creator_t* creator()
    {
        return new object_create_t<test_obj_2_t>();
    }
};

struct test_obj_3_t : public object_t {
    struct make_t : public object_factory_t::creator_t {
        make_t()
            : ref_(0)
        {
        }
        virtual object_t* create(object_type_t, object_service_t service)
        {
            ++ref_;
            object_t* obj = new test_obj_3_t(service);
            return obj;
        }
        virtual void destroy(object_t* obj)
        {
            delete obj;
            --ref_;
        }
        int ref_;
    };

    test_obj_3_t(object_service_t)
        : object_t(type())
    {
        // give up our own reference
        ref_.dec();
    }

    static object_type_t type()
    {
        return e_type_obj_3;
    }
};

extern
bool test_object_1()
{
    object_factory_t factory(nullptr);

    factory.add_creator<test_obj_1_t>();
    factory.add_creator<test_obj_2_t>();

    object_ref_t r1 = factory.create<test_obj_1_t>();
    object_ref_t r2 = factory.create<test_obj_2_t>();

    object_ref_t r3 = factory.create(e_type_obj_1);
    object_ref_t r4 = factory.create(e_type_obj_2);

    TEST_ASSERT(r1.get().is_a(e_type_obj_1));
    TEST_ASSERT(!r1.get().is_a(e_type_obj_2));

    TEST_ASSERT(r2.get().is_a(e_type_obj_2));
    TEST_ASSERT(!r2.get().is_a(e_type_obj_1));

    TEST_ASSERT(r3.get().is_a(e_type_obj_1));
    TEST_ASSERT(!r3.get().is_a(e_type_obj_2));

    TEST_ASSERT(r4.get().is_a(e_type_obj_2));
    TEST_ASSERT(!r4.get().is_a(e_type_obj_1));

    TEST_ASSERT(!r1->is_a(r2.get()));
    TEST_ASSERT(r1->is_a(r3.get()));
    TEST_ASSERT(!r1->is_a(r4.get()));

    TEST_ASSERT(!r2->is_a(r1.get()));
    TEST_ASSERT(!r2->is_a(r3.get()));
    TEST_ASSERT(r2->is_a(r4.get()));

    TEST_ASSERT(r1->cast<test_obj_1_t>().magic() == test_obj_1_t::magic());

    return true;
};

extern
bool test_object_2()
{
    object_ref_t ref1;
    TEST_ASSERT(!ref1.valid());

    test_obj_3_t::make_t* make = new test_obj_3_t::make_t;

    object_factory_t factory(nullptr);
    factory.add_creator(e_type_obj_3, make);

    TEST_ASSERT(make->ref_ == 0);
    {
        object_ref_t obj1 = factory.create<test_obj_3_t>();
        TEST_ASSERT(obj1->ref_count() == 1);
        TEST_ASSERT(make->ref_ == 1);
        factory.collect();
        TEST_ASSERT(make->ref_ == 1);
    }
    factory.collect();
    TEST_ASSERT(make->ref_ == 0);

    return true;
}

extern
bool test_object_3()
{
    object_factory_t factory(nullptr);
    factory.add_creator<test_obj_1_t>();

    object_ref_t ref1;
    {
        object_ref_t ref2;
        TEST_ASSERT(!ref1.valid());
        {
            object_ref_t obj = factory.create<test_obj_1_t>();
            TEST_ASSERT(obj->ref_count() == 1);
            ref1 = obj;
            TEST_ASSERT(obj->ref_count() == 2);
            {
                ref2 = obj;
                TEST_ASSERT(obj->ref_count() == 3);
            }
            TEST_ASSERT(obj->ref_count() == 3);
        }
        TEST_ASSERT(ref1->ref_count() == 2);
    }
    TEST_ASSERT(ref1->ref_count() == 1);
    {
        object_t* obj;
        obj = &ref1.get();
        ref1.dispose();
        TEST_ASSERT(obj->ref_count() == 0);
    }
    return true;
}

extern
bool test_object_4()
{
    object_factory_t factory(nullptr);
    factory.add_creator<test_obj_1_t>();
    // try to create unregistered object
    object_ref_t obj1 = factory.create<test_obj_3_t>();
    TEST_ASSERT(!obj1.valid());
    // try to create registered object
    object_ref_t obj2 = factory.create<test_obj_1_t>();
    TEST_ASSERT(obj2.valid());

    return true;
}

extern
bool test_object_5()
{
    enum {
        e_obj_test_t
    };

    struct obj_test_t : public object_ex_t<e_obj_test_t, obj_test_t> {
        int32_t* service_;
        int32_t value_;

        obj_test_t(object_service_t service)
            : object_ex_t()
            , service_(static_cast<int32_t*>(service))
            , value_(0)
        {
        }

        void init(int value)
        {
        }
    };

    int32_t value;
    object_factory_t factory(&value);
    factory.add_creator<obj_test_t>();

    object_ref_t obj_1 = factory.create<obj_test_t>();
    TEST_ASSERT(obj_1->is_alive());
    TEST_ASSERT(obj_1->cast<obj_test_t>().service_ == &value);
    TEST_ASSERT(obj_1->cast<obj_test_t>().value_ == 0);

    object_ref_t obj_2 = factory.create<obj_test_t>(1);
    TEST_ASSERT(obj_2->is_alive());
    TEST_ASSERT(obj_2->cast<obj_test_t>().service_ == &value);
    TEST_ASSERT(obj_2->cast<obj_test_t>().value_ == 1);

    return true;
}
