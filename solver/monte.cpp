#include "lsearch.hpp"
#include <algorithm>
#include <numeric>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include "utility.hpp"
#include "learn.hpp"

constexpr u32 MONTE_INITIAL_TIMES = 2;
constexpr u32 MONTE_MIN_TIMES = 2;
constexpr u32 MONTE_EXPAND_LIMIT = 700;
constexpr double MONTE_TIME_LIMIT = 12000;
constexpr u8 MONTE_MT_LIMIT = 25;
constexpr double MONTE_PROGRESSIVE_BIAS = 15;
i16 current_eval = 0;

#define MOD_RANDOM(val) (val & 7)

Montecarlo::Montecarlo()
{
        ucb_c = UCB_C;
}

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

Direction Montecarlo::learning_or_random(Node *node, Agent &agent, u64 hash)
{
        Direction d;
        try{
                d = learning_map.at(hash)->random_select(random()).direction;
		if(!agent.is_movable(node->field, d))
                        throw std::out_of_range("out_of_range was occured");
        }catch(const std::out_of_range &e){
                do{
                        d = int_to_direction(MOD_RANDOM(random()));
                }while(!agent.is_movable(node->field, d));
        }

        return d;
}

static void expand_node_sub(std::vector<Node *>::iterator begin, std::vector<Node *>::iterator end, std::function<void(Node *)> apply_child)
{
        std::for_each(begin, end, apply_child);
}

void Montecarlo::expand_node(Node *node, std::function<void(Node *)> apply_child)
{
        node->expand();
        /*マルチスレッドは一旦やめる
        if(depth >= MONTE_MT_LIMIT){
                std::thread
                        th1(expand_node_sub, std::begin(node->ref_children()), std::begin(node->ref_children()) + (node->ref_children().size() >> 1), apply_child),
                        th2(expand_node_sub, std::begin(node->ref_children()) + (int)((node->ref_children().size() >> 1)), std::end(node->ref_children()), apply_child);
                th1.join();
                th2.join();
        }else
        */
        std::for_each(std::begin(node->ref_children()), std::end(node->ref_children()), apply_child);
}

void Montecarlo::expand_not_ai_turn(Node *node, std::function<void(Node *)> apply_child)
{
        node->douji_expand();
        for(Node *child : node->ref_children()){
                apply_child(child);
        }
        /*
        auto &&good_nodes = listup_node_greedy_turn(node, 2, MY_TURN);
        for(Node *child : good_nodes){
                //child->expand();
                auto &&vec = listup_node_greedy(child, 3);
                std::for_each(std::begin(vec), std::end(vec), apply_child);
        }
        */
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
        std::mutex mtx;
        trying += playout_process(target, limit);

        if(target->trying >= MONTE_EXPAND_LIMIT){
                std::vector<PlayoutResult *> buf;
                buf.reserve(81);
                target->ucb = -1;
                /*
                  expand_node(target->node, [&, this](Node *child){
                  PlayoutResult *tmp = new PlayoutResult(child, target);
                  trying += playout_process(tmp, llim);
                  buf.push_back(tmp);
                  });
                */
                expand_not_ai_turn(target->node, [&, this](Node *child){
                                PlayoutResult *tmp = new PlayoutResult(child, target);
                                playout_process(tmp, llim);
                                {
                                        std::lock_guard<std::mutex> lock(mtx);
                                        trying += llim;
                                        buf.push_back(tmp);
                                }
                        });

                synchronized_adding(result, buf);
        }

        return trying;
}

u64 Montecarlo::dbbuild_select_and_play(std::vector<PlayoutResult *> &result, PlayoutResult *target, u16 llim)
{
        u64 trying = 0;
        trying += dbbuild_playout_process(target, limit);

        if(target->trying >= MONTE_EXPAND_LIMIT){
                std::vector<PlayoutResult *> buf;
                buf.reserve(81);
                target->ucb = -1;
                expand_not_ai_turn(target->node, [&, this](Node *child){
                                PlayoutResult *tmp = new PlayoutResult(child, target);
                                dbbuild_playout_process(tmp, llim);
                                {
                                        trying += llim;
                                        buf.push_back(tmp);
                                }
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

std::vector<Node *> Montecarlo::listup_node_greedy(Node *node, u8 rank)
{
        std::vector<Node *> nodes, result;
        node->expand();

        for(Node *n : node->ref_children()){
                n->evaluate();
                nodes.push_back(n);
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

        bool del_flag = false;
        i64 max_score = nodes.at(0)->get_score();
        for(Node *n : nodes){
                if(del_flag){
                        delete n;
                        continue;
                }
                if(max_score != n->get_score()){
                        max_score = n->get_score();
                        if(!--rank){
                                del_flag = true;
                                delete n;
                                continue;
                        }
                }
                result.push_back(n);
        }

        return result;
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

        bool del_flag = false;
        i64 max_score = nodes.at(0)->get_score();
        for(Node *n : nodes){
                if(del_flag){
                        delete n;
                        continue;
                }

                if(max_score != n->get_score()){
                        max_score = n->get_score();
                        if(!--rank){
                                del_flag = true;
                                delete n;
                                continue;
                        }
                }
                result.push_back(n);
        }

        return result;
}

Node *Montecarlo::random_greedy(Node *node, u8 rank)
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

        bool del_flag = false;
        i64 max_score = nodes.at(0)->get_score();
        for(Node *n : nodes){
                if(del_flag){
                        delete n;
                        continue;
                }

                if(max_score != n->get_score()){
                        max_score = n->get_score();
                        if(!--rank){
                                del_flag = true;
                                delete n;
                                continue;
                        }
                }
                result.push_back(n);
        }

        xor128 rand;
        return result.at(rand() % result.size());
}

std::vector<Node *> Montecarlo::listup_node_greedy_turn(Node *node, u8 rank, u8 turn)
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

        if(IS_MYTURN(turn)){
                std::sort(std::begin(nodes), std::end(nodes),
                          [](const Node *n1, const Node *n2){
                                  return n1->get_score() > n2->get_score();
                          });
        }else{
                std::sort(std::begin(nodes), std::end(nodes),
                          [](const Node *n1, const Node *n2){
                                  return n1->get_score() < n2->get_score();
                          });
        }

        bool del_flag = false;
        i64 max_score = nodes.at(0)->get_score();
        for(Node *n : nodes){
                if(del_flag){
                        delete n;
                        continue;
                }

                if(max_score != n->get_score()){
                        max_score = n->get_score();
                        if(!--rank){
                                del_flag = true;
                                delete n;
                                continue;
                        }
                }
                result.push_back(n);
        }

        return result;
}

const Node *Montecarlo::random_play(Node *node)
{
        Node *working = new Node(node);
        random_half_play(working, node->turn);
        return working;
}


const Node *Montecarlo::select_final(Node *node)
{
        std::vector<Node *> &&nodes = listup_node_greedy(node, 4);
        return get_first_child(nodes.at(0));
}

const Node *Montecarlo::greedy(Node *node)
{
        Node *ret = random_greedy(node, 1);
        ret->evaluate();
        return ret;
}

const Node *Montecarlo::greedy_montecarlo(Node *node, u8 depth)
{
        if(node->evaluate() < 0)
                current_eval = node->get_score() >> 1;
        std::vector<Node *> &&nodes = listup_node_greedy2(node, 3);
        if(nodes.size() == 1) return nodes.at(0);
        std::vector<PlayoutResult *> original, result;
        u64 total_trying = 0;
        this->depth = depth;
        this->limit = MONTE_MIN_TIMES;
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        u16 init_times = (30000 + ((i64)depth << 5)) / nodes.size();
        total_trying += 30000;

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
        original.at(0)->node->evaluate();
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


u64 learning_found;
u64 learning_not_found;


bool Montecarlo::isbadPlayoutResult(std::vector<PlayoutResult *> pr) {
        float minu = 0;
        float sum, variance, mean, precision;
        std::vector<float> ucbList;

        for(PlayoutResult *prin: pr) {
                ucbList.push_back(prin->percentage() * 100.0);
        }

        // ucbListの平均
        sum = 0;
        for(float ucb: ucbList) {
                sum += ucb;
        }
        mean = sum/(float)ucbList.size();    // 誤差対策

        // 分散
        sum = 0;
        for(float ucb: ucbList) {
                sum += pow(ucb-mean, 2);
        }
        variance = sum/(float)ucbList.size();
        precision = 1.0/variance;
        std::cout << "** UCB **" << std::endl;
        std::cout << "mean = " << mean << std::endl;
        std::cout << "variance = " << variance << std::endl;
        std::cout << "precision = " << precision << std::endl;


/*        std::sort(std::begin(ucbList), std::end(ucbList), std::greater<float>());

        for(int i=0; i<5; i++) {
                minu += std::abs(ucbList[i]-ucbList[i+1]);
        }*/
        return (precision > 20) ? true : false;
}

/*
 * モンテカルロ法のアルゴリズム
 */
const Node *Montecarlo::let_me_monte(Node *node, u8 depth)
{
        std::vector<PlayoutResult *> original, result;
        u64 total_trying = 0;
        u64 counter = 0, index = 0;
        this->depth = depth;
        this->limit = MONTE_MIN_TIMES;
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        learning_not_found = learning_found = 0;

        if(!depth)
                return select_final(node);

        Node *nodesave(node);
        // 一個下のノードを展開
        node->douji_expand();
/*
        for(Node *child : node->ref_children()){
                child->dump_json_file("debug.json");
                child->draw();
                getchar();
        }
*/

        // progressib widening
        for(Node *child : node->ref_children())
                child->evaluate();
        for(Node *child : nodesave->ref_children()) child->evaluate();  // greedy用
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()),
                  [](const Node *n1, const Node *n2){
#ifdef I_AM_ENEMY
                          return n1->get_score() < n2->get_score();
#endif
#ifdef I_AM_ME
                          return n1->get_score() > n2->get_score();
#endif
                  });
        for(Node *child : node->ref_children()){
                PlayoutResult *tmp = new PlayoutResult(child, nullptr);
                original.push_back(tmp);
        }

        {
                ThreadPool tp(2, 100);
                for(PlayoutResult *p : original){
                        while(!tp.add(std::make_shared<initial_playout>(this, p, 70))){
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                }
        }

        total_trying += 70 * original.size();

        puts("stage1 finished");
        result.push_back(original.at(index++));

        // 各ノードに対してシュミレーションを行う
        printf("thinking");
        double cntlim = 0;
        cntlim += (MONTE_PROGRESSIVE_BIAS * std::pow(1.01, index));
        while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() <= MONTE_TIME_LIMIT){
                counter++;
                //update_ucb_c();
                if(counter == (u64)cntlim){
                        if(original.size() <= index){
                                continue;
                        }
                        result.push_back(original.at(index++));
                        cntlim += (MONTE_PROGRESSIVE_BIAS * std::pow(1.01, index));
//                        std::cout << "result_size = " << result.size() << std::endl;
//                        std::cout << "cntlim = " << cntlim << std::endl;
                        counter = 0;
                }
                //printf("%ldnodes\n", result.
                //put_dot();
                std::for_each(std::begin(result), std::end(result),
                              [total_trying](PlayoutResult *p){ p->calc_ucb(total_trying);});

                // UCBでソート
                std::sort(std::begin(result), std::end(result),
                          [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->ucb > r2->ucb; });

                //result.at(0)->draw();
                //std::thread th1([&](){
                                        add_trying(&total_trying, select_and_play(result, result.at(0), 5));
                                        //});
                /*
                        th2([&](){
                                    add_trying(&total_trying, select_and_play(result, result.at(1), MONTE_INITIAL_TIMES));
                            });
                th2.join();
                */
                //th1.join();
        }
        putchar('\n');

        std::sort(std::begin(original), std::end(original),
                  [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->trying > r2->trying; });

        std::cout << "missing_rate: " << (double)learning_not_found / (double)(learning_found + learning_not_found) << std::endl;

#ifdef __DEBUG_MODE
        std::for_each(std::begin(original), std::end(original),
                      [](PlayoutResult *r){ r->draw(); });
#endif
        // 各ノードのUCBの値を見て貪欲法に切り替える
        if(isbadPlayoutResult(original)) {
                // greedy_original
                std::cout << "Greeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeedy" << std::endl;
                std::sort(std::begin(nodesave->ref_children()), std::end(nodesave->ref_children()),
                        [](const Node *n1, const Node *n2){
        #ifdef I_AM_ENEMY
                                return n1->get_score() < n2->get_score();
        #endif
        #ifdef I_AM_ME
                                return n1->get_score() > n2->get_score();
        #endif
                        });
                nodesave->ref_children().at(0)->evaluate();
                return nodesave->ref_children().at(0);
        }

        std::for_each(std::begin(original) + 1, std::end(original),
                      [](PlayoutResult *r){ delete r->node; delete r; });

        // 一番いい勝率のやつを返す
        printf("***TOTAL TRYING***  ========>  %ld\n", total_trying);
        original.at(0)->node->evaluate();
        return original.at(0)->node;
}

/*
 * データベース構築プログラム
 */
void Montecarlo::create_database(Node *node, i64 timelimit, u8 depth)
{
        std::vector<PlayoutResult *> original, result;
        u64 total_trying = 0;
        u64 counter = 0, index = 0;
        this->depth = depth;
        this->limit = MONTE_MIN_TIMES;
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

        learning_not_found = learning_found = 0;

        node->expand();

        for(Node *child : node->ref_children())
                child->evaluate();
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()),
                  [](const Node *n1, const Node *n2){
#ifdef I_AM_ENEMY
                          return n1->get_score() < n2->get_score();
#endif
#ifdef I_AM_ME
                          return n1->get_score() > n2->get_score();
#endif
                  });
        for(Node *child : node->ref_children()){
                PlayoutResult *tmp = new PlayoutResult(child, nullptr);
                original.push_back(tmp);
        }

        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Database building is started." << std::endl;

        std::cout << "Entering stage1..." << std::endl;

        for(PlayoutResult *p : original){
                put_dot();
                dbbuild_playout_process(p, 700);
        }
        printf("\n");

        total_trying += 600 * original.size();

        std::cout << "Stage1 was finished." << std::endl;
        std::cout << "Entering stage2..." << std::endl;

        result.push_back(original.at(index++));

        while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() <= timelimit){
                counter++;
                if(counter == 20){
                        if(original.size() <= index){
                                continue;
                        }
                        result.push_back(original.at(index++));
                        counter = 0;
                }
                std::for_each(std::begin(result), std::end(result),
                              [total_trying](PlayoutResult *p){ p->calc_ucb(total_trying);});

                std::sort(std::begin(result), std::end(result),
                          [](const PlayoutResult *r1, const PlayoutResult *r2){ return r1->ucb > r2->ucb; });
                total_trying += dbbuild_select_and_play(result, result.at(0), 5);
        }

        std::cout << "Stage2 was finished." << std::endl;

        std::cout << "Size of database will be " << buffered_data.size() << "." << std::endl;
        std::cout << "------------------------------------------" << std::endl;
}

u64 Montecarlo::dbbuild_playout_process(PlayoutResult *tmp, u16 limit)
{
        u16 win, i;

        win = 0;
        for(i = 0;i < limit;i++)
                if(dbbuild_playout(tmp->node, depth) == WIN)
                        win++;
        tmp->update(i, win);
        return i;
}

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

void Montecarlo::go_learning(Node *node, u8 turn)
{
        Node *clone = new Node(node);
        clone->force_set_turn(turn);
        Node *ret = random_greedy(clone, 1);
        write_learning_data(clone, ret);
        delete clone;
        delete ret;
}

void Montecarlo::buffering_learning_data(Node *node, u8 turn)
{
        Node *clone = new Node(node);
        /*
        clone->force_set_turn(turn);
        Node *ret = random_greedy(clone, 1);
        __buffering_learning_data(clone, ret);
        */
        __buffering_learning_data(clone);
        delete clone;
        //delete ret;
}

void Montecarlo::write_out_data_base(const char *file)
{
        std::ofstream ofs(file, std::ios::binary);
        if(!ofs){
                std::cerr << "Failed to open file." << std::endl;
        }

        u64 size = buffered_data.size();
        ofs.write((char *)&size, sizeof(u64));
        for(auto &elem : buffered_data){
                ofs.write((char *)&elem, sizeof(db_element));
                //ofs << elem.hash << "\t" << (int)elem.dir << std::endl;
        }
}

void Montecarlo::__buffering_learning_data(Node *before, Node *after)
{
#ifdef I_AM_ME
        std::vector<action> &&states = before->generate_state_hash(MY_TURN);
#endif
#ifdef I_AM_ENEMY
        std::vector<action> &&states = before->generate_state_hash(ENEMY_TURN);
#endif
        Direction d1 = after->get_last_action(0);
        Direction d2 = after->get_last_action(1);

        buffered_data.emplace_back(states.at(0).state_hash, d1);
        buffered_data.emplace_back(states.at(1).state_hash, d2);
}

void Montecarlo::__buffering_learning_data(Node *node)
{
        std::vector<action> &&states = node->generate_state_hash();
        auto [d1, d2] = node->find_greedy(MY_TURN);
        auto [d3, d4] = node->find_greedy(ENEMY_TURN);
        buffered_data.push_back(db_element(states.at(0).state_hash, d1));
        buffered_data.push_back(db_element(states.at(1).state_hash, d2));
        buffered_data.push_back(db_element(states.at(2).state_hash, d3));
        buffered_data.push_back(db_element(states.at(3).state_hash, d4));
}

std::array<Direction, 4> Montecarlo::get_learning_direction(Node *node)
{
        Direction m1, m2, e1, e2;

        std::vector<action> &&actions = node->generate_state_hash();

        try{
                m1 = learning_map.at(actions[0].state_hash)->random_select(random()).direction;
		if(!node->my_agent1.is_movable(node->field, m1))
                        throw -1;
                learning_found++;
        }catch(const std::out_of_range &e){
                learning_not_found++;
                //buffering_learning_data(node, MY_TURN);
                do{
                        m1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent1.is_movable(node->field, m1));
        }catch(...){
                do{
                        m1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent1.is_movable(node->field, m1));
        }

        try{
                m2 = learning_map.at(actions[1].state_hash)->random_select(random()).direction;
		if(!node->my_agent2.is_movable(node->field, m2))
                        throw 0;
                learning_found++;
        }catch(const std::out_of_range &e){
                learning_not_found++;
                //buffering_learning_data(node, MY_TURN);
                do{
                        m2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent2.is_movable(node->field, m2));
        }catch(...){
                do{
                        m2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent2.is_movable(node->field, m2));
        }

        try{
                e1 = learning_map.at(actions[2].state_hash)->random_select(random()).direction;
		if(!node->enemy_agent1.is_movable(node->field, e1))
                        throw 0;
                learning_found++;
        }catch(const std::out_of_range &e){
                learning_not_found++;
                //buffering_learning_data(node, ENEMY_TURN);
                do{
                        e1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent1.is_movable(node->field, e1));
        }catch(...){
                do{
                        e1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent1.is_movable(node->field, e1));
        }

        try{
                e2 = learning_map.at(actions[3].state_hash)->random_select(random()).direction;
		if(!node->enemy_agent2.is_movable(node->field, e2))
                        throw 0;
                learning_found++;
        }catch(const std::out_of_range &e){
                learning_not_found++;
                //buffering_learning_data(node, ENEMY_TURN);
                do{
                        e2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent2.is_movable(node->field, e2));
        }catch(...){
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
                d1 = learning_map.at(actions[0].state_hash)->random_select(random()).direction;
		if(!node->my_agent1.is_movable(node->field, d1))
                        throw std::out_of_range("out_of_range was occured");
        }catch(const std::out_of_range &e){
                do{
                        d1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent1.is_movable(node->field, d1));
        }

        try{
                d2 = learning_map.at(actions[1].state_hash)->random_select(random()).direction;
		if(!node->my_agent2.is_movable(node->field, d2))
                        throw std::out_of_range("out_of_range was occured");
        }catch(const std::out_of_range &e){
                do{
                        d2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->my_agent2.is_movable(node->field, d2));
        }

        if(node->my_agent1.check_conflict(d1, node->my_agent2, d2))
                return -1;
        if(node->my_agent1.check_conflict(d1, node->enemy_agent1, STOP) && node->my_agent1.check_conflict(d2, node->enemy_agent2, STOP))
                return -1;
        if(node->my_agent2.check_conflict(d1, node->enemy_agent1, STOP) && node->my_agent2.check_conflict(d2, node->enemy_agent2, STOP))
                return -1;


        node->play_half(d1, d2, MY_TURN);

        return 0;
}

i8 Montecarlo::enemy_random_half_play(Node *node)
{
        Direction d1, d2;

        std::vector<action> &&actions = node->generate_state_hash(ENEMY_TURN);

        try{
                d1 = learning_map.at(actions[0].state_hash)->random_select(random()).direction;
		if(!node->enemy_agent1.is_movable(node->field, d1))
                        throw std::out_of_range("out_of_range was occured");
        }catch(const std::out_of_range &e){
                do{
                        d1 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent1.is_movable(node->field, d1));
        }

        try{
                d2 = learning_map.at(actions[1].state_hash)->random_select(random()).direction;
		if(!node->enemy_agent2.is_movable(node->field, d2))
                        throw std::out_of_range("out_of_range was occured");
        }catch(const std::out_of_range &e){
                do{
                        d2 = int_to_direction(MOD_RANDOM(random()));
                }while(!node->enemy_agent2.is_movable(node->field, d2));
        }

        if(node->enemy_agent1.check_conflict(d1, node->enemy_agent2, d2))
                return -1;
        if(node->enemy_agent1.check_conflict(d1, node->my_agent1, STOP) && node->enemy_agent1.check_conflict(d2, node->my_agent2, STOP))
                return -1;
        if(node->enemy_agent2.check_conflict(d1, node->my_agent1, STOP) && node->enemy_agent2.check_conflict(d2, node->my_agent2, STOP))
                return -1;

        node->play_half(d1, d2, ENEMY_TURN);

        return 0;
}

i8 Montecarlo::random_half_play(Node *node, u8 turn)
{
        if(IS_MYTURN(turn))
                return my_random_half_play(node);
        else
                return enemy_random_half_play(node);
}

Judge Montecarlo::dbbuild_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Judge result;

        /*
         * ここで一回ランダムに片方が行う必要がある
         */
        if(random_half_play(current, node->turn) == -1)
                return LOSE;
        while(depth--){
                buffering_learning_data(current, ENEMY_TURN);
                if(learning_map.size() == 0 || depth & 7)
                        current->play(find_random_legal_direction(current));
                else
                        current->play(get_learning_direction(current));
        }

        current->evaluate();
        if((current->get_score()) < 0){
#ifdef I_AM_ENEMY
                result = WIN;
#endif
#ifdef I_AM_ME
                result = LOSE;
#endif
        }else if(current->get_score() > 0){
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

Judge Montecarlo::faster_playout(Node *node, u8 depth)
{
        Node *current = new Node(node);
        Judge result;

        /*
         * ここで一回ランダムに片方が行う必要がある
         */
        /*
        if(random_half_play(current, node->turn) == -1)
                return LOSE;
        */
        /*
        std::cout << "not found case: " << learning_not_found << std::endl;
        current->dump_json_file("debug.json");
        getchar();
        */
        while(depth--){
                if(depth & 1)
                        current->play(get_learning_direction(current));
                else
                        current->play(find_random_legal_direction(current));
        }

        current->evaluate();

        if(current->get_score() < 0){
#ifdef I_AM_ENEMY
                result = WIN;
#endif
#ifdef I_AM_ME
                result = LOSE;
#endif
        }else if(current->get_score() > 0){
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
