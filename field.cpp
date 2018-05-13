#include "include/types.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <random>

u8 Field::ac_shift_offset;
u64 Field::field_size;
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
	std::uniform_int_distribution<> rand33(0,33);	//0~32の乱数
	for(int i=0; i<(int)field_size; i++) {
		this->field[i].set_score_value(rand33(mt)-16);	//field[i] に -16~16 の乱数をセット
	}
}

/* 
 *フィールドを○を使って描画します 
	*/
void Field::Draw() {
	for(int i=0; i<field_size_y; i++) {
		for(int j=0; j<field_size_x; j++) {
                        std::cout << std::setw(3) << (int)at(j, i).get_score_value();
		}
		std::cout << std::endl;
	}
}


#define MAKE_POINT(x, y) ((x) | ((y) << 4))
#define IS_EDGE(VAL) (!(VAL >> 4) || !(VAL & 0x0f))

void Field::expand_one_panel_2_4(u8 point, std::deque<std::pair<Panel, u8>> & queue)
{
        u8 x = point & 0x0f, y = point >> 4;
        const Panel up = at(x, y - 1);
        const Panel right = at(x + 1, y);
        const Panel down = at(x, y + 1);
        const Panel left = at(x - 1, y);
        if(up.is_pure_panel()) queue.push_back(std::make_pair(up, MAKE_POINT(x, y - 1)));
        if(right.is_pure_panel()) queue.push_back(std::make_pair(right, MAKE_POINT(x + 1, y)));
        if(down.is_pure_panel()) queue.push_back(std::make_pair(down, MAKE_POINT(x, y + 1)));
        if(left.is_pure_panel()) queue.push_back(std::make_pair(left, MAKE_POINT(x - 1, y)));
}


bool Field::calc_local_area_score_sub(const Panel panel, std::deque<std::pair<Panel, u8>> & queue)
{
        while(queue.size()){
                const std::pair<Panel, u8> panel_pair = queue.front();
                queue.pop_front();
                if(IS_EDGE(panel_pair.second) && panel_pair.first.is_pure_panel()){
                        return false;
                }
                expand_one_panel_2_4(panel_pair.second, queue);
        }

        return true;
}

u64 Field::calc_local_area_score()
{
        std::deque<std::pair<Panel, u8>> queue;
        
        for(u8 y = 0;y < field_size_y;y++){
                for(u8 x = 0;x < field_size_x;x++){
                        const Panel panel = at(x, y);
                        if(panel.is_pure_panel() && !y && !x){
                                continue;
                        }
                        calc_local_area_score_sub(panel, queue);
                }
        }

        return 0;
}
