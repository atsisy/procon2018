#include "include/types.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>

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
Panel Field::at(u8 x, u8 y)
{
        return field.at(x + (y << this->ac_shift_offset));
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
 *フィールドを○を使って描画します 
	*/
void Field::Draw() {
	for(int i=0; i<field_size_y; i++) {
		for(int j=0; j<field_size_x; j++) {
			std::cout << std::setw(2) << (int)(this->field[i*field_size_x+j].get_score_value());
		}
		std::cout << std::endl;
	}
}
