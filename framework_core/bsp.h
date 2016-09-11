#pragma once
#include <cstdint>
#include <vector>
#include <memory>

#include "vec2.h"

struct bsp_t {

    struct node_t {

        struct {
            vec2f_t n_;
            float d_;

            void set(const vec2f_t & a,
                     const vec2f_t & b) {

                n_ = vec2f_t::normalize(vec2f_t::cross(b-a));
                d_ = n_ * a;
            }
        } plane_;

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