#include "include/types.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <random>

u8 Field::ac_shift_offset;
u8 Field::field_size;
u8 Field::field_size_x;
u8 Field::field_size_y;

Field::Field(): field(field_size)
{}

/*
 * atメソッド　フィールド情報のアクセサメソッド
 * 引数
 * u8 x: フィールドのx座標
 * u8 y: フィールドのy座標
 * 返り値
 * (x, y)のパネル
 */
Panel Field::at(u8 x, u8 y) const
{
        return field.at(x + (y << this->ac_shift_offset));
}

/*
 * make_atメソッド　フィールドの特定のパネルのメタ情報を変更するメソッド
 * 引数
 * u8 x: フィールドのx座標
 * u8 y: フィールドのy座標
 * u8 attribute: これに変更する
 *　　　MINE_ATTR: 自分のパネルに変更
 *　　　ENEMY_ATTR: 敵のパネルに変更
 *　　　PURE_ATTR: どちらでもない。パネル取り除き時
 * 返り値
 * なし
 */
void Field::make_at(u8 x, u8 y, u8 attribute)
{
        switch(attribute){
        case MINE_ATTR:
                field.at(x + (y << this->ac_shift_offset)).make_mine();
                break;
        case ENEMY_ATTR:
                field.at(x + (y << this->ac_shift_offset)).make_enemy();
                break;
        case PURE_ATTR:
                field.at(x + (y << this->ac_shift_offset)).clear_meta();
                break;
        }
}

FieldBuilder::FieldBuilder(u8 width, u8 height)
{
	Field::field_size_x = width;
	Field::field_size_y = height;
        Field::ac_shift_offset = (u64)(std::log2(width) + 0.5);
        Field::field_size = height << Field::ac_shift_offset;
}

/*
 * 自分のパネルが置かれている部分の合計得点を返す
 */
u64 Field::calc_mypanels_score()
{
        u64 tmp_score = 0;
        
        for(Panel panel : field){
                if(panel.is_my_panel()){
                        tmp_score += panel.get_score_value();
                }
        }

        return tmp_score;
}

/*
 * 敵のパネルが置かれている部分の合計得点を返す
 */
u64 Field::calc_enemypanels_score()
{
        u64 tmp_score = 0;
        
        for(Panel panel : field){
                if(panel.is_enemy_panel()){
                        tmp_score += panel.get_score_value();
                }
        }

        return tmp_score;
}

/*
 * 自分の合計と敵の合計の差を返す。上記２つの関数を使って差を求めるよりも高速
 */
u64 Field::calc_sumpanel_score()
{
        u64 tmp_score = 0;
        
        for(Panel panel : field){
                if(panel.is_my_panel()){
                        tmp_score += panel.get_score_value();
                }else if(panel.is_enemy_panel()){
                        tmp_score -= panel.get_score_value();
                }
        }

        return tmp_score;
}

u64 Field::score()
{
        /*
         * 未実装
         */
        return 0;
}

/*
 * フィールドのパネルの得点をランダムにセットします
 * -16 ~ 16
 */
void Field::randSetPanel() {
	std::random_device rnd;
	std::mt19937 mt(rnd());	//メルセンヌ・ツイスタ
	std::uniform_int_distribution<> rand33(0,32);	//0~32の乱数
	for(int i=0; i<(int)field_size; i++) {
		this->field[i].set_score_value(rand33(mt)-16);	//field[i] に -16~16 の乱数をセット
	}
}

/* 
 *フィールドをパネルのスコアを使って描画します 
	*/
void Field::Draw() {
	for(int i=0; i<field_size_y; i++) {
		for(int j=0; j<field_size_x; j++) {
                        std::cout << std::setw(3) << (int)at(j, i).get_score_value();
		}
		std::cout << std::endl;
	}
}

/*
 * 一人のagentで閉路を作る関数
 * agentの今の位置から(end_x, end_y)へ閉路を作ります
 * -1：閉路を作るのに失敗、0：成功
 */
u8 Closed::LoadClosed(Agent agent, Field & field,  u8 end_x, u8 end_y) {
	i8 buf = -1;
	
	//エージェントの動きを最初からたどって目的の場所にたどり着くか判定
	u8 i=0; //カウンタ
	for(u8 coordinate:agent.locus) {
		if(coordinate == MAKE_HASH(end_x, end_y)) {
			buf = i;
			break;
		}
		i++;
	}
	if(buf == -1) return -1;
	
	u8 locus_size = agent.locus.size();
	for(int i=buf; i<locus_size; i++) {
		this->closed.push_back(agent.locus[i]);
	}
	return 0;
}

void Closed::Draw() {
	for(int i=0; i<(int)this->closed.size(); i++) {
		std::cout << (int)closed[i] << std::endl;
	}
}

/*
 * 閉路のスコアを返す関数
 * field:計算する盤面
 */
 u64 Closed::CalcScore(Field & field) {
	 u64 sum = 0;
	 for(u8 coordinate:this->closed) {
		 sum += field.field[coordinate].get_score_value();
	 }

	return sum;
 }
