#include <cstdint>
#include <array>

#include <SDL/SDL.h>

#include "../../framework_core/common.h"
#include "../../framework_core/random.h"
#include "../../framework_core/astar.h"
#include "../../framework_draw/draw.h"

#include "test.h"

static const int32_t C_SIZE = 8;

struct map_waypoint_t {
    int32_t x, y;

    bool operator == (const map_waypoint_t & o) const {
        return o.x==x && o.y==y;
    }
};

struct map_t {
    static const uint32_t width_  = 48;
    static const uint32_t height_ = 32;

    random_t rand_;
    draw_t & draw_;
    std::array<uint8_t, width_*height_> tile_;

    map_t(draw_t & draw)
        : rand_(0x1234)
        , draw_(draw) {
    }

    void init() {
        for (int i = 0; i<width_*height_; ++i) {
            bool fill = false;
            // border
            fill |= ((i/width_)==0);
            fill |= ((i/width_)==height_-1);
            fill |= ((i%width_)==0);
            fill |= ((i%width_)==width_-1);
            // random crap
            fill |= (rand_.rand()&0x3)==0;
            tile_[i] = uint8_t(fill ? 0x1 : 0x0);
        }
    }

    uint8_t get(const map_waypoint_t & w) const {
        return tile_[w.x + w.y * width_];
    }

    void draw() {
        for (int i = 0; i<width_*height_; ++i) {
            if (tile_[i]) {
                const int32_t x = i % width_;
                const int32_t y = i / width_;
                draw_.colour_ = 0xff0000;
                draw_.rect(recti_t(x, y, 1, 1, recti_t::e_relative) * C_SIZE);
            }
        }
    }
};

struct pathfind_t final:
    pathfind::astar_t<map_waypoint_t, 1024, 128, 64*64, pathfind_t> {

    pathfind_t(map_t & map) : map_(map) {}

    virtual bool node_expand(const as_node_t & in,
                             const map_waypoint_t & goal) override final {
        // expand pattern
        static const std::array<int32_t, 4> dx = {-1, 1, 0, 0};
        static const std::array<int32_t, 4> dy = { 0, 0,-1, 1};
        
        // for all adjacent tile
        for (int i = 0; i<4; ++i) {
            map_waypoint_t wp{in.waypoint_.x+dx[i], 
                              in.waypoint_.y+dy[i]};
            // skip walls
            if (map_.get(wp))
                continue;

            // if this tile is fair game
            if (!mask_.is_set(linear_id(wp))) {
                as_node_t * n = node_alloc();
                n->parent_   = &in;
                n->waypoint_ = wp;
                n->length_   = in.length_+1;
                n->cost_     = in.length_ + node_cost(wp, goal);
                // push node into open set
                node_push(n);
            }
        }
        return true;
    }

    virtual uint32_t node_cost(const map_waypoint_t & dest,
                               const map_waypoint_t & pos) const override final {
        // manhattan distance
        int32_t dx = dest.x-pos.x;
        int32_t dy = dest.y-pos.y;
        return uint32_t(absv(dx) + absv(dy));
    }

    virtual size_t linear_id(const map_waypoint_t & a) const override final {
        // 2d coordinate to 1d index
        return a.x + a.y * map_.width_;
    }

protected:
    map_t & map_;
};

struct test_astar_t : public test_t {

    draw_t & draw_;
    map_waypoint_t start_;
    map_t map_;
    pathfind_t astar_;

    test_astar_t(draw_t & draw)
        : draw_(draw)
        , map_(draw)
        , astar_(map_)
    {
        start_ = map_waypoint_t{map_.width_/2,
                                map_.height_/2};
    }

    virtual void tick() override {

        draw_.colour_ = 0x404040;
        draw_.clear();
        map_.draw();
        // get mouse state
        int32_t mx, my;
        int32_t b = SDL_GetMouseState(&mx, &my);
        mx = clampv<int32_t>(1, mx/C_SIZE, map_.width_-2);
        my = clampv<int32_t>(1, my/C_SIZE, map_.height_-2);
        // check mouse buttons
        if (SDL_BUTTON_LMASK & b) {
            start_.x = mx;
            start_.y = my;
        }
        // set end point
        map_waypoint_t end{mx, my};
        // create waypoint stack to receive output path
        pathfind::stack_t<map_waypoint_t> path;
        // 
        bool path_found = false;
        // perform and A* search
        if (!map_.get(start_)&&!map_.get(end)) {
            if (astar_.search(start_, end, path)) {
                path_found = true;
            }
        }
        // debug draw the closed list
        for (int i = 0; i<map_.width_*map_.height_; ++i) {
            const int32_t x = i%map_.width_;
            const int32_t y = i/map_.width_;
            if (astar_.get_mask().is_set(x + y * map_.width_)) {
                rect(x, y, 0x202020);
            }
        }
        // debug draw the open list
        while (!astar_.heap().empty()) {
            auto node = astar_.heap().pop();
            map_waypoint_t wp = node->waypoint_;
            rect(wp.x, wp.y, 0x202090);
        }
        // draw the output path
        if (path_found) {
            while (!path.empty()) {
                rect(path.top().x, path.top().y, 0x00ff00);
                path.pop();
            }
        }
    }

    void rect(int32_t x, int32_t y, uint32_t rgb) {
        draw_.colour_ = rgb;
        draw_.rect(recti_t(x, y, 1, 1, recti_t::e_relative) * C_SIZE);
    }
};

extern test_t * new_test_astar(draw_t & draw) {
    test_astar_t * test = new test_astar_t(draw);
    test->map_.init();
    return test;
}