#include "lsearch.hpp"
#include <algorithm>

constexpr u8 MONTE_DEPTH = 50;
constexpr u32 MONTE_SIMULATION_TIMES = 200;

Montecarlo::Montecarlo()
        : random(std::random_device()())
{}

/*
 * モンテカルロ法のアルゴリズム
 */
Node *Montecarlo::let_me_monte(Node *node)
{
        std::vector<PlayoutResult> result;
        u16 win, lose;

        // 一個下のノードを展開
        node->expand();

        // 各ノードに対してシュミレーションを行う
        for(Node *child : node->ref_children()){
                win = lose = 0;
                for(u8 i = 0;i < MONTE_SIMULATION_TIMES;i++){
                        switch(playout(child, MONTE_DEPTH)){
                        case WIN:
                                win++;
                                break;
                        case LOSE:
                                lose++;
                                break;
                        default:
                                break;
                        }
                }
                result.emplace_back((double)win / (double)MONTE_SIMULATION_TIMES, child);
        }
        
        // 勝率でソート
        std::sort(std::begin(result), std::end(result), [](const PlayoutResult r1, const PlayoutResult r2){ return r1.percentage > r2.percentage; });

#ifdef __DEBUG_MODE
        std::for_each(std::begin(result), std::end(result), [](PlayoutResult r){ std::cout << "Win:" << r.percentage * 100 << "%" << std::endl; });
#endif
        // 一番いい勝率のやつを返す
        return result.at(0).node;
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
