#pragma once

#include "types.hpp"
#include <random>
#include <array>

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
private:
        // フィールド
        Field *field;
        // 評価値
        i64 score;
        u8 turn;

        const Node *parent;

        /*
         * 敵味方それぞれ2つずつのエージェント
         */
        Agent my_agent1;
        Agent my_agent2;
        Agent enemy_agent1;
        Agent enemy_agent2;

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

        void play(std::array<Direction, 4> dirs);
        void play_half(Direction d1, Direction d2, u8 turn);
        
        std::string dump_json() const;
        
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

        
        /*
         * 自分自身を評価するメソッド
         */
        i64 evaluate();

        void put_score_info();
        
        /*
         * 展開するやつ
         */
        void expand();

        Node *get_specific_child(Direction agent1, Direction agent2);
        
        void set_score(i16 score)
        {
                this->score = score;
        }

        i64 get_score() const
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

constexpr float UCB_C = 0.5;
struct PlayoutResult {

        double percentage;
        Node *node;
        u16 trying;
        u16 win;
        float ucb;

        PlayoutResult(Node *node)
        {
                this->node = node;
                percentage = 0;
                trying = 0;
                win = 0;
        }
        
        void update(u16 trying, u16 win)
        {
                this->trying += trying;
                this->win += win;
                this->percentage = (double)this->win / (double)this->trying;
        }

        /*
          wi は i 番目の手での勝利数
          ni は i 番目の手での試行回数
          t は全ての手での試行回数
          c は「勝率が良い手」と「試行回数が不十分な手」のどちらを優先するかの度合いを決めるパラメータ
         */
        float calc_ucb(u32 global_total_trying, u32 local_total_trying)
        {
                return (ucb = (win / local_total_trying)
                                + (UCB_C * std::sqrt(std::log(global_total_trying) / local_total_trying)));
        }
};

struct LocalPlayoutResult {

        u16 times;
        u16 win;

        LocalPlayoutResult(u16 times, u16 win){ this->times = times; this->win = win; }
};

class Montecarlo {
private:
        std::mt19937 random;

        const Node *get_first_child(const Node *node);
        LocalPlayoutResult playout_process(Node *child, u16 limit);
        void apply_playout_to_data(std::vector<PlayoutResult> &data, int limit);
        Judge playout(Node *node, u8 depth);
        Judge faster_playout(Node *node, u8 depth);
        std::array<Direction, 4> find_random_legal_direction(Node *node);
        std::array<Direction, 4> check_direction_legality(Node *node, std::array<Direction, 4> dirs);
        void random_half_play(Node *node, u8 turn);
        
        Node *simulation(Node *node);
        
public:
        const Node *let_me_monte(Node *node);
        Montecarlo();
};
