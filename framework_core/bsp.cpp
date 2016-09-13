#include "bsp.h"

bool bsp_t::compile(vec2f_t *p, size_t vertices) {
    return false;
}

float bsp_t::classify(const geometry::line_t<vec2f_t> & l,
                      const geometry::edge_t<vec2f_t> & e) const {

    return false;
}

geometry::line_t<vec2f_t> bsp_t::best_splitter(const edge_list_t & edges) const {
    using namespace geometry;
    line_t<vec2f_t> out { edges[0].p0, edges[0].p1 };
    float best = 0.f;
    bool first = true;
    for (const edge_t<vec2f_t> & i : edges) {
        line_t<vec2f_t> line {i.p0, i.p1};
        float score = 0.f;
        for (const edge_t<vec2f_t> & j : edges) {
            score += classify(line, j);
        }
        if (first || (score < best)) {
            score = best;
            out = line;
            first = false;
        }
    }
    return out;
}

bool bsp_t::split(node_t & node) const {
    using namespace geometry;

    line_t<vec2f_t> splitter = best_splitter(*node.edge_.get());

    // todo start splitting etc

    return false;
}
