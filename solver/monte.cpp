#include "lsearch.hpp"
#include <algorithm>
#include <numeric>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include "utility.hpp"
#include "learn.hpp"

constexpr u32 MONTE_INITIAL_TIMES = 20;
constexpr u32 MONTE_MIN_TIMES = 100;
constexpr u32 MONTE_EXPAND_LIMIT = 500;
constexpr double MONTE_TIME_LIMIT = 10000;
constexpr u8 MONTE_MT_LIMIT = 25;
i16 current_eval = 0;

#define MOD_RANDOM(val) (val & 7)

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

static inline void put_dot()
{
        putchar('.');
        fflush(stdout);
}

static void expand_node_sub(std::vector<Node *>::iterator begin, std::vector<Node *>::iterator end, std::function<void(Node *)> apply_child)
{
        std::for_each(begin, end, apply_child);
}

void Montecarlo::expand_node(Node *node, std::function<void(Node *)> apply_child)
{
        node->expand();
        if(depth >= MONTE_MT_LIMIT){
                std::thread
                        th1(expand_node_sub, std::begin(node->ref_children()), std::begin(node->ref_children()) + (node->ref_children().size() >> 1), apply_child),
                        th2(expand_node_sub, std::begin(node->ref_children()) + (int)((node->ref_children().size() >> 1)), std::end(node->ref_children()), apply_child);
                th1.join();
                th2.join();
        }else
                std::for_each(std::begin(node->ref_children()), std::end(node->ref_children()), apply_child);
}

static void synchronized_adding(std::vector<PlayoutResult *> &dst, std::vector<PlayoutResult *> &data)
{
        std::mutex mtx;
        {
                std::lock_guard<std::mutex> lock(mtx);
                for(PlayoutResult *node : data)
                        dst.push_back(node);
        }
}

u64 Montecarlo::select_and_play(std::vector<PlayoutResult *> &result, PlayoutResult *target, u16 llim)
{
        u64 trying = 0;
        trying += playout_process(target, limit);

        if(target->trying >= MONTE_EXPAND_LIMIT){
                std::vector<PlayoutResult *> buf;
                buf.reserve(81);
                target->ucb = -1;
                expand_node(target->node, [&, this](Node *child){
                                PlayoutResult *tmp = new PlayoutResult(child, target);
                                trying += playout_process(tmp, llim);
                                buf.push_back(tmp);
                        });
                synchronized_adding(result, buf);
        }

        return trying;
}

static void add_trying(u64 *total, u64 val)
{
        std::mutex mtx;
        {
                std::lock_guard<std::mutex> lock(mtx);
                *total += val;
        }
}

std::vector<Node *> Montecarlo::listup_node_greedy(Node *node)
{
        std::vector<Node *> nodes;
        node->expand();
        for(Node *child : node->ref_children()){
                child->expand();
                for(Node *gcn : child->ref_children()){
                        gcn->evaluate();
                        nodes.push_back(gcn);
                }
        }
        
        std::sort(std::begin(nodes), std::end(nodes),
                  [](const Node *n1, const Node *n2){
#ifdef I_AM_ENEMY
                          return n1->get_score() < n2->get_score();
#endif
#ifdef I_AM_ME
                          return n1->get_score() > n2->get_score();
#endif
                  });
        return nodes;
}

std::vector<Node *> Montecarlo::listup_node_greedy2(Node *node, u8 rank)
{
        std::vector<Node *> nodes;
        std::vector<Node *> result;
        i64 local;
        node->expand();
        for(Node *child : node->ref_children()){
                child->expand();
                local = 0;
                nodes.push_back(child);
                for(Node *gcn : child->ref_children()){
                        local += gcn->evaluate();
                }
                child->set_score(local);
        }

        std::sort(std::begin(nodes), std::end(nodes),
                  [](const Node *n1, const Node *n2){
#ifdef I_AM_ENEMY
                          return n1->get_score() < n2->get_score();
#endif
#ifdef I_AM_ME
                          return n1->get_score() > n2->get_score();
#endif
          });

        i64 max_score = nodes.at(0)->get_score();
        for(Node *n : nodes){
                if(max_score != n->get_score()){
                        max_score = n->get_score();
                        if(!--rank)
                                break;
                }
                result.push_back(n);
        }

        return result;
}

const Node *Montecarlo::select_final(Node *node)
{
        std::vector<Node *> &&nodes = listup_node_greedy(node);
        return get_first_child(nodes.at(0));
}

const Node *Montecarlo::greedy(Node *node)
{
        return listup_node_greedy2(node, 1).at(0);
}

const Node *Montecarlo::greedy_montecarlo(Node *node, u8 depth)
{
        if(node->evaluate() < 0)
                current_eval = node->get_score() >> 1;
        std::vector<Node *> &&nodes = listup_node_greedy2(node, 2);
        if(nodes.size() == 1) return nodes.at(0);
        std::vector<PlayoutResult *> original, result;
        u64 total_trying = 0;
        this->depth = depth;
        this->limit = MONTE_MIN_TIMES;
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        u16 init_times = (50000 + ((i64)depth << 5)) / nodes.size();
        total_trying += 50000;

        for(Node *child : nodes){
                PlayoutResult *tmp = new PlayoutResult(child, nullptr);
                result.push_back(tmp);
                original.push_back(tmp);       
        }

        {
                ThreadPool tp(3, 100);
                for(PlayoutResult *p : result){
                        while(!tp.add(std::make_shared<initial_playout>(this, p, init_times))){
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                }

        }
//        exit(0);
        
        if(result.size() == 1) return get_first_child(result.at(0)->node);
        while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() <= MONTE_TIME_LIMIT){
                put_dot();
                std::for_each(std::begin(result), std::end(result),
                              [total_trying](PlayoutResult *p){ p->calc_ucb(total_trying);});
                // UCBでソート
                std::sort(std::begin(result), std::end(result),
                          [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->ucb > r2->ucb; });

                std::thread th1([&](){
                                        add_trying(&total_trying, select_and_play(result, result.at(0), MONTE_INITIAL_TIMES));
                                }),
                        th2([&](){
                                    add_trying(&total_trying, select_and_play(result, result.at(1), MONTE_INITIAL_TIMES));
                            });
                th2.join();
                th1.join();
        }
        putchar('\n');

        std::sort(std::begin(original), std::end(original),
                  [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->trying > r2->trying; });

        printf("***TOTAL TRYING***  ========>  %ld\n", total_trying);
        std::cout << (int)original.at(0)->trying << "trying" << std::endl;
        std::for_each(std::begin(original), std::end(original),
                      [](PlayoutResult *r){ r->draw(); });
        return original.at(0)->node;
}

const Node *Montecarlo::select_better_node(std::vector<PlayoutResult *> &sorted_children)
{
        u8 i = 0;
        std::vector<const Node *> same_pos;
        const PlayoutResult *top = sorted_children.at(i++);
        same_pos.push_back(top->node);

        for(; i < sorted_children.size();++i)
                if(top->node->has_same_pos(sorted_children.at(i)->node))
                        same_pos.push_back(sorted_children.at(i)->node);

        std::sort(std::begin(same_pos), std::end(same_pos),
                  [](const Node *n1, const Node *n2){
#ifdef I_AM_ENEMY
                          return n1->get_score() < n2->get_score();
#endif
#ifdef I_AM_ME
                          return n1->get_score() > n2->get_score();
#endif
                  });

        return get_first_child(same_pos.at(0));
}



/*
 * モンテカルロ法のアルゴリズム
 */
const Node *Montecarlo::let_me_monte(Node *node, u8 depth)
{
        std::vector<PlayoutResult *> original, result;
        u64 total_trying = 0;
        this->depth = depth;
        this->limit = MONTE_MIN_TIMES;
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        if(!depth)
                return select_final(node);

        // 一個下のノードを展開
        expand_node(node, [&result, &original, &total_trying, depth, this](Node *child){
                                  PlayoutResult *tmp = new PlayoutResult(child, nullptr);
                                  total_trying += playout_process(tmp, MONTE_INITIAL_TIMES);
                                  result.push_back(tmp);
                                  original.push_back(tmp); });

        // 各ノードに対してシュミレーションを行う        
        printf("thinking");
        while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() <= MONTE_TIME_LIMIT){
                //printf("%ldnodes\n", result.
                put_dot();
                std::for_each(std::begin(result), std::end(result),
                              [total_trying](PlayoutResult *p){ p->calc_ucb(total_trying);});

                // UCBでソート
                std::sort(std::begin(result), std::end(result),
                          [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->ucb > r2->ucb; });

                std::thread th1([&](){
                                        add_trying(&total_trying, select_and_play(result, result.at(0), MONTE_INITIAL_TIMES));
                                }),
                        th2([&](){
                                    add_trying(&total_trying, select_and_play(result, result.at(1), MONTE_INITIAL_TIMES));
                            });
                th2.join();
                th1.join();
        }
        putchar('\n');

        std::sort(std::begin(original), std::end(original),
                  [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->trying > r2->trying; });
        
#ifdef __DEBUG_MODE
        std::for_each(std::begin(original), std::end(original),
                      [](PlayoutResult *r){ std::cout << "Win:" << r->percentage() * 100 << "%" << std::endl; });
#endif
        // 一番いい勝率のやつを返す
        printf("***TOTAL TRYING***  ========>  %ld\n", total_trying);
        std::cout << (int)original.at(0)->trying << "trying" << std::endl;
        return original.at(0)->node;
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

u64 Montecarlo::playout_process(PlayoutResult *tmp, u16 limit)
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
                d1 = int_to_direction(MOD_RANDOM(random()));
        }while(
                (node->turn && !node->enemy_agent1.is_movable(node->field, d1)) ||
                (!node->turn && !node->my_agent1.is_movable(node->field, d1)));
        do{
                d2 = int_to_direction(MOD_RANDOM(random()));
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
    m1 = int_to_direction(MOD_RANDOM(random()));
  }while(!node->my_agent1.is_movable(node->field, m1) || !(node->check_panel_score(m1, node->my_agent1) >= 0));
  do{
    m2 = int_to_direction(MOD_RANDOM(random()));
  }while(!node->my_agent2.is_movable(node->field, m2) || !(node->check_panel_score(m2, node->my_agent2) >= 0));
  do{
    e1 = int_to_direction(MOD_RANDOM(random()));
  }while(!node->enemy_agent1.is_movable(node->field, e1) || !(node->check_panel_score(e1, node->enemy_agent1) >= 0));
  do{
    e2 = int_to_direction(MOD_RANDOM(random()));
  }while(!node->enemy_agent2.is_movable(node->field, e2) || !(node->check_panel_score(e2, node->enemy_agent2) >= 0));

  return check_direction_legality(node, {m1, m2, e1, e2});
}


std::array<Direction, 4> Montecarlo::get_learning_direction(Node *node)
{
        Direction m1, m2, e1, e2;

        std::vector<action> &&actions = node->generate_state_hash();

        try{
                m1 = learning_map.at(actions[0].state_hash)->random_select().direction;
		if(!node->my_agent1.is_movable(node->field, m1))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{
                        m1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent1.is_movable(node->field, m1));
        }
        
        try{
                m2 = learning_map.at(actions[1].state_hash)->random_select().direction;
		if(!node->my_agent2.is_movable(node->field, m2))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        m2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent2.is_movable(node->field, m2));
        }
        
        try{
                e1 = learning_map.at(actions[2].state_hash)->random_select().direction;
		if(!node->enemy_agent1.is_movable(node->field, e1))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        e1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent1.is_movable(node->field, e1));
        }
		
        try{
                e2 = learning_map.at(actions[3].state_hash)->random_select().direction;
		if(!node->enemy_agent2.is_movable(node->field, e2))
			  throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        e2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent2.is_movable(node->field, e2));
        }
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

i8 Montecarlo::my_random_half_play(Node *node)
{
        Direction d1, d2;

        std::vector<action> &&actions = node->generate_state_hash(MY_TURN);

        try{
                d1 = learning_map.at(actions[0].state_hash)->random_select().direction;
		if(!node->my_agent1.is_movable(node->field, d1))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{
                        d1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent1.is_movable(node->field, d1));
        }
        
        try{
                d2 = learning_map.at(actions[1].state_hash)->random_select().direction;
		if(!node->my_agent2.is_movable(node->field, d2))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        d2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent2.is_movable(node->field, d2));
        }

        if(node->my_agent1.check_conflict(d1, node->my_agent2, d2))
                return -1;
        
        return 0;
}

i8 Montecarlo::enemy_random_half_play(Node *node)
{
        Direction d1, d2;

        std::vector<action> &&actions = node->generate_state_hash(ENEMY_TURN);

        
        try{
                d1 = learning_map.at(actions[2].state_hash)->random_select().direction;
		if(!node->enemy_agent1.is_movable(node->field, d1))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        d1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent1.is_movable(node->field, d1));
        }
		
        try{
                d2 = learning_map.at(actions[3].state_hash)->random_select().direction;
		if(!node->enemy_agent2.is_movable(node->field, d2))
                        throw std::out_of_range("unko");
        }catch(const std::out_of_range &e){
                do{            
                        d2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent2.is_movable(node->field, d2));
        }

        if(node->enemy_agent1.check_conflict(d1, node->enemy_agent2, d2))
                return -1;
        
        return 0;
}

i8 Montecarlo::random_half_play(Node *node, u8 turn)
{
        if(IS_MYTURN(turn))
                return my_random_half_play(node);
        else
                return enemy_random_half_play(node);
}

Judge Montecarlo::faster_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Judge result;
        
        /*
         * ここで一回ランダムに片方が行う必要がある
         */
        if(random_half_play(current, node->turn) == -1)
                return LOSE;

        while(depth--){
                if(depth & 1)
                        current->play(find_random_legal_direction(current));
                else
                        current->play(get_learning_direction(current));
        }

        if((current->evaluate()) < 0){
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
