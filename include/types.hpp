#pragma once

#include <cstdint>
#include <vector>
#include <initializer_list>
#include <type_traits>

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

constexpr u8 MINE_ATTR = 0b01;
constexpr u8 ENEMY_ATTR = 0b10;
constexpr u8 PURE_ATTR = 0b00;

constexpr u8 EXTRACT_PLAYER_INFO = 0b00000011;

#define MAKE_HASH(x, y) ((y << 4) | x)

class Field;
class Closed;
/* Panelクラス
 * パネルの情報を保持するクラス
 */
class Panel {

        /*
         * PanelをFieldから操作したいのでフレンドクラスとする
         */
        friend Field;
        friend Closed;
private:
        /*
         * 一度に全ビットを扱いたい場合はplain_bitsを使用
         * ビットフィールドで扱いたい場合はvalueもしくはmeta_infoを使用
         */
        union {
                struct {
                        // パネルのスコア。符号ありa
                        i8 value: 6;

                        /*
                         * メタ情報ビット (2bit)
                         * 上位ビット: ここが1ならば敵のパネル
                         * 下位ビット: ここが1ならば味方のパネル
                         * どちらとも0ならば、どちらにも占領されていないことを示す
                         */
                        u8 meta_info: 2;
                };

                u8 plain_bits;
        };
        
        
        /* 
         * パネルにスコアを格納
         * score:代入する数値
         */
	void set_score_value(i8 score) {
		value = score;
	}
        
public:

        bool is_my_panel()
        {
                return meta_info & MINE_ATTR;
        }

        bool is_enemy_panel()
        {
                return meta_info & ENEMY_ATTR;
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
                meta_info |= MINE_ATTR;
        }

        void make_enemy()
        {
                meta_info |= ENEMY_ATTR;
        }

        void clear_meta()
        {
                meta_info = 0;
        }

        i8 get_score_value()
        {
                return value;
        }

        u8 get_plain_bits()
        {
                return plain_bits;
        }
};

class FieldBuilder;
class Agent;
//class Closed;
/*
 * Fieldクラス
 * 一つのフィールドを表すクラス
 */
class Field {
        /*
         * FieldBuilderクラスからはメタ情報を受け取るため、フレンドクラスとする。
         */
        friend FieldBuilder;
        friend Agent;
        friend Closed;
        
private:
        // アクセスのとき、y座標をどれだけシフトするか
        static u8 ac_shift_offset;

        // フィールドの要素数
        static u8 field_size;
        
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

/*
 * change_atメソッド フィールド情報のアクセサメソッド
 * このメソッドは、fieldメンバを変更することができる
 */
        void make_at(u8 x, u8 y, u8 attribute);

/*
 * フィールドのx,yの値を受け取り、Field一次元配列での要素番号を返す
 */
	u8 xyIndex(u8 x, u8 y) {
		return x+(y<<Field::ac_shift_offset);
	}
	
public:
		
        Field();

        /*
         * atメソッド フィールド情報のアクセサメソッド
         * このメソッドは、fieldメンバを変更することはできない
         */
        Panel at(u8 x, u8 y) const;

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
                _DEBUG_PRINTPI("Field::ac_shift_offset", Field::ac_shift_offset);
                _DEBUG_PRINTPI("Field::field_size", Field::field_size);
                _DEBUG_PUTS_SEPARATOR();
        }
#endif
};

enum Direction {
        UP = 0,
        RUP = 1,
        RIGHT = 2,
        RDOWN = 3,
        DOWN = 4,
        LDOWN = 5,
        LEFT = 6,
        LUP = 7,
        STOP = 8
};

template <typename Head, typename ... Tail>
constexpr u8 generate_agent_meta(const Head head, Tail ... tails) noexcept
{
        u8 result = head;
        using swallow = std::initializer_list<int>;
        (void)swallow{ (void(result |= tails), 0)... };
        return result;
}

#ifdef __DEBUG_MODE
inline void test_generate_agent_meta()
{
        _DEBUG_PUTS_SEPARATOR();
        puts("*debug test for generate_agent_meta constexpr function*");
        constexpr u8 data = generate_agent_meta(MINE_ATTR, ENEMY_ATTR, 4, 8);
        constexpr u8 conv = MINE_ATTR | ENEMY_ATTR | 4 | 8;
        printf("expected -> 0x%x\n", conv);
        printf("result of function ->0x%x\n", data);
        if(conv == data){
                puts("SUCCESS!!");
        }else{
                puts("BUG FIXME: 'u8 generate_agent_meta()'");
        }
        _DEBUG_PUTS_SEPARATOR();
}
#endif

class Agent {
private:
        u8 x: 4;
        u8 y: 4;
        u8 meta_info;

        void move_up()
        {
                y--;
        }

        void move_rup()
        {
                x++;
                y--;
        }

        void move_right()
        {
                x++;
        }

        void move_rdown()
        {
                x++;
                y++;
        }

        void move_down()
        {
                y++;
        }

        void move_ldown()
        {
                x--;
                y++;
        }

        void move_left()
        {
                x--;
        }

        void move_lup()
        {
                x--;
                y--;
        }

        void move_stop()
        {}

        u8 extract_player_info() const
        {
                return meta_info & EXTRACT_PLAYER_INFO;
        }
        
        /*
    *自分の位置からdirectionの方向を見て色が存在するか判定する関数 
    *8近傍を見るとき for でループさせる。このとき 第二引数に i を入れるときは型キャストを忘れないこと！ (Direction)i
    */
           // bool isMine_LookNear(Field & field, Direction direction);
        
public:
        Agent(u8 x, u8 y, u8 meta);
        void move(Field & field, Direction direction);
        std::vector<u8> locus;	//エージェントの動作の軌跡

        bool is_mine();
        bool is_enemy();
        bool isMine_LookNear(Field & field, Direction direction);
};


class Closed {
private:
	//閉路の座標を保存するベクター
	//std::vector<u64> closed;
	
	//引数x,yで示した盤面の位置から見てDirectionの方向にこのclosedのパネルが存在し、かつ指定した座標が閉路の辺の座標でないか判定する関数
	bool CheckPanelLine(u8 x, u8 y, Direction direction);
	
public:
	std::vector<u8> closed;
	
	//agentの今の位置からその軌跡をたどり(end_x, end_y)の座標に向かって閉路を作る
	//一人のエージェントだけで閉路を作成する関数
	u8 LoadClosed(Agent agent, Field & field, u8 end_x, u8 end_y);
	
	//今の閉路のスコアを計算する関数
	u64 CalcScore(Field & field);
	
#ifdef __DEBUG_MODE
        void print_closed(Field & field)
        {
			int sum = 0;
			i8 score;
			_DEBUG_PUTS_SEPARATOR();
			puts("closed's debug message.");
            for(u8 panel:closed) {
				score = field.field[panel].get_score_value();
                printf("score: %d\n", (int)score);
                sum += score;
			}
			printf("\nTotal Score: %d\n", (int)CalcScore(field));
			_DEBUG_PUTS_SEPARATOR();
        }
#endif
	
	void Draw();
};
