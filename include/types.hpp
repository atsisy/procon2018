#pragma once

#include <cstdint>
#include <bits/stdc++.h>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <deque>
#include <unordered_map>

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
using i128 = __int128;

constexpr u8 MINE_ATTR = 0b01;
constexpr u8 ENEMY_ATTR = 0b10;
constexpr u8 PURE_ATTR = 0b00;

constexpr u8 EXTRACT_PLAYER_INFO = 0b00000011;

constexpr i16 STOP_GET_SCORE = 0xDEAD;

#include "utility.hpp"

#define MAKE_HASH(x, y) ((y << 4) | x)

constexpr u8 PANEL_SIMPLIFIED_HASH_SIZE = 6;

class Field;
/* Panelクラス
 * パネルの情報を保持するクラス
 */
class Panel {
	/*
         * PanelをFieldから操作したいのでフレンドクラスとする
         */
	friend Field;

    private:
	/*
         * 一度に全ビットを扱いたい場合はplain_bitsを使用
         * ビットフィールドで扱いたい場合はvalueもしくはmeta_infoを使用
         */
	union {
		struct {
			// パネルのスコア。符号ありa
			i8 value : 6;

			/*
                         * メタ情報ビット (2bit)
                         * 上位ビット: ここが1ならば敵のパネル
                         * 下位ビット: ここが1ならば味方のパネル
                         * どちらとも0ならば、どちらにも占領されていないことを示す
                         */
			u8 meta_info : 2;
		};

		u8 plain_bits;
	};

	/*
         * パネルにスコアを格納
         * score:代入する数値
         */
	void set_score_value(i8 score)
	{
		value = score;
	}

    public:
	bool is_my_panel() const
	{
		return meta_info & MINE_ATTR;
	}

	bool is_enemy_panel() const
	{
		return meta_info & ENEMY_ATTR;
	}

	bool is_not_pure_panel() const
	{
		return meta_info;
	}

	bool is_pure_panel() const
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

	i8 get_score_value() const
	{
		return value;
	}

	u8 get_plain_bits() const
	{
		return plain_bits;
	}

	bool are_you(u8 flag) const
	{
		return meta_info & flag;
	}

	u8 get_meta() const
	{
		return meta_info;
	}

	bool is_abs_my_panel()
	{
#ifdef I_AM_ME
		return meta_info & MINE_ATTR;
#endif
#ifdef I_AM_ENEMY
		return meta_info & ENEMY_ATTR;
#endif
	}

	u8 simplified_hash(i8 pos_avg, i8 neg_avg, u8 who, bool on_agent) const
	{
		u8 ret = 0, tmp = 0;

		/*
                 * スコアの状態で3bit
                 * 平均以上 ... 0b0100
                 * 平均以下 ... 0b0011
                 * 0 ... 0b0010
                 * 平均未満負 ... 0b0001
                 * 平均以下負 ... 0b0000
                 */
		i8 score = get_score_value();
		if (score >= pos_avg)
			tmp = 4;
		else if (score > 0)
			tmp = 3;
		else if (score == 0)
			tmp = 2;
		else if (score >= neg_avg)
			tmp = 1;
		else
			tmp = 0;
		ret |= tmp;
		ret <<= 2;

		/*
                 * パネルの占有状態で2bit
                 */
		if (is_pure_panel())
			tmp = 0;
		else if (are_you(who))
			tmp = 1;
		else
			tmp = 2;
		ret |= tmp;
		ret <<= 1;

		/*
                 * エージェントがその上に乗っているかで1bit
                 */
		if (on_agent) {
			ret |= 1;
		}

		return ret;
	}
};

class FieldBuilder;
class Agent;
class Node;
class FieldEvaluater;

#define MAKE_POINT(x, y) ((x) | ((y) << 4))

constexpr u8 POSITIVE_ONLY = 0;
constexpr u8 NEGATIVE_ONLY = 1;
constexpr u8 WHOLE = 2;

class Closed;

class UF {
public:
        std::vector<int> data;
        std::vector<int> rank;

        UF(int size) {
                data = std::vector<int>(size);
                rank = std::vector<int>(size);
                for(int i=0; i<size; i++) {
                        data[i] = i;
                        rank[i] = 0;
                }
        }

        int root(int x) {
                return data[x] == x ? x : data[x] = root(data[x]);
        }

        bool same(int x, int y) {
                return root(x) == root(y);
        }

        void unite(int x, int y) {
                x = root(x);
                y = root(y);
                if(x == y) return;

                if(rank[x] < rank[y]) {
                        data[x] = y;
                } else {
                        data[y] = x;
                        if(rank[x]  == rank[y]) rank[x]++;
                }
        }
};

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
	friend Node;
	friend FieldEvaluater;

	friend bool is_edge(u8 value);
	friend Closed;

        friend UF;

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
	i16 calc_mypanels_score();
	// 敵のパネルが置かれている部分の合計得点
	i16 calc_enemypanels_score();
	// 自分の合計と敵の合計の差。上記２つの関数を使って差を求めるよりも高速
	i16 calc_sumpanel_score();

	/*
 * make_atメソッド フィールド情報のアクセサメソッド
 * このメソッドは、fieldメンバを変更することができる
 */
	void make_at(u8 x, u8 y, u8 attribute);

	/*
         * set_score_atメソッド フィールド情報のアクセサメソッド
         * このメソッドは、fieldメンバを変更することができる
         */
	void set_score_at(u8 x, u8 y, i8 score);

	/*
 * フィールドのx,yの値を受け取り、Field一次元配列での要素番号を返す
 */
	u8 xyIndex(u8 x, u8 y)
	{
		return x + (y << Field::ac_shift_offset);
	}

        u8 indexY(u8 index) {
                return index/(field_size/field_size_y);
        }

        u8 indexX(u8 index) {
                return index-indexY(index)*(field_size/field_size_y);
        }

        // ある属性のパネルに挟まれているか調べる関数
        bool isPanelMineBetween(int x, int y);
        bool isPanelEnemyBetween(int x, int y);

public:
        Field();

	/*
         * atメソッド フィールド情報のアクセサメソッド
         * このメソッドは、fieldメンバを変更することはできない
         */
	Panel at(u8 x, u8 y) const;

	/*
         * 自分の複製を返すメソッド
         */
	Field *clone() const;

	u64 score();

	// フィールドのパネルの数値をランダムでセットする関数
	void randSetPanel();

	// jsonの文字列返却するメソッド
	std::string dump_json();

        void dump_json_file(const char *file_name);

        i64 max_score();

	// フィールドの描画関数
	void Draw();

	void draw_status();

	i8 get_field_score_avg(u8 flag);

	bool is_within(i8 x, i8 y) const
	{
		return (x >= 0 && x < field_size_x) &&
		       (y >= 0 && y < field_size_y);
	}

        UF makePureTreeMine();
        UF makePureTreeEnemy();
        std::unordered_map<int, std::vector<int>> makePureTerritory(UF &&pureTree);

        // panelを含むpurePanelモデルのスコアを計算
        bool checkLocalArea(int x, int y, u8 attr);
        i16 calcMineScore(std::unordered_map<int, std::vector<int>> &pureTree);
        i16 calcEnemyScore(std::unordered_map<int, std::vector<int>> &pureTree);

	bool is_edge(i8 x, i8 y) const
	{
		return (x <= 0 || x >= (field_size_x - 1)) ||
		       (y <= 0 || y >= (field_size_y - 1));
	}
};

/*
 * 渡された座標（MAKE_POINTでつくったやつ）がエッジまたはOUTの場合trueを返す
 */
inline bool is_edge(u8 value)
{
	return ((u8)((value & 0x0f) - (Field::field_size_x - 1)) <=
		(u8)(-Field::field_size_x + 1)) ||
	       ((u8)((value >> 4) - (Field::field_size_y - 1)) <=
		(u8)(-Field::field_size_y + 1));
}

/*
 * 渡された座標（MAKE_POINTでつくったやつ）がエッジまたはOUTの場合trueを返す
 */
/*
inline bool is_out(u8 value)
{
        return ((u8)((value & 0x0f) - Field::field_size_x) < (u8)(-Field::field_size_x))
                ||
                ((u8)((value >> 4) - Field::field_size_y) < (u8)(-Field::field_size_y));
}
*/

class QRFormatParser;

class FieldBuilder {
    private:
	QRFormatParser *original_data;

    public:
	/*
         * コンストラクタ
         * QRFormatparserオブジェクトへのポインタを渡す
         */
	FieldBuilder(QRFormatParser *parser);

	FieldBuilder(i32 field_size_x, i32 field_size_y);

	/*
         * リソースを解放するメソッド
         */
	void release_resource();
	/*
         * 初期状態のnodeを生成するメソッド
         */
	Node *create_root_node();

#ifdef __DEBUG_MODE
	void print_status()
	{
		_DEBUG_PUTS_SEPARATOR();
		puts("class FieldBuilder debug message.");
		_DEBUG_PRINTPI("Field::ac_shift_offset",
			       Field::ac_shift_offset);
		_DEBUG_PRINTPI("Field::field_size", Field::field_size);
		_DEBUG_PUTS_SEPARATOR();
	}
#endif
};

/*
 * Rectクラス
 * 正方形を表すクラス
 * まあ、幅と高さを表したいときはこのクラス使ってください。
 * 型はご自由に。
 */
template <typename T> class Rect {
    public:
	T width;
	T height;

	Rect(T width, T height)
	{
		this->width = width;
		this->height = height;
	}

	Rect()
	{
		this->width = 0;
		this->height = 0;
	}
};

/*
 * QRコードフォーマットのパーサ
 */
class QRFormatParser {
	friend FieldBuilder;

    private:
	// パネルスコアの羅列
	std::vector<i8> scores;

	// エージェントの座標
	std::vector<Rect<i16> > my_agent_point;

	// フィールドの幅と高さ
	Rect<i16> width_height;

	/*
         * 詳細はメソッド定義の部分に記述
         */
	Rect<i16> get_pair_numbers(const std::string &first_two_part);
	std::vector<i8> load_one_line(const std::string &part_str);
	Rect<i16> translate_to_agent_point(const Rect<i16> point);
	std::vector<std::string> split(const std::string &&s, char delim);
	std::string load_full_string(std::string file_name);

    public:
	QRFormatParser(std::string file_name);
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

inline Direction int_to_direction(int num)
{
	switch (num) {
	case 0:
		return UP;
	case 1:
		return RUP;
	case 2:
		return RIGHT;
	case 3:
		return RDOWN;
	case 4:
		return DOWN;
	case 5:
		return LDOWN;
	case 6:
		return LEFT;
	case 7:
		return LUP;
	case 8:
		return STOP;
	default:
#ifdef __DEBUG_MODE
		std::cout << "invalid number: " << num << std::endl;
#endif
		return STOP;
	};
}

inline std::string direction_to_str(Direction dir)
{
	switch (dir) {
	case UP:
		return std::string("UP");
	case RUP:
		return std::string("RUP");
	case RIGHT:
		return std::string("RIGHT");
	case RDOWN:
		return std::string("RDOWN");
	case DOWN:
		return std::string("DOWN");
	case LDOWN:
		return std::string("LDOWN");
	case LEFT:
		return std::string("LEFT");
	case LUP:
		return std::string("LUP");
	case STOP:
		return std::string("STOP");
	default:
		return std::string("STOP");
	};
}

inline Direction which_direction(i8 x, i8 y)
{
	if (x == -1 && y == -1)
		return LUP;
	else if (x == 0 && y == -1)
		return UP;
	else if (x == 1 && y == -1)
		return RUP;
	else if (x == -1 && y == 0)
		return LEFT;
	else if (x == 0 && y == 0)
		return STOP;
	else if (x == 1 && y == 0)
		return RIGHT;
	else if (x == -1 && y == 1)
		return LDOWN;
	else if (x == 0 && y == 1)
		return DOWN;
	else if (x == 1 && y == 1)
		return RDOWN;
	else {
		puts("BUG in which_direction");
		std::cout << (int)x << ":" << (int)y << std::endl;
	}
	return STOP;
}

template <typename Head, typename... Tail>
constexpr u8 generate_agent_meta(const Head head, Tail... tails) noexcept
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
	if (conv == data) {
		puts("SUCCESS!!");
	} else {
		puts("BUG FIXME: 'u8 generate_agent_meta()'");
	}
	_DEBUG_PUTS_SEPARATOR();
}
#endif

class Agent {
	friend Node;

        friend Node;

private:
        u8 x: 4;
        u8 y: 4;
        u8 meta_info;

        Direction blockdirection;
        u8 blocktern;

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

        void just_move(Direction direction);
        void turn_back(Direction direction);
        std::vector<Direction> movable_direction(Field *field) const;

        /*
         *自分の位置からdirectionの方向を見て色が存在するか判定する関数
         *8近傍を見るとき for でループさせる。このとき 第二引数に i を入れるときは型キャストを忘れないこと！ (Direction)i
         */
	bool isMine_LookNear(Field &field, Direction direction);

    public:
	Agent(u8 x, u8 y, u8 meta);
  Agent(u8 meta);

	void move(Field *field, Direction direction);

	bool is_movable(Field *field, Direction dir);

	i8 get_blockscore(Field &field, Direction k)
	{
		u8 kx = this->x +
			((k / 2 + 1) % 4 - 1) % 2; // kx = agent.x+direction(1)
		u8 ky = this->y + (k / 2 - 1) % 2; // ky = agent.y+direction(1)
		i8 score = field.at(kx + 1, ky).get_score_value() +
			   field.at(kx - 1, ky).get_score_value() +
			   field.at(kx, ky + 1).get_score_value() +
			   field.at(kx, ky - 1).get_score_value();
		return score;
	}

	Agent aftermove_agent(u8 addx, u8 addy)
	{
		return Agent(this->x + addx, this->y + addy, MINE_ATTR);
	}

	void setblockdirection(Direction direction)
	{
		this->blockdirection = direction;
	}

	void moveblock(Field &field)
	{
		// 進むべきブロックの方向と今のターン数から今進むべき方向を計算し移動
		this->move(field, int_to_direction(((7 + blockdirection) % 8 +
						    2 * blocktern) %
						   8));
	}

	bool check_conflict(Direction mine, Agent enemy, Direction es);

	void draw() const;

	void move(Field &field, Direction direction);
	void protected_move(Field *field, Direction direction);
	std::vector<u8> locus; //エージェントの動作の軌跡bool is_mine();
	bool is_enemy();

	bool operator==(const Agent &agent)
	{
		return this->x == agent.x && this->y == agent.y &&
		       this->meta_info == agent.meta_info;
	}

	bool same_location(const Agent &agent) const
	{
		return this->x == agent.x && this->y == agent.y;
	}

	bool same_location(i8 x, i8 y) const
	{
		return this->x == x && this->y == y;
	}

	std::pair<i8, i8> diff(const Agent agent) const
	{
		return std::make_pair(this->x - agent.x, this->y - agent.y);
	}

	bool debug_out(Field *field)
	{
		return !field->is_within(x, y);
	}
};

// 二人のエージェントで閉路を作るときのフラグを管理するクラス
class ClosedFlag {
    private:
	u8 index_me, index_pair;

    public:
	u8 indexme()
	{
		return this->index_me;
	}
	u8 indexpair()
	{
		return this->index_pair;
	}

	ClosedFlag();
	ClosedFlag(u8 index_me, u8 index_pair)
		: index_me(index_me), index_pair(index_pair)
	{
	}
};

class Closed {
    private:
	// 閉路が作成できたか
	bool canMake;

	//閉路の座標を保存するベクター
	std::vector<u8> closed;

	//引数x,yで示した盤面の位置から見てDirectionの方向にこのclosedのパネルが存在し、かつ指定した座標が閉路の辺の座標でないか判定する関数
	bool CheckPanelLine(u8 x, u8 y, Direction direction);

    public:
	// 二人で閉路を作るときのフラグ管理ベクター
	static std::vector<ClosedFlag> closedFlag;

	//今の閉路のスコアを計算する関数
	u64 CalcScore(Field &field);

	// この閉路が正しく作れているか返す関数
	bool canMakeClosed()
	{
		return this->canMake;
	}

	// デフォルトコンストラクタ
	Closed();

	// 一人のエージェントで閉路を生成するコンストラクタ
	Closed(Agent agent, Field &field, u8 end_x, u8 end_y);

	// 二人のエージェントで Closed::closedFlag をもとに閉路を生成するコンストラクタ
	Closed(Agent a1, Agent a2);

#ifdef __DEBUG_MODE
	void print_closed(Field &field)
	{
		int sum = 0;
		i8 score;
		_DEBUG_PUTS_SEPARATOR();
		puts("closed's debug message.");
		for (u8 panel : closed) {
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

class FieldEvaluater {
    private:
	static u8 meta_data;
	static i16
	calc_sub_local_area_score(const Field *field, const Panel panel,
				  util::queue<std::pair<Panel, u8> > &queue,
				  std::vector<u8> &done_list);
	static i16 expand_to_arounds(const Field *field, u8 point,
				     util::queue<std::pair<Panel, u8> > &queue,
				     std::vector<u8> &done_list,
				     std::vector<u8> &checking);

    public:
	static i16 calc_local_area(const Field *field);
	static void set_target(u8 flag);
};

class Plan {
    private:
	Direction d1;
	Direction d2;

    public:
	Plan(Direction d1, Direction d2)
	{
		this->d1 = d1;
		this->d2 = d2;
	}

	Plan()
	{
	}

	u64 encode_hash()
	{
		u64 hash = 0;
		hash |= (char)d1;
		hash <<= 4;
		hash |= (char)d2;
		return hash;
	}

	void draw()
	{
	  std::cout << "[" << direction_to_str(d1) <<  "_" << (int)d1 << ", "
		    << direction_to_str(d2) << "_" << (int)d2 << "]" << std::endl;
	}

	static Plan decode_hash(u64 hash)
	{
		Direction d1, d2;
		d2 = int_to_direction(hash & 0x0f);
		hash >>= 4;
		d1 = int_to_direction(hash & 0x0f);

		return Plan(d1, d2);
	}

	Direction get_d1()
	{
		return d1;
	}

	Direction get_d2()
	{
		return d2;
	}

	std::pair<Direction, Direction> get_direction()
	{
		return std::make_pair(d1, d2);
	}
};

template <typename Cand> class VoteBox {
    private:
	std::unordered_map<Cand, int> box;

    public:
	void vote(Cand cand)
	{
		if (box.find(cand) != std::end(box)) {
			box[cand]++;
		} else {
			box[cand] = 1;
		}
	}

	Cand select()
	{
		int max = 0;
		Cand top;

		for (auto &[cand, count] : box) {
			if (count > max) {
				max = count;
				top = cand;
			}
		}

		return top;
	}
};
