#include "bsp.h"

using namespace geometry;

enum {
    e_class_pos,
    e_class_neg,
    e_class_coincident,
    e_class_split,
};

namespace {

    int32_t classify(const linef_t &l,
                     const vec2f_t &p) {

        const float d = l.distance(p);
        if (fp_cmp(d, 0.f)) {
            return e_class_coincident;
        }
        return (d > 0.f) ? e_class_pos : e_class_neg;
    }

    int32_t classify(const linef_t &l,
                     const edgef_t &e) {

        int32_t code_a = classify(l, e.p0);
        int32_t code_b = classify(l, e.p1);
        // adjust coincident points
        if (code_a == e_class_coincident) {
            code_a = code_b;
        }
        if (code_b == e_class_coincident) {
            code_b = code_a;
        }
        // if they are the same then return
        if (code_a == code_b) {
            return code_a;
        }
        // neither can be coincident at this point and must split
        return e_class_split;
    }

    void split(const linef_t &l,
               const edgef_t &e,
               valid_t<edgef_t> & pos,
               valid_t<edgef_t> & neg) {

        switch (classify(l, e)) {
            case (e_class_pos):
            case (e_class_coincident):
                pos = e;
                break;

            case (e_class_neg):
                neg = e;
                break;

            case (e_class_split): {
                vec2f_t i;
                assert(intersect(l, e, i));
                if (l.distance(e.p0) > 0.f) {
                    pos = edgef_t{i, e.p0};
                    neg = edgef_t{e.p1, i};
                } else {
                    pos = edgef_t{i, e.p1};
                    neg = edgef_t{e.p0, i};
                }
                break;
            }
        }
    }

    linef_t best_splitter(const bsp_t::edge_list_t &edges) {
        using namespace geometry;
        linef_t out{edges[0].p0, edges[0].p1};
        float best = 0.f;
        bool first = true;
        for (const edgef_t &i : edges) {
            linef_t line{i.p0, i.p1};
            float score = 0.f;
            for (const edgef_t &j : edges) {
                score += classify(line, j);
            }
            if (first || (score < best)) {
                best = score;
                out = line;
                first = false;
            }
        }
        return out;
    }

} // namespace {}

bool bsp_t::compile(vec2f_t *p, size_t vertices) {
    return false;
}

bool bsp_t::split(node_t & node) const {
    using namespace geometry;

    linef_t splitter = best_splitter(*node.edge_.get());

    // todo start splitting etc

    return false;
}
