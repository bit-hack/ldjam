#include "../framework/file.h"

#include "testing.h"

namespace {
bool test_file_1(testing_t& test)
{
    file_reader_t file;
    TEST_ASSERT(file.open("test.bin"));
    TEST_ASSERT(file.is_open());

    return true;
}

bool test_file_2(testing_t& test)
{
    file_reader_t file("test.bin");
    TEST_ASSERT(file.is_open());

    file_reader_t new_file = std::move(file);
    TEST_ASSERT(new_file.is_open());

    return true;
}

bool test_file_3(testing_t& test)
{
    file_reader_t file;
    TEST_ASSERT(!file.is_open());
    TEST_ASSERT(file.open("nonexistant.file") == false);
    TEST_ASSERT(!file.is_open());
    return true;
}
};

extern void register_test_file(testing_t& testing)
{
    testing.add_test("file test 1", test_file_1);
    testing.add_test("file test 2", test_file_2);
    testing.add_test("file test 3", test_file_3);
}
