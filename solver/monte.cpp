#include "lsearch.hpp"
#include <algorithm>

constexpr u8 MONTE_DEPTH = 60;
constexpr u32 MONTE_FINAL_TIMES = 2000;
constexpr u32 MONTE_MIN_TIMES = 90;
constexpr u32 MONTE_ADDITIONAL_SIM_TIMES = 20;
constexpr double MONTE_THD_WIN_PERCENTAGE = 0.7;

Montecarlo::Montecarlo()
        : random(std::random_device()())
{}

/*
 * モンテカルロ法のアルゴリズム
 */
Node *Montecarlo::let_me_monte(Node *node)
{
        std::vector<PlayoutResult> result;
        u16 win, lose, limit, i;

        // 一個下のノードを展開
        node->expand();

        // 各ノードに対してシュミレーションを行う
        for(Node *child : node->ref_children()){
                win = lose = 0;
                limit = MONTE_MIN_TIMES;
                
                for(i = 0;i <= limit && i < MONTE_FINAL_TIMES;i++){
                        if(i == limit && ((double)win / (double)i) > MONTE_THD_WIN_PERCENTAGE){
                                limit += MONTE_ADDITIONAL_SIM_TIMES;
                        }
                        switch(faster_playout(child, MONTE_DEPTH)){
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
                result.emplace_back((double)win / (double)i, child);
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

Judge Montecarlo::faster_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Direction d1, d2;
        bool turn = false;
        Judge result;
        
        while(depth--){
                do{
                        d1 = int_to_direction(random() % 9);
                }while(
                        (!turn && !current->enemy_agent1.is_movable(current->field, d1)) ||
                        (turn && !current->my_agent1.is_movable(current->field, d1)));
                do{
                        d2 = int_to_direction(random() % 9);
                }while(
                        (!turn && !current->enemy_agent2.is_movable(current->field, d2)) ||
                        (turn && !current->my_agent2.is_movable(current->field, d2)));
                current->play(turn, d1, d2);
                turn = !turn;
        }

        if(current->evaluate() < 0)
                result = WIN;
        else if(current->evaluate() > 0)
                result = LOSE;
        else
                result = DRAW;
        delete current;
        return result;
}
