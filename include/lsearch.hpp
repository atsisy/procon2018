#pragma once

#include <random>
#include <array>
#include <memory>
#include "types.hpp"

using namespace util;

struct action {
        u64 state_hash;
        Direction act;

        action(u64 hash) { state_hash = hash; act = STOP; }
};

class Search;
class Montecarlo;

constexpr u8 MY_TURN = 0;
constexpr u8 ENEMY_TURN = !MY_TURN;
#define IS_MYTURN(turn) (!turn)

/*
 * 古典的探索法に見られるノードを表すクラス
 */
class Node {
        friend FieldBuilder;
        friend FieldEvaluater;
        friend Search;
        friend Montecarlo;

        friend std::array<Direction, 4> check_direction_legality_static(Node *node, std::array<Direction, 4> dirs);
private:
        // フィールド
        Field *field;
        // 評価値
        i16 score;
        u8 turn;

        const Node *parent;

        /*
         * 敵味方それぞれ2つずつのエージェント
         */
        Agent my_agent1;
        Agent my_agent2;
        Agent enemy_agent1;
        Agent enemy_agent2;

        std::array<Direction, 2> last_action;

        std::vector<Node *> children;

        /*
         * ルートノード生成用のコンストラクタ
         */
        Node(Field *field, Rect<i16> agent1, Rect<i16> agent2);

        /*
         * ターンによりノードの展開方法が異なるため、この２つのメソッドを使い分けよう！
         */
        void expand_enemy_node();
        void expand_my_node();

        void force_set_turn(u8 turn)
        {
                this->turn = turn;
        }

        void play(std::array<Direction, 4> dirs);
        void play_half(Direction d1, Direction d2, u8 turn);

        bool has_same_pos(const Node *node);

        std::string dump_json() const;
        std::vector<action> __generate_state_hash(std::vector<Agent> agents) const;

        void first_step_enemy(Direction dir1, Direction dir2);
        void first_step_mine(Direction dir1, Direction dir2);

        std::vector<Node *> expand_my_specific_children(std::vector<Direction> &for_a1, std::vector<Direction> &for_a2);
        std::vector<Node *> expand_enemy_specific_children(std::vector<Direction> &for_a1, std::vector<Direction> &for_a2);
        Direction __find_greedy(Agent agent, u32 rand);
        std::pair<Direction, Direction> find_greedy(u8 turn);

        void douji_expand();
public:
        /*
         * 普通に使ってほしいコンストラクタ
         * 基本的には引数parentのクローン
         */
        Node(const Node *parent);

        /*
         * JSONからNodeを復元する
         */
        Node(const char *json_path);

        /*
         * デバッグ用のメソッド
         * 情報を吐くよ！！
         */
        void draw() const;

        void dump_json_file(const char *file_name) const;

        i8 check_panel_score(Direction d, Agent agent);


        /*
         * 自分自身を評価するメソッド
         */
        i16 evaluate();

        void put_score_info();

        /*
         * 展開するやつ
         */
        void expand();

        Node *get_specific_child(Direction agent1, Direction agent2);
        std::vector<Node *> expand_specific_children(std::vector<Direction> &&for_a1, std::vector<Direction> &&for_a2);

        void set_score(i16 score)
        {
                this->score = score;
        }

        i16 get_score() const
        {
                return score;
        }

        u8 toggled_turn() const
        {
                return !turn;
        }

        std::vector<Node *> &ref_children()
        {
                return children;
        }

        std::vector<action> generate_state_hash(u8 turn) const;
        std::vector<action> generate_state_hash() const;

        bool nobody(i8 x, i8 y) const;
        std::array<Direction, 4> agent_diff(const Node *node) const;

        Direction get_last_action(u8 index) const
        {
                return last_action[index];
        }
};

class Search {
private:
        i64 ab_max(Node *node, u8 depth, i16 a, i16 b);
        i64 ab_min(Node *node, u8 depth, i16 a, i16 b);

        i8 slant(Agent agent, Field &field, u8 depth, Direction *result);

public:
        Node *absearch(Node *root);
        i8 slantsearch(Agent agent, Field & field);
};


enum Judge {
        LOSE = 0,
        WIN = 1,
        DRAW = 2,
};

constexpr float UCB_C = std::sqrt(2);
struct PlayoutResult {

        Node *node;
        PlayoutResult *parent;
        u32 trying;
        u32 win;
        float ucb;

        PlayoutResult(Node *node, PlayoutResult *p)
        {
                this->node = node;
                this->parent = p;
                trying = 0;
                win = 0;
        }

        void update(u16 trying, u16 win)
        {
                this->trying += trying;
                this->win += win;
                if(parent != nullptr)
                        parent->update(trying, win);
        }

        double percentage()
        {
                return (double)this->win / (double)this->trying;
        }

        /*
          wi は i 番目の手での勝利数
          ni は i 番目の手での試行回数
          t は全ての手での試行回数
          c は「勝率が良い手」と「試行回数が不十分な手」のどちらを優先するかの度合いを決めるパラメータ
         */
        float calc_ucb(u32 global_total_trying)
        {
                if(ucb == -1)
                        return -1;
                return (ucb = ((float)this->win / (float)this->trying)
                        + (UCB_C * std::sqrt(std::log((float)global_total_trying) / (float)this->trying)));
        }

        /*
         * N : そのノードでのplayoutの回数
         Pi : ノードのなかのi番目の指し手のプレイアウト時の勝率
         Si : ノードのなかのi番目の指し手のプレイアウトの回数
         */
        float calc_ucb_tuned(u32 global_total)
        {
                if(ucb == -1)
                        return -1;
                const float Pi = (float)this->win / (float)this->trying;
                ucb = (Pi +
                       (UCB_C * std::sqrt((std::log(global_total) /
                                   ((float)this->trying)) *
                                  std::min(0.25, Pi - std::pow(Pi, 2)
                                           + std::sqrt((2.0 * std::log(global_total)) / (float)this->trying)))));
                 //Pi + sqrt( log N / Si min ( 1/4 , Pi - Pi**2 + sqrt( (2logN)/Si) );
                return ucb;
        }

        void draw()
        {
                std::cout << "\tWin: " << percentage() * 100 << "%"
                          << "\tUCB: " << ucb
                          << "\ttrying: " << trying << std::endl;
        }
};

struct db_element {
        u64 hash;
        Direction dir;

        db_element(u64 hash, Direction dir)
        {
                this->hash = hash;
                this->dir = dir;
        }
};

struct LocalPlayoutResult {

        u16 times;
        u16 win;

        LocalPlayoutResult(u16 times, u16 win){ this->times = times; this->win = win; }
};

class initial_playout;

constexpr u8 MONTE_DEPTH = 70;
class Montecarlo {

        friend initial_playout;

private:
        util::xor128 random;
        u8 depth;
        u32 limit;
        float ucb_c;
        std::vector<db_element> buffered_data;

        void update_ucb_c()
                {
                        static u64 count;
                        ucb_c = UCB_C * std::pow(0.995, ++count);
                }

        const Node *get_first_child(const Node *node);
        u64 playout_process(PlayoutResult *tmp, u16 limit);
        u64 dbbuild_playout_process(PlayoutResult *tmp, u16 limit);
        Judge playout(Node *node, u8 depth);
        Judge faster_playout(Node *node, u8 depth);
        Judge dbbuild_playout(Node *node, u8 depth);
        std::array<Direction, 4> find_random_legal_direction(Node *node);
        std::array<Direction, 4> get_learning_direction(Node *node);
        std::array<Direction, 4> check_direction_legality(Node *node, std::array<Direction, 4> dirs);
        i8 enemy_random_half_play(Node *node);
        i8 my_random_half_play(Node *node);
        i8 random_half_play(Node *node, u8 turn);
        void expand_node(Node *node, std::function<void(Node *)> apply_child);
        Node *simulation(Node *node);
        u64 select_and_play(std::vector<PlayoutResult *> &result, PlayoutResult *target, u16 llim);
        u64 dbbuild_select_and_play(std::vector<PlayoutResult *> &result, PlayoutResult *target, u16 llim);
        const Node *select_final(Node *node);
        const Node *select_better_node(std::vector<PlayoutResult *> &sorted_children);
        std::vector<Node *> listup_node_greedy(Node *node, u8 rank);
        std::vector<Node *> listup_node_greedy2(Node *node, u8 rank);
        std::vector<Node *> listup_node_greedy_turn(Node *node, u8 rank, u8 turn);
        Node *random_greedy(Node *node, u8 rank);
        void expand_not_ai_turn(Node *node, std::function<void(Node *)> apply_child);
        Direction learning_or_random(Node *node, Agent &agent, u64 hash);

        bool isbadPlayoutResult(std::vector<PlayoutResult *> pr);

        void go_learning(Node *node, u8 turn);
        void buffering_learning_data(Node *node, u8 turn);
        void __buffering_learning_data(Node *before, Node *after);
        void __buffering_learning_data(Node *node);

public:
        void create_database(Node *node, i64 timelimit, u8 depth);
        const Node *let_me_monte(Node *node, u8 depth);
        const Node *greedy(Node *node);
        const Node *greedy_montecarlo(Node *node, u8 depth);
        const Node *random_play(Node *node);

        void write_out_data_base(const char *file);

        Montecarlo();
};

class initial_playout : public Runnable {

private:
        PlayoutResult *pr;
        u16 times;
        Montecarlo *monte;

public:
        void run()
        {
                monte->playout_process(pr, times);
        }

        initial_playout(Montecarlo *m, PlayoutResult *p, u16 t)
        {
                pr = p;
                times = t;
                monte = m;
        }
};


void write_learning_data(const Node *before, const Node *after);
