#include "../../framework_core/event.h"

#define TEST_ASSERT(X) {if (!(X)) { return false; }}

struct local_listener_t : public event_listener_t {

    virtual void recieve_event(const event_t * e) {
        assert(e);
        received_.insert(e->type_);
    }

    bool has(uint32_t type) const {
        return received_.find(type)!=received_.end();
    }

    std::set<uint32_t> received_;
};

struct global_listener_t: public event_listener_t {

    virtual void recieve_event(const event_t * e) {
        assert(e);
        received_.insert(e->type_);
    }

    bool has(uint32_t type) const {
        return received_.find(type)!=received_.end();
    }

    std::set<uint32_t> received_;
};

bool test_event_1()
{
    event_stream_t stream;

    global_listener_t global;
    stream.add(&global);

    local_listener_t local;
    uint32_t filter[] = {1, 3, 5};
    stream.add(&local, filter);

    for (uint32_t i=0; i<4; ++i) {
        event_t e = {i};
        stream.send(&e);
    }

    stream.remove(&local, filter);

    {
        event_t e = {5};
        stream.send(&e);
    }

    stream.remove(&global);

    {
        event_t e = {6};
        stream.send(&e);
    }

    TEST_ASSERT(local.received_.size()==2);
    TEST_ASSERT(local.has(1));
    TEST_ASSERT(local.has(3));

    TEST_ASSERT(global.received_.size()==5);
    TEST_ASSERT(global.has(0));
    TEST_ASSERT(global.has(1));
    TEST_ASSERT(global.has(2));
    TEST_ASSERT(global.has(3));
    TEST_ASSERT(global.has(5));
    
    // <---- ---- ---- ---- ---- ---- ---- ---- ---- todo: test multiple locals / globals
    
    // <---- ---- ---- ---- ---- ---- ---- ---- ---- todo: test derived event types and safe casts

    return true;
}
