#include "include/lsearch.hpp"
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

void Field::set_score_at(u8 x, u8 y, i8 score)
{
        field.at(x + (y << this->ac_shift_offset)).set_score_value(score);
}

Field *Field::clone() const
{
        return new Field(*this);
}

FieldBuilder::FieldBuilder(QRFormatParser *parser)
{
        double tmp;
        
	Field::field_size_x = parser->width_height.width;
	Field::field_size_y = parser->width_height.height;

        tmp = std::log2(parser->width_height.width);
        Field::ac_shift_offset = (u64)(tmp + ((tmp - (u64)tmp) == 0.0 ? 0 : 1));
        
        Field::field_size = parser->width_height.height << Field::ac_shift_offset;
        original_data = parser;
}

/*
 * FieldBuilder::release_resourceメソッド
 * リソースを解放するためのメソッド
 */
void FieldBuilder::release_resource()
{
        delete original_data;
}

/*
 * FieldBuilder::create_root_nodeメソッド
 * 初期状態のFieldオブジェクトを生成するメソッド
 * 引数
 * 無し
 * 返り値
 * 初期状態のFieldオブジェクトを含むNodeオブジェクトへのポインタ
 */
Node *FieldBuilder::create_root_node()
{
        Field *root_field = new Field;

        /*
         * スコア振り分け
         */
        for(u8 y = 0;y < Field::field_size_y;y++){
                for(u8 x = 0;x < Field::field_size_x;x++){
                        root_field->set_score_at(
                                x, y,
                                original_data->scores.at((y * Field::field_size_x) + x)
                                );
                }
        }

        /*
         * Fieldオブジェクトへのポインタとエージェントの位置情報を渡す
         */
        return new Node(
                root_field,
                original_data->my_agent_point.at(0),
                original_data->my_agent_point.at(1)
                );
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
