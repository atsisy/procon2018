#pragma once

#include "types.hpp"
#include <random>

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

        std::string dump_json();
        
        /*
         * 自分自身を評価するメソッド
         */
        i64 evaluate();
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
        void draw();

        void dump_json_file(const char *file_name);

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

        int slant(Agent agent, Field &field, i8 depth, Direction *result);

public:
        Node *absearch(Node *root);
        Direction slantsearch(Agent agent, Field & field, u8 depth);
};


enum Judge {
        LOSE = 0,
        WIN = 1,
        DRAW = 2,
};

struct PlayoutResult {

        double percentage;
        Node *node;

        PlayoutResult(double p, Node *n){ percentage = p, node = n; }
};

class Montecarlo {
private:
        std::mt19937 random;
        
        Judge playout(Node *node, u8 depth);
        Node *simulation(Node *node);
        
public:
        Node *let_me_monte(Node *node);
        Montecarlo();
};
