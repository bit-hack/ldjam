#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <memory>

#include "common.h"
#include "geometry.h"
#include "vec2.h"

struct bsp_t {

//    typedef geometry::edgef_t edgef_t;
//    typedef geometry::linef_t linef_t;

    typedef std::vector<geometry::edgef_t> edge_list_t;

    struct node_t {
        geometry::linef_t plane_;
        std::array<node_t*, 2> child_;

        bool is_leaf() const {
            return !(child_[0] || child_[1]);
        }

        std::unique_ptr<edge_list_t> edge_;
    };

    bool compile(vec2f_t * p, size_t vertices);

protected:

    bool split(node_t & node) const;

    std::vector<node_t *> node_;
    node_t * root_;
};
