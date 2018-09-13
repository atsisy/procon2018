#include "lsearch.hpp"
#include <algorithm>
#include <numeric>
#include <chrono>
#include "utility.hpp"

constexpr u32 MONTE_FINAL_TIMES = 2000;
constexpr u32 MONTE_MIN_TIMES = 50;
constexpr u32 MONTE_ADDITIONAL_SIM_TIMES = 20;
constexpr double MONTE_THD_WIN_PERCENTAGE = 0.7;
constexpr double MONTE_INCREASE_RATE = 1.07;
constexpr double MONTE_TIME_LIMIT = 7000;


Montecarlo::Montecarlo()
{}

static inline double get_win_average(std::vector<PlayoutResult *> &data)
{
        double avg_percentage = 0;
        for(u8 i = 0;i < data.size();i++){
                avg_percentage += data.at(i)->percentage();
        }
        return avg_percentage / (double)data.size();
}

const Node *Montecarlo::get_first_child(const Node *node)
{
        if(node->parent->parent == NULL)
                return node;
        return get_first_child(node->parent);
}

void Montecarlo::expand_node(Node *node, std::function<void(Node *)> apply_child)
{
        node->expand();
        std::for_each(std::begin(node->ref_children()), std::end(node->ref_children()), apply_child);
}

/*
 * モンテカルロ法のアルゴリズム
 */
const Node *Montecarlo::let_me_monte(Node *node, u8 depth)
{
        std::vector<PlayoutResult *> original, result;
        u16 limit;
        u64 total_trying = 0;
        std::chrono::system_clock::time_point start, end;
        start = end = std::chrono::system_clock::now();

        // 一個下のノードを展開
        expand_node(node, [&result, &original, &total_trying, depth, this](Node *child){
                                  PlayoutResult *tmp = new PlayoutResult(child, nullptr);
                                  total_trying += playout_process(tmp, 700, depth);
                                  result.push_back(tmp);
                                  original.push_back(tmp); });

        // 各ノードに対してシュミレーションを行う        
        limit = MONTE_MIN_TIMES;

        printf("thinking");
        while(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <= MONTE_TIME_LIMIT){
                //printf("%ldnodes\n", result.
                putchar('.');
                fflush(stdout);
                std::for_each(std::begin(result), std::end(result),
                              [total_trying](PlayoutResult *p){ p->calc_ucb(total_trying);});

                // UCBでソート
                std::sort(std::begin(result), std::end(result), [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->ucb > r2->ucb; });

                {
                        PlayoutResult *target;
                        //u8 index = 0;
                        //while(result.at(index++)->expanded);

                        target = result.at(0);
                        
                        if(target->ucb == 0){
                                puts("error");
                                exit(0);
                        }
                        total_trying += playout_process(target, limit, depth);

                        if(target->trying >= 5000){
                                target->ucb = 0;
                                expand_node(target->node, [&](Node *child){
                                                PlayoutResult *tmp = new PlayoutResult(child, target);
                                                total_trying += playout_process(tmp, 100, depth);
                                                result.push_back(tmp);
                                        });
                        }

                        end = std::chrono::system_clock::now();
                }

//std::cout << "TOP -> WIN RATE = " << result.at(0)->percentage << ", UCB = " << result.at(0)->ucb << std::endl;
        }
        putchar('\n');

        std::sort(std::begin(original), std::end(original), [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->trying > r2->trying; });
        
#ifdef __DEBUG_MODE
        std::for_each(std::begin(original), std::end(original), [](PlayoutResult *r){ std::cout << "Win:" << r->percentage() * 100 << "%" << std::endl; });
#endif
        // 一番いい勝率のやつを返す
        printf("***TOTAL TRYING***  ========>  %ld\n", total_trying);
        std::cout << (int)original.at(0)->trying << "trying" << std::endl;
        return get_first_child(original.at(0)->node);
}

/*
void Montecarlo::apply_playout_to_data(std::vector<PlayoutResult> &data, int limit)
{
        Node *child;
        for(PlayoutResult &p : data){
                child = p.node;
                LocalPlayoutResult &&result = this->playout_process(child, limit);
                p.update(result.times, result.win);
        }
}
*/

u64 Montecarlo::playout_process(PlayoutResult *tmp, u16 limit, u8 depth)
{
        u16 win, i;
        
        win = 0;
        for(i = 0;i < limit;i++)
                if(faster_playout(tmp->node, depth) == WIN)
                        win++;
        tmp->update(i, win);
        return i;
}

/*
 * 一手分のシュミレーションを行う
 */
Node *Montecarlo::simulation(Node *node)
{
        Direction d1, d2;
        
        do{
                d1 = int_to_direction(random() % 9);
        }while(
                (node->turn && !node->enemy_agent1.is_movable(node->field, d1)) ||
                (!node->turn && !node->my_agent1.is_movable(node->field, d1)));
        do{
                d2 = int_to_direction(random() % 9);
        }while(
                (node->turn && !node->enemy_agent2.is_movable(node->field, d2)) ||
                (!node->turn && !node->my_agent2.is_movable(node->field, d2)));
        return node->get_specific_child(d1, d2);
}

/*
 * Playoutを実行する
 */
Judge Montecarlo::playout(Node *node, u8 depth)
{
        Node *current = node;
        Node *prev = node;

        current = simulation(current);
        
        while(depth--){
                prev = current;
                current = simulation(current);
                delete prev;
        }

        if(current->evaluate() > 0)
                return WIN;
        else if(current->evaluate() < 0)
                return LOSE;
        else
                return DRAW;
}

std::array<Direction, 4> Montecarlo::find_random_legal_direction(Node *node)
{
        Direction m1, m2, e1, e2;
        
        do{
                m1 = int_to_direction(random() % 9);
        }while(!node->my_agent1.is_movable(node->field, m1));
        do{
                m2 = int_to_direction(random() % 9);
        }while(!node->my_agent2.is_movable(node->field, m2));
        do{
                e1 = int_to_direction(random() % 9);
        }while(!node->enemy_agent1.is_movable(node->field, e1));
        do{
                e2 = int_to_direction(random() % 9);
        }while(!node->enemy_agent2.is_movable(node->field, e2));

        return check_direction_legality(node, {m1, m2, e1, e2});
}

std::array<Direction, 4> Montecarlo::check_direction_legality(Node *node, std::array<Direction, 4> dirs)
{
        Direction m1, m2, e1, e2;

        m1 = dirs[0];
        m2 = dirs[1];
        e1 = dirs[2];
        e2 = dirs[3];
        
        if(node->my_agent1.check_conflict(dirs[0], node->my_agent2, dirs[1]))
                m1 = m2 = STOP;
        if(node->my_agent1.check_conflict(dirs[0], node->enemy_agent1, dirs[2]))
                m1 = e1 = STOP;
        if(node->my_agent1.check_conflict(dirs[0], node->enemy_agent2, dirs[3]))
                m1 = e2 = STOP;
        if(node->my_agent2.check_conflict(dirs[1], node->enemy_agent1, dirs[2]))
                m2 = e1 = STOP;
        if(node->my_agent2.check_conflict(dirs[1], node->enemy_agent2, dirs[3]))
                m2 = e2 = STOP;
        if(node->enemy_agent1.check_conflict(dirs[2], node->enemy_agent2, dirs[3]))
                e1 = e2 = STOP;
        
        return {m1, m2, e1, e2};
}

i8 Montecarlo::random_half_play(Node *node, u8 turn)
{
        Direction d1, d2;
        
        do{
                d1 = int_to_direction(random() % 9);
        }while(
                (turn && !node->enemy_agent1.is_movable(node->field, d1)) ||
                (!turn && !node->my_agent1.is_movable(node->field, d1)));
        do{
                d2 = int_to_direction(random() % 9);
        }while(
                (turn && !node->enemy_agent2.is_movable(node->field, d2)) ||
                (!turn && !node->my_agent2.is_movable(node->field, d2)));

        if(IS_MYTURN(turn)){
                if(node->my_agent1.check_conflict(d1, node->my_agent2, d2))
                        return -1;
        }else{
                if(node->enemy_agent1.check_conflict(d1, node->enemy_agent2, d2))
                        return -1;
        }
        
        node->play_half(d1, d2, turn);

        return 0;
}

Judge Montecarlo::faster_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Judge result;
        
        /*
         * ここで一回ランダムに片方が行う必要がある
         */
        if(random_half_play(current, node->turn) == -1){
                result = DRAW;
                return result;
        }

        while(depth--){
                current->play(find_random_legal_direction(current));
        }

        if(current->evaluate() < 0){
#ifdef I_AM_ENEMY
                result = WIN;
#endif
#ifdef I_AM_ME
                result = LOSE;
#endif
        }else if(current->evaluate() > 0){
#ifdef I_AM_ENEMY
                result = LOSE;
#endif
#ifdef I_AM_ME
                result = WIN;
#endif
        }else
                result = DRAW;
        
        delete current;
        return result;
}
