#include "testing.h"
#include "../framework/objects.h"

namespace
{
    enum
    {
        e_type_obj_1,
        e_type_obj_2
    };

    struct test_obj_1_t : public object_t
    {
        test_obj_1_t()
            : object_t(type())
        {
        }

        static object_type_t type()
        {
            return e_type_obj_1;
        }

        static object_factory_t::creator_t * creator()
        {
            return new object_create_t<test_obj_1_t>();
        }

        static uint32_t magic() {
            return 0x12345678;
        }
    };


    struct test_obj_2_t : public object_t
    {
        test_obj_2_t()
            : object_t(type())
        {
        }

        static object_type_t type()
        {
            return e_type_obj_2;
        }

        static object_factory_t::creator_t * creator()
        {
            return new object_create_t<test_obj_2_t>();
        }
    };

    void object_test_1(testing_t & test)
    {
        object_factory_t factory;

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

        TEST_ASSERT(r1->cast<test_obj_1_t>().magic() ==
                    test_obj_1_t::magic());
    };
}

extern
bool test_objects(testing_t & testing)
{
    testing.add_test("object test 1", object_test_1);
    return false;
}
