#pragma once

#include "types.hpp"
#include "utility.hpp"
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
        i16 evaluate();

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

constexpr float UCB_C = 0.3;
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
};

struct LocalPlayoutResult {

        u16 times;
        u16 win;
        
        LocalPlayoutResult(u16 times, u16 win){ this->times = times; this->win = win; }
};

class Montecarlo {
private:
        util::xor128 random;
        u8 depth;
        u32 limit;

        const Node *get_first_child(const Node *node);
        u64 playout_process(PlayoutResult *child, u16 limit);
        void apply_playout_to_data(std::vector<PlayoutResult> &data, int limit);
        Judge playout(Node *node, u8 depth);
        Judge faster_playout(Node *node, u8 depth);
        std::array<Direction, 4> find_random_legal_direction(Node *node);
        std::array<Direction, 4> check_direction_legality(Node *node, std::array<Direction, 4> dirs);
        i8 random_half_play(Node *node, u8 turn);
        void expand_node(Node *node, std::function<void(Node *)> apply_child);
        Node *simulation(Node *node);
        u64 select_and_play(std::vector<PlayoutResult *> &result, PlayoutResult *target);
        
public:
        const Node *let_me_monte(Node *node, u8 depth);
        Montecarlo();
};
