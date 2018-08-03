#pragma once

#include "types.hpp"

class Search;

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

        /*
         * ルートノード生成用のコンストラクタ
         */
        Node(Field *field, Rect<i16> agent1, Rect<i16> agent2);

        /*
         * ターンによりノードの展開方法が異なるため、この２つのメソッドを使い分けよう！
         */
        std::vector<Node *> expand_enemy_node() const;
        std::vector<Node *> expand_my_node() const;

        /*
         * 自分自身を評価するメソッド
         */
        Node *evaluate();
public:
        /*
         * 普通に使ってほしいコンストラクタ
         * 基本的には引数parentのクローン
         */
        Node(const Node *parent);

        /*
         * デバッグ用のメソッド
         * 情報を吐くよ！！
         */
        void draw();

        /*
         * 展開するやつ
         */
        std::vector<Node *> expand() const;
        
        void set_score(i16 score)
        {
                this->score = score;
        }

        u8 toggled_turn() const
        {
                return !turn;
        }
};

class SlantTree {
private:

public:

};

class Search {
private:
        Node *ab(Node *node, u8 depth, i16 a, i16 b);
		i8 slant(Agent agent, Field &field, u8 depth, Direction *result);
public:
        Node *absearch(Node *root);
        i8 slantsearch(Agent agent, Field & field);
};
