#pragma once

#include "types.hpp"

/*
 * 古典的探索法に見られるノードを表すクラス
 */
class Node {

        friend FieldBuilder;
        
private:
        // フィールド
        Field *field;
        // 評価値
        i64 score;

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
};
