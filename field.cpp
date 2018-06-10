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

inline bool is_edge(u8 value)
{
        return !((u8)((value & 0x0f) - 1) < 10) || !((u8)((value >> 4) - 1) < 10);
}


i16 Field::expand_one_panel_2_4(u8 point, std::deque<std::pair<Panel, u8>> & queue)
{
        i16 score = 0;
        const u8 x = point & 0x0f, y = point >> 4;
        const Panel up = at(x, y - 1);
        const Panel right = at(x + 1, y);
        const Panel down = at(x, y + 1);
        const Panel left = at(x - 1, y);

        
        if(up.is_pure_panel()){
                score += std::abs(up.get_score_value());
                queue.push_back(std::make_pair(up, MAKE_POINT(x, y - 1)));
        }
        if(right.is_pure_panel()){
                score += std::abs(right.get_score_value());
                queue.push_back(std::make_pair(right, MAKE_POINT(x + 1, y)));
        }
        if(down.is_pure_panel()){
                score += std::abs(down.get_score_value());
                queue.push_back(std::make_pair(down, MAKE_POINT(x, y + 1)));
        }
        if(left.is_pure_panel()){
                score += std::abs(left.get_score_value());
                queue.push_back(std::make_pair(left, MAKE_POINT(x - 1, y)));
        }

        return score;
}


i16 Field::calc_local_area_score_sub(const Panel panel, std::deque<std::pair<Panel, u8>> & queue, std::vector<u8> & done_list)
{
        i16 score = std::abs(panel.get_score_value());
        
        while(queue.size()){
                const std::pair<Panel, u8> panel_pair = queue.front();
                queue.pop_front();
                if(is_edge(panel_pair.second) && panel_pair.first.is_pure_panel()){
                        queue.clear();
                        return 0;
                }
                done_list.push_back(panel_pair.second);
                score += expand_one_panel_2_4(panel_pair.second, queue);
        }

        return score;
}

i16 Field::calc_local_area_score()
{
        std::deque<std::pair<Panel, u8>> queue;
        std::vector<u8> done_list;
        i16 score = 0;

        const u8 y_range = field_size_y - 1;
        const u8 x_range = field_size_x - 1;
        u8 x, y;
        
        for(y = 1;y < y_range;y++){
                for(x = 1;x < x_range;x++){
                        const u8 point = MAKE_POINT(x, y);
                        const Panel panel = at(x, y);
                        if(panel.is_not_pure_panel()){
                                continue;
                        }
                        if(std::find(std::begin(done_list), std::end(done_list), point) != std::end(done_list)){
                                continue;
                        }

                        queue.push_back(std::make_pair(panel, point));
                        
                        score += calc_local_area_score_sub(panel, queue, done_list);
                }
        }

        return score;
}
