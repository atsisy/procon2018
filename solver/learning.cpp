#include "learn.hpp"
#include "utility.hpp"
#include <unordered_map>
#include <cstdio>

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
                if(agent.is_movable(field, t.direction))
                        vec.push_back(t.direction);
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
        std::ifstream ifs(file);
        std::unordered_map<u64, te_list *> map;
        char str[256];
        u64 hash;
        i32 dir;
        
        if(ifs.fail()){
                std::cerr << "failed to open the file" << std::endl;
                exit(-1);
        }

        while(ifs.getline(str, 256 - 1)){
                sscanf(str, "%ld %d", &hash, &dir);
                if(map.find(hash) == std::end(map)){
                        te_list *tmp = new te_list;
                        map[hash] = tmp;
                }
                map[hash]->add(int_to_direction(dir));
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
        test(analyze_learning_data(argv[1]));
        return 0;
}
*/
