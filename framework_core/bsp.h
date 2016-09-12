#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <memory>

#include "geometry.h"
#include "vec2.h"

struct bsp_t {

    struct node_t {
        geometry::line_t<vec2f_t> plane_;
        std::array<node_t*, 2> child_;

        bool is_leaf() const {
            return !(child_[0] || child_[1]);
        }

        std::unique_ptr<std::vector<vec2f_t>> edge_;
    };

    bool compile(vec2f_t * p, size_t vertices);

protected:
    std::vector<node_t *> node_;
    node_t * root_;
};
