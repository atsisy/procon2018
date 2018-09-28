#pragma once

#include "types.hpp"

struct te {
        Direction direction;
        u64 total;
        float percentage;

        te() {}
        te(Direction dir) { direction = dir; total = 0; percentage = 0; }
        float calc_percentage(u64 global_total)
        {
                return (percentage = (((double)total / (double)global_total) * 100.0));
        }
};

struct te_list {
        std::vector<te> list;
        u64 total;

        void add(Direction dir);
        void sort();

        te random_select();
        te random_select(u32 r);
};

extern std::unordered_map<u64, te_list *> learning_map;







