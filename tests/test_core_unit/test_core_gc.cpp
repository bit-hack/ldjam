#include <array>

#include "../../framework_core/gc.h"
#include "../test_lib/test_lib.h"

using namespace test_lib;

#define TEST_ASSERT(X) {if (!(X)) { return false; }}

struct object_t : public tengu::gc_object_t {

  object_t(int &r)
    : a(nullptr)
    , b(nullptr)
    , ref(r)
  {
    ++ref;
  }

  ~object_t() {
    --ref;
  }

  object_t *a, *b;
  int &ref;

  static void gc_enum(const gc_object_t *o, std::vector<const gc_object_t *> &out) {
    const auto *obj = static_cast<const object_t*>(o);
    if (obj->a) { out.push_back(obj->a); }
    if (obj->b) { out.push_back(obj->b); }
  }
};

struct test_gc_1_t: public test_t {

    test_gc_1_t()
        : test_t("test gc 1")
    {
    }

    bool run() {
        using namespace tengu;

        int ref = 0;

        gc_t gc;
        object_t *o0 = gc.alloc<object_t>(ref);

        o0->a = gc.alloc<object_t>(ref);
        o0->a->a = gc.alloc<object_t>(ref);
        o0->a->b = gc.alloc<object_t>(ref);

        TEST_ASSERT(ref == 4);
        gc.check_in(o0);
        gc.collect();
        TEST_ASSERT(ref == 4);

        o0->b = gc.alloc<object_t>(ref);
        o0->a->b = gc.alloc<object_t>(ref);
        TEST_ASSERT(ref == 6);

        o0->a = nullptr;
        gc.check_in(o0);
        gc.collect();
        TEST_ASSERT(ref == 2);

        return true;
    }
};

struct test_gc_2_t : public test_t {

    int ref;
    object_t* root;
    tengu::gc_t gc;

    test_gc_2_t()
        : test_t("test gc 2")
      , ref(0)
      , root(nullptr)
    {
    }

    bool run()
    {
        using namespace tengu;

        root = gc.alloc<object_t>(ref);
        TEST_ASSERT(root);

        for (int i = 0; i < 10000; ++i) {

          const auto &a = gc.allocs();
          TEST_ASSERT(!a.empty());

          size_t index = rand() % a.size();
          object_t *item = static_cast<object_t*>(a[index]);
          TEST_ASSERT(item);

          switch (rand() % 2) {
          case 0:
            item->a = gc.alloc<object_t>(ref);
            TEST_ASSERT(item->a);
            break;
          case 1:
            item->b = gc.alloc<object_t>(ref);
            TEST_ASSERT(item->b);
            break;
          }

          if (rand() % 20 == 0) {
            gc.check_in(root);
            gc.collect();
            TEST_ASSERT(ref == a.size());
          }
        }

        return true;
    }
};

static std::array<test_lib::register_t*, 2> reg_test = {
    test_lib::register_t::test<test_gc_1_t>(),
    test_lib::register_t::test<test_gc_2_t>()
};
