#include <unordered_map>
#include <cassert>

template <typename key_t>
struct registry_t {

    template <typename type_t>
    void insert(key_t key, type_t & value) {
        map_[key] = node_t{get_tag<type_t>(), (void*)&value};
    }

    bool contains(key_t key) const {
        auto itt = map_.find(key);
        return itt != map_.end();
    }

    template <typename type_t>
    type_t * lookup(key_t key) {
        void * tag = get_tag<type_t>();
        auto itt = map_.find(key);
        if (itt == map_.end() || itt->second.tag_ != tag) {
            return nullptr;
        }
        else {
            assert(itt->first == key);
            return static_cast<type_t*>(itt->second.value_);
        }
    }

    template <typename type_t>
    const type_t * lookup(key_t key) const {
        void * tag = get_tag<type_t>();
        assert(tag);
        auto itt = map_.find(key);
        if (itt == map_.cend() || itt->second.tag_ != tag) {
            return nullptr;
        }
        else {
            assert(itt->first == key);
            assert(itt->second.value_);
            return static_cast<const type_t*>(itt->second.value_);
        }
    }

    template <typename type_t>
    void remove(key_t key) {
        void * tag = get_tag<type_t>();
        assert(tag);
        auto itt = map_.find(key);
        if (itt == map_.end() || itt->second.tag_ != tag) {
            // non existant object
        }
        else {
            assert(itt->first == key);
            map_.erase(itt);
        }
    }

protected:
    struct node_t {
        void * tag_;
        void * value_;
    };

    template <typename type_t>
    void* get_tag() const {
        static type_t dummy;
        return (void*) &dummy;
    }

    std::unordered_map<key_t, node_t> map_;
};
