#include <array>

extern bool test_anim_1();
extern bool test_file_1();
extern bool test_file_2();
extern bool test_file_3();
extern bool test_fsm_1();
extern bool test_object_1();
extern bool test_object_2();
extern bool test_object_3();
extern bool test_object_4();
extern bool test_object_5();
extern bool test_timer_1();
extern bool test_timer_2();

struct test_t {
    const char * name_;
    bool (*test_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 12> tests = {{
    TEST(test_anim_1),
    TEST(test_file_1),
    TEST(test_file_2),
    TEST(test_file_3),
    TEST(test_fsm_1),
    TEST(test_object_1),
    TEST(test_object_2),
    TEST(test_object_3),
    TEST(test_object_4),
    TEST(test_object_5),
    TEST(test_timer_1),
    TEST(test_timer_2),
}};

int main(const int argc, char *args[]) {
    for (const auto test : tests) {
        printf("%s %s\n", test.name_, test.test_() ? "pass" : "fail");
    }
    return 0;
}
