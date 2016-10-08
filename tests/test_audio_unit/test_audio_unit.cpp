#include "../../framework_audio/source/chip/source_chip.h"

using namespace tengu;

struct test_t {
    test_t(const char * name) :
        name_(name),
        pass_(true)
    {}
    const char * const name_;
    virtual bool run() = 0;
    bool pass() const {
        return pass_;
    }
protected:
    bool pass_;
};

struct test_source_chip_t : public test_t {

    std::array<float, 1024*2> out_;

    test_source_chip_t()
        : test_t("test_source_chip_t")
    {}

    virtual bool run() override {
        using namespace source_chip;
        config_t config;
        config += config_t::entry_t{config_t::e_nestr, 0};
        audio_source_chip_t chip(44100);
        chip.init(config);

        chip.push(event_t{0, event_t::e_note_on,  {0, 64, 128}});
        chip.push(event_t{0, event_t::e_note_on,  {0, 65, 128}});
        chip.push(event_t{0, event_t::e_note_off, {0, 65, 128}});
        chip.push(event_t{0, event_t::e_note_off, {0, 64, 128}});

        chip.render(out_.data(), out_.size());
        return pass_;
    }
};

struct suite_t {

    std::vector<test_t *> tests_;

    suite_t() {
        tests_.push_back(new test_source_chip_t);
    }

    int32_t run() {
        int32_t fails = 0;
        for (test_t * test:tests_) {
            const bool res = test->run();
            printf("%s %s\n", res ? "pass - " : "fail - ", test->name_);
            fails += res ? 0 : 1;
        }
        return fails;
    }
};

int main(const int argc, char *args[]) {
    suite_t suite;
    return suite.run();
}
