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
u8 FieldEvaluater::meta_data;

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


void Field::draw_status()
{
        for(u8 y = 0;y < field_size_y;y++){
                for(u8 x = 0;x < field_size_x;x++){
                        printf(" %c", (at(x, y).is_pure_panel() ? 'P' : at(x, y).is_my_panel() ? 'M' : 'E'));
                }
                printf("\n");
        }
}


#define MAKE_POINT(x, y) ((x) | ((y) << 4))

inline bool is_edge(u8 value)
{
        return !((u8)((value & 0x0f) - 1) < 10) || !((u8)((value >> 4) - 1) < 10);
}


i16 FieldEvaluater::expand_to_arounds(const Field *field, u8 point, std::deque<std::pair<Panel, u8>> & queue, std::vector<u8> & done_list)
{
        i16 score = 0;
        const u8 x = point & 0x0f, y = point >> 4;
        u8 tmp;
        const Panel up = field->at(x, y - 1);
        const Panel right = field->at(x + 1, y);
        const Panel down = field->at(x, y + 1);
        const Panel left = field->at(x - 1, y);

        tmp = MAKE_POINT(x, y - 1);
        if(!up.are_you(meta_data & EXTRACT_PLAYER_INFO) && std::find(std::begin(done_list), std::end(done_list), tmp) == std::end(done_list)){
                score += std::abs(up.get_score_value());
                queue.push_back(std::make_pair(up, tmp));
        }

        tmp = MAKE_POINT(x + 1, y);
        if(!right.are_you(meta_data & EXTRACT_PLAYER_INFO) && std::find(std::begin(done_list), std::end(done_list), tmp) == std::end(done_list)){
                score += std::abs(right.get_score_value());
                queue.push_back(std::make_pair(right, tmp));
        }

        tmp = MAKE_POINT(x, y + 1);
        if(!down.are_you(meta_data & EXTRACT_PLAYER_INFO) && std::find(std::begin(done_list), std::end(done_list), tmp) == std::end(done_list)){
                score += std::abs(down.get_score_value());
                queue.push_back(std::make_pair(down, tmp));
        }

        tmp = MAKE_POINT(x - 1, y);
        if(!left.are_you(meta_data & EXTRACT_PLAYER_INFO) && std::find(std::begin(done_list), std::end(done_list), tmp) == std::end(done_list)){
                score += std::abs(left.get_score_value());
                queue.push_back(std::make_pair(left, tmp));
        }

        return score;
}


i16 FieldEvaluater::calc_sub_local_area_score(const Field *field, const Panel panel, std::deque<std::pair<Panel, u8>> & queue, std::vector<u8> & done_list)
{
        i16 score = 0;
        
        while(queue.size()){
                const std::pair<Panel, u8> panel_pair = queue.front();
                queue.pop_front();
                if(is_edge(panel_pair.second) && panel_pair.first.is_pure_panel()){
                        queue.clear();
                        return 0;
                }
                done_list.push_back(panel_pair.second);
                score += expand_to_arounds(field, panel_pair.second, queue, done_list);
        }

        queue.clear();
        return score;
}

i16 FieldEvaluater::calc_local_area(const Field *field)
{
        std::deque<std::pair<Panel, u8>> queue;
        std::vector<u8> done_list;
        i16 score = 0, tmp_score;

        const u8 y_range = Field::field_size_y - 1;
        const u8 x_range = Field::field_size_x - 1;
        u8 x, y;
        
        for(y = 1;y < y_range;y++){
                for(x = 1;x < x_range;x++){
                        const u8 point = MAKE_POINT(x, y);
                        const Panel panel = field->at(x, y);
                        if(panel.are_you(meta_data & EXTRACT_PLAYER_INFO)){
                                continue;
                        }
                        if(std::find(std::begin(done_list), std::end(done_list), point) != std::end(done_list)){
                                continue;
                        }

                        queue.push_back(std::make_pair(panel, point));

                        tmp_score = calc_sub_local_area_score(field, panel, queue, done_list);
                        score += (tmp_score ? tmp_score + std::abs(panel.get_score_value()) : 0);
                }
        }

        return score;
}

void FieldEvaluater::set_target(u8 flag)
{
        meta_data &= 0xfc;
        meta_data |= flag;
}
