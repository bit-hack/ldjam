#include <array>
#include "../test_lib/test_lib.h"
#include "../../framework_core/registry.h"

using namespace test_lib;

struct test_core_registry_t: public test_t {

    test_core_registry_t()
        : test_t("test_core_registry_t")
    {
    }

    virtual bool run() override {
        using namespace tengu;

        struct object_t {
            int val_;
        };

        int32_t temp1 = 10;
        object_t tempobj{1337};

        registry_t<const char *> reg;
        reg.insert<int32_t>("int_key", temp1);
        reg.insert("obj_key", tempobj);

        int32_t * val = reg.lookup<int32_t>("int_key");
        TEST_ASSERT(val!=nullptr);
        TEST_ASSERT(val==&temp1);

        TEST_ASSERT(reg.contains("int_key"));
        TEST_ASSERT(!reg.contains("non_existant"));

        object_t * test1 = reg.lookup<object_t>("obj_key");
        TEST_ASSERT(test1!=nullptr);
        TEST_ASSERT(test1->val_==1337);

        int32_t * thing = reg.lookup<int32_t>("bad_key");
        TEST_ASSERT(thing==nullptr);

        reg.remove<int32_t>("int_key");
        TEST_ASSERT(!reg.contains("int_key"));

        reg.remove<float>("invalid_item");

        return true;
    }
};

static std::array<register_t*, 1> reg_test = {
    register_t::test<test_core_registry_t>()
};
