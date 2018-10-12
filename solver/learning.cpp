#include "learn.hpp"
#include "utility.hpp"
#include <unordered_map>
#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>

struct db_element {
        u64 hash;
        Direction dir;
};


void te_list::add(Direction direction)
{
        ++total;
        for(te &t : list){
                if(t.direction == direction){
                        t.total++;
                        return;
                }
        }

        te t(direction);
        t.total++;
        list.push_back(t);
        return;
}

void te_list::sort()
{
        std::sort(std::begin(list), std::end(list),
                  [](const te &t1, const te &t2){
                          return t1.percentage > t2.percentage;
                                  });
}

std::vector<Direction> te_list::get_list()
{
        std::vector<Direction> vec;

        for(te &t : list)
                vec.push_back(t.direction);

        return vec;
}

std::vector<Direction> te_list::get_filtered_list(Field *field, Agent &agent)
{
        std::vector<Direction> vec;

        for(te &t : list){
                /*
                if(agent.is_movable(field, t.direction))
                        vec.push_back(t.direction);
                */
        }

        return vec;
}

te te_list::random_select()
{
        util::xor128 random;
        double random_variable = (double)(random() % 101);
        te ret;

        for(te t : list){
                ret = t;
                random_variable -= t.percentage;
                if(random_variable < 0)
                        break;
        }

        return ret;
}

te te_list::random_select(u32 r)
{
        double random_variable = (double)(r % 101);
        te ret;

        for(te t : list){
                ret = t;
                random_variable -= t.percentage;
                if(random_variable < 0)
                        break;
        }

        return ret;
}

void test(std::unordered_map<u64, te_list *> &&map)
{
        i64 hash;
        puts("hello");
        while(true){
                std::cin >> hash;
                try{
                        te t = map.at(hash)->random_select();
                        std::cout << "Direction: " << t.direction << "\t"
                                "Percentage: " << t.percentage << "%\t"
                                "Total: " << t.total << std::endl;
                }catch(const std::out_of_range &e){
                        std::cout << "out" << std::endl;
                        continue;
                }
        }
}

std::unordered_map<u64, te_list *> analyze_learning_data(const char *file)
{
        std::unordered_map<u64, te_list *> map;

        int fd = open(file, O_RDONLY);
        if(fd < 0) {
                printf("Error : can't open file\n");
                exit(0);
        }
        void *mapped = (void *)mmap(NULL, 512000000, PROT_READ, MAP_SHARED, fd, 0);
        u64 hash;
        i32 dir;

        u64 size = *(u64 *)mapped;
        std::cout << "Size of data base: " << size << std::endl;
        db_element *elements = (db_element *)((u64)mapped + sizeof(u64));
        map.reserve(1000000);
        while(size--){
                hash = elements->hash;
                dir = elements->dir;
                
                if(map.find(hash) == std::end(map)){
                        te_list *tmp = new te_list;
                        map[hash] = tmp;
                }
                map[hash]->add(int_to_direction(dir));

                elements++;
        }

        for(std::pair<const long unsigned int, te_list*> &pair : map){
                for(te &t : pair.second->list){
                        t.calc_percentage(pair.second->total);
                }
                pair.second->sort();
        }

        return map;
}

/*
int main(int argc, char **argv)
{
        analyze_learning_data(argv[1]);
        return 0;
}

*/
