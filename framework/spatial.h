#include <cstdint>
#include <array>
#include <list>
#include <unordered_set>
#include <cstring>
#include <functional>

struct body_t
{
    float x, y;
    float vx, vy;
    float r;
};

struct body_pair_set_t
{
    typedef std::pair<body_t*, body_t*> pair_t;

    struct hash_func_t
    {
        static uint64_t hash(uint64_t key)
        {
            key += ~(key << 32);
            key ^=  (key >> 22);
            key += ~(key << 13);
            key ^=  (key >>  8);
            key +=  (key <<  3);
            key ^=  (key >> 15);
            key += ~(key << 27);
            key ^=  (key >> 31);
            return key;
        }

        size_t operator () (const pair_t & in) const
        {
            return hash(uint64_t(in.first)^
                   hash(uint64_t(in.second)));
        }
    };

    void insert(body_t * a, body_t * b);

    void clear();

    typedef std::unordered_set<pair_t, hash_func_t> base_t;

    base_t::iterator begin() {
        return base_.begin();
    }

    base_t::iterator end() {
        return base_.end();
    }

protected:
    base_t base_;
};

struct body_set_t
{
    typedef std::unordered_set<body_t*> base_t;

    void insert(body_t * a);

    void clear();

    bool operator[] (body_t * a) const;

    base_t::iterator begin() {
        return base_.begin();
    }

    base_t::iterator end() {
        return base_.end();
    }

protected:
    base_t base_;
};

struct spatial_t
{
    static const uint32_t width = 32;
    static const uint32_t items = 1024;

    struct slot_t
    {
        int32_t x, y;
        body_t * obj;
    };

    struct bound_t
    {
        int x0, y0;
        int x1, y1;

        bool contains(int32_t x, int32_t y) const
        {
            return x>=x0 && x<=x1 && y>=y0 && y<=y1;
        }

        bool operator == (const bound_t & o) const
        {
            return memcmp(this, &o, sizeof(o))==0;
        }
    };

    std::array<std::list<slot_t>, items> hash_;

    std::list<slot_t> & slot(int32_t x, int32_t y);

    bound_t object_bound(body_t * obj);

    void insert(body_t * obj);

    void slot_erase(int32_t x, int32_t y, body_t * obj);

    void slot_insert(int32_t x, int32_t y, body_t * obj);

    void remove(body_t * obj);

    void move(body_t * obj, std::function<void(body_t&)> func)
    {
        // old object bound
        bound_t ob0 = object_bound(obj);
        // move object
        func(*obj);
        // new object bound
        bound_t ob1 = object_bound(obj);
        // update hash map
        move(obj, ob0, ob1);
    }

    void query_collisions(body_pair_set_t & out);

    void query_radius(float x, float y, float r, body_set_t & out);

    void query_rect(float x0, float y0, float x1, float y1, body_set_t & out);

    void query_ray(float x0, float y0, float x1, float y1, body_set_t & out);

    int dbg_ocupancy(int32_t x, int32_t y);

protected:
    void move(body_t *, const bound_t &, const bound_t &);

    void xform_in(int32_t & x, int32_t & y)
    {
        x /= width;
        y /= width;
    }
};
