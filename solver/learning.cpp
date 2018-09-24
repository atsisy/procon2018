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

te te_list::random_select()
{
        xor128 random;
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
                        tmp->add(int_to_direction(dir));
                }
        }

        for(std::pair<const long unsigned int, te_list*> &pair : map){
                for(te t : pair.second->list){
                        t.calc_percentage(pair.second->total);
                }
                pair.second->sort();
        }

        return map;
}

int main(int argc, char **argv)
{
        analyze_learning_data(argv[1]);
        return 0;
}
