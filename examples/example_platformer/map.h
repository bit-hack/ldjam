#pragma once
#include "common.h"

struct obj_map_t : public tengu::object_ex_t<e_object_map, obj_map_t> {

    static const int32_t _MAP_WIDTH = 32;
    static const int32_t _MAP_HEIGHT = 32;
    static const int32_t _CELL_WIDTH = 16;
    static const int32_t _CELL_HEIGHT = 16;

    typedef tengu::vec2i_t vec2i_t;

    service_t & service_;
    tengu::collision_map_t collide_;

    obj_map_t(tengu::object_service_t s)
        : object_ex_t()
        , service_(*static_cast<service_t*>(s))
        , collide_(vec2i_t{_MAP_WIDTH, _MAP_HEIGHT},
                   vec2i_t {_CELL_WIDTH, _CELL_HEIGHT})
    {
        generate();
    }

    void generate() {
        using namespace tengu;

        tengu::collision_map_t & c = collide_;
        random_t random(0x12345);
        const int32_t _CHANCE = 10;
        c.clear(0);
        uint8_t * tile = c.get();
        for (int32_t y=0; y<c.size().y; ++y) {
            for (int32_t x=0; x<c.size().x; ++x) {
                bool set = random.rand_chance(_CHANCE);
                set |= y==0;
                set |= y==(c.size().y-1);
                set |= x==0;
                set |= x==(c.size().x-1);
                tile[x + y * c.size().x] = set ? e_tile_solid : 0;
            }
        }
        // preprocess map for collision data
        c.preprocess();
    }

    void draw() {
        using namespace tengu;

        collision_map_t & c = collide_;
        draw_ex_t & draw = service_.draw_;
        draw.colour_ = 0x4092c3;
        uint8_t * tile = c.get();
        for (int32_t y=0; y<c.size().y; ++y) {
            for (int32_t x=0; x<c.size().x; ++x) {
                const uint8_t t = tile[x + y * c.size().x];
                if (!(t & e_tile_solid)) {
                    continue;
                }
                draw.rect<true>(recti_t(x * 16, y * 16, 16, 16, recti_t::e_relative));
            }
        }
    }

    virtual void tick() override {
        draw();
    }
};