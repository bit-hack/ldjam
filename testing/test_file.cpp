#include "../framework_core/file.h"

#include "testing.h"

namespace {
bool test_file_1(testing_t& test)
{
    const char * PATH = "out.bin";

    file_reader_t file;
    TEST_ASSERT(file.open(PATH));
    TEST_ASSERT(file.is_open());

    std::string path;
    TEST_ASSERT(file.get_path(path));
    TEST_ASSERT(path==PATH);

    size_t p0 = 0;
    TEST_ASSERT(file.get_pos(p0));
    TEST_ASSERT(p0==0x0);

    size_t size = 0;
    TEST_ASSERT(file.size(size));
    TEST_ASSERT(size==0x58);

    uint32_t babe = 0;
    TEST_ASSERT(file.read(babe));
    TEST_ASSERT(babe==0xDEAFBEEF);

    size_t p1 = 0;
    TEST_ASSERT(file.get_pos(p1));
    TEST_ASSERT(p1==0x4);

    std::string str1;
    TEST_ASSERT(file.read_cstr(str1));
    TEST_ASSERT(str1=="Hello World");

    size_t p2 = 0;
    TEST_ASSERT(file.get_pos(p2));
    TEST_ASSERT(p2==0x10);

    std::string str2;
    TEST_ASSERT(file.read_cstr(str2));
    TEST_ASSERT(str2=="A C++ String");

    size_t p3 = 0;
    TEST_ASSERT(file.get_pos(p3));
    TEST_ASSERT(p3==0x1D);

    std::array<char, 5> hello = {{0}};
    TEST_ASSERT(file.read(hello));
    for (int i = 0; i<hello.size(); ++i)
        TEST_ASSERT(hello[i]=="Hello"[i]);

    size_t p4 = 0;
    TEST_ASSERT(file.get_pos(p4));
    TEST_ASSERT(p4==0x22);

    char hithere[0xA] = {0};
    TEST_ASSERT(file.read(hithere));
    for (int i = 0; i<sizeof(hithere); ++i)
        TEST_ASSERT(hithere[i]=="Hi there!"[i]);

    size_t p5 = 0;
    TEST_ASSERT(file.get_pos(p5));
    TEST_ASSERT(p5==0x2C);

    TEST_ASSERT(file.seek(0x30));
    size_t p6 = 0;
    TEST_ASSERT(file.get_pos(p6));
    TEST_ASSERT(p6==0x30);

    uint16_t a16[] = {0, 0, 0, 0};
    TEST_ASSERT(file.read(a16));
    TEST_ASSERT(a16[0]==1);
    TEST_ASSERT(a16[1]==2);
    TEST_ASSERT(a16[2]==3);
    TEST_ASSERT(a16[3]==4);

    size_t p7 = 0;
    TEST_ASSERT(file.get_pos(p7));
    TEST_ASSERT(p7==0x38);

    TEST_ASSERT(file.seek(0x40));
    std::string pstr1;
    TEST_ASSERT(file.read_pstr<uint16_t>(pstr1));
    TEST_ASSERT(pstr1=="A PString!");

    TEST_ASSERT(file.close());
    TEST_ASSERT(!file.is_open());

    return true;
}

bool test_file_2(testing_t& test)
{
#if 0
    file_reader_t file("temp.bin");
    TEST_ASSERT(file.is_open());

    file_reader_t new_file = std::move(file);
    TEST_ASSERT(new_file.is_open());
#endif

#if 0
    file_writer_t file;
    void * my_thing = (void*)10;
    file.write(my_thing);
#endif

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

bool test_file_4(testing_t& test)
{
    const char * PATH = "out.bin";

    file_writer_t file;
    TEST_ASSERT(!file.is_open());

    TEST_ASSERT(file.open(PATH));
    TEST_ASSERT(file.is_open());

    std::string path;
    TEST_ASSERT(file.get_path(path));
    TEST_ASSERT(path==PATH);

    size_t size = 0;
    TEST_ASSERT(file.size(size));
    TEST_ASSERT(size==0);

    const uint32_t value = 0xDEAFBEEF;
    TEST_ASSERT(file.write(value));

    TEST_ASSERT(file.size(size));
    TEST_ASSERT(size==4);

    TEST_ASSERT(file.write("Hello World"));

    TEST_ASSERT(file.size(size));
    TEST_ASSERT(size==4+12);

    TEST_ASSERT(file.write(std::string("A C++ String")));

    TEST_ASSERT(file.size(size));
    TEST_ASSERT(size==4+12+13);

    // warning, danger!  this could call the char* overload, if not carefull.
    const char almost_str[] = {'H', 'e', 'l', 'l', 'o'};
    TEST_ASSERT(file.write(almost_str));

    const char * my_str = "Hi there!";
    TEST_ASSERT(file.write(my_str));

    const uint8_t a8[] = {01, 02, 03, 04};
    TEST_ASSERT(file.write(a8));

    const std::array<uint16_t, 4> a16 = {{01, 02, 03, 04}};
    TEST_ASSERT(file.write(a16));

    TEST_ASSERT(file.fill<char>('\xCC', 8));

    const std::string pstr1 = "A PString!";
    TEST_ASSERT(file.write_pstr<uint16_t>(pstr1));

    const char * pstr2 = "A PString!";
    TEST_ASSERT(file.write_pstr<uint16_t>(pstr2));

    TEST_ASSERT(file.close());
    TEST_ASSERT(!file.is_open());
    return true;
}
};

extern void register_test_file(testing_t& testing)
{
    testing.add_test("file write test 1", test_file_4);
    testing.add_test("file read test 1", test_file_1);
    testing.add_test("file read test 2", test_file_2);
    testing.add_test("file read test 3", test_file_3);
}
