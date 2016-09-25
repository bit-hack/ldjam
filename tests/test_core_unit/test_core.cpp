#include <array>

extern bool test_anim_1();
extern bool test_file_1();
extern bool test_file_2();
extern bool test_file_3();
extern bool test_file_4();
extern bool test_fsm_1();
extern bool test_object_1();
extern bool test_object_2();
extern bool test_object_3();
extern bool test_object_4();
extern bool test_object_5();
extern bool test_object_6();
extern bool test_object_7();
extern bool test_timer_1();
extern bool test_timer_2();
extern bool test_event_1();
extern bool test_buffer_1();

struct test_t {
    const char * name_;
    bool (*test_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 17> tests = {{
    TEST(test_anim_1),
    TEST(test_file_1),
    TEST(test_file_2),
    TEST(test_file_3),
    TEST(test_file_4),
    TEST(test_fsm_1),
    TEST(test_object_1),
    TEST(test_object_2),
    TEST(test_object_3),
    TEST(test_object_4),
    TEST(test_object_5),
    TEST(test_object_6),
    TEST(test_object_7),
    TEST(test_timer_1),
    TEST(test_timer_2),
    TEST(test_event_1),
    TEST(test_buffer_1),
}};

int main(const int argc, char *args[]) {
    int32_t fails = 0;
    for (const auto test : tests) {
        bool pass = test.test_();
        fails += pass ? 0 : 1;
        printf("%s %s\n", test.name_, pass ? "pass" : "fail");
    }
    return fails;
}
