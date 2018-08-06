#include "lsearch.hpp"
#include <algorithm>
#include <numeric>

constexpr u8 MONTE_DEPTH = 60;
constexpr u32 MONTE_FINAL_TIMES = 2000;
constexpr u32 MONTE_MIN_TIMES = 200;
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
        std::vector<PlayoutResult> *result = new std::vector<PlayoutResult>;
        std::vector<PlayoutResult> *next = new std::vector<PlayoutResult>;
        
        u16 win, lose, limit, i;
        double avg_percentage;

        // 一個下のノードを展開
        node->expand();

        std::for_each(std::begin(node->ref_children()), std::end(node->ref_children()),
                      [&result](Node *child){ result->emplace_back(0, child); });

        // 各ノードに対してシュミレーションを行う
        
        limit = MONTE_MIN_TIMES;

        while(result->size() > 1){
                for(PlayoutResult &p : *result){
                        Node *child = p.node;
                        win = lose = 0;
                        for(i = 0;i < limit;i++){
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
                
                        next->emplace_back((double)win / (double)i, child);
                }
                
                // 勝率でソート
                std::sort(std::begin(*next), std::end(*next), [](const PlayoutResult r1, const PlayoutResult r2){ return r1.percentage > r2.percentage; });
                // 平均勝率を算出
                avg_percentage = 0;
                for(u8 i = 0;i < next->size();i++){
                        avg_percentage += next->at(i).percentage;
                }
                avg_percentage /=  (double)result->size();

                std::cout << "TOP -> " << next->at(0).percentage << std::endl;
                
                delete result;
                result = new std::vector<PlayoutResult>;
                for(u8 i = 0;i < next->size();i++){
                        if(next->at(i).percentage > avg_percentage)
                                result->push_back(next->at(i));
                        else
                                break;
                }
                delete next;
                next = new std::vector<PlayoutResult>;
        }

#ifdef __DEBUG_MODE
        std::for_each(std::begin(*result), std::end(*result), [](PlayoutResult r){ std::cout << "Win:" << r.percentage * 100 << "%" << std::endl; });
#endif
        // 一番いい勝率のやつを返す
        return result->at(0).node;
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

void Montecarlo::random_half_play(Node *node, u8 turn)
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
                        d1 = d2 = STOP;
        }else{
                if(node->enemy_agent1.check_conflict(d1, node->enemy_agent2, d2))
                        d1 = d2 = STOP;
        }
        
        node->play_half(d1, d2, turn);
}

Judge Montecarlo::faster_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Judge result;
        
        /*
         * ここで一回ランダムに片方が行う必要がある
         */
        random_half_play(current, MY_TURN);
        
        while(depth--){
                current->play(find_random_legal_direction(current));
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
