#pragma once

#include <cstdint>
#include <vector>

#include "debug.hpp"

/*
 * 整数型定義
 */
using u8 = std::uint_fast8_t;
using i8 = std::int_fast8_t;
using u16 = std::uint_fast16_t;
using i16 = std::int_fast16_t;
using u32 = std::uint_fast32_t;
using i32 = std::int_fast32_t;
using u64 = std::uint_fast64_t;
using i64 = std::int_fast64_t;

/* Panelクラス
 * パネルの情報を保持するクラス
 */
class Panel {

private:
        // パネルのスコア。符号あり
        i8 value: 6;

        /*
         * メタ情報ビット (2bit)
         * 上位ビット: ここが1ならば敵のパネル
         * 下位ビット: ここが1ならば味方のパネル
         * どちらとも0ならば、どちらにも占領されていないことを示す
         */
        u8 meta_info: 2;
        
public:

        bool is_my_panel()
        {
                return meta_info & 0b01;
        }

        bool is_enemy_panel()
        {
                return meta_info & 0b10;
        }

        bool is_not_pure_panel()
        {
                return meta_info;
        }

        bool is_pure_panel()
        {
                return !meta_info;
        }

        void make_mine()
        {
                meta_info |= 0b01;
        }

        void make_enemy()
        {
                meta_info |= 0b10;
        }

        void clear_meta()
        {
                meta_info = 0;
        }

        i8 get_score_value()
        {
                return value;
        }
        
        /* 
    * パネルにスコアを格納
    * score:代入する数値
    */
	void set_score_value(i8 score) {
		value = score;
	}
};

class FieldBuilder;

/*
 * Fieldクラス
 * 一つのフィールドを表すクラス
 */
class Field {

        /*
    * FieldBuilderクラスからはメタ情報を受け取るため、フレンドクラスとする。
    */
        friend FieldBuilder;
        
private:
        // アクセスのとき、y座標をどれだけシフトするか
        static u8 ac_shift_offset;

        // フィールドの要素数
        static u64 field_size;
        
        //フィールドのxサイズyサイズ
        static u8 field_size_x;
        static u8 field_size_y;
        
        std::vector<Panel> field;

        /*
    * フィールド上の得点を計算するメソッド
    */
        // 自分のパネルが置かれている部分の合計得点
        u64 calc_mypanels_score();
        // 敵のパネルが置かれている部分の合計得点
        u64 calc_enemypanels_score();
        // 自分の合計と敵の合計の差。上記２つの関数を使って差を求めるよりも高速
        u64 calc_sumpanel_score();

public:
		
        Field();

        // フィールド情報のアクセサメソッド
        Panel at(u8 x, u8 y);
		
        u64 score();
        
        // フィールドのパネルの数値をランダムでセットする関数
        void randSetPanel();
        
        // フィールドの描画関数
        void Draw();
};

class FieldBuilder {
private:

public:
        /*
    * コンストラクタ
    * フィールドの幅と高さを受け取る
    */
        FieldBuilder(u8 width, u8 height);

#ifdef __DEBUG_MODE
        void print_status()
        {
                _DEBUG_PUTS_SEPARATOR();
                puts("class FieldBuilder debug message.");
                _DEBUG_PUTS_SEPARATOR();
        }
#endif
};
