#include "lsearch.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <random>
#include <iterator>
#include <fstream>
#include "picojson.h"

u8 Field::ac_shift_offset;
u8 Field::field_size;
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

/*
 * Field::set_score_atメソッド
 * 指定した座標のパネルのスコアを変更するメソッド
 * このメソッドは必ずプライベートにすること
 */
void Field::set_score_at(u8 x, u8 y, i8 score)
{
        field.at(x + (y << this->ac_shift_offset)).set_score_value(score);
}


i32 Field::which_shougen(i8 x, i8 y)
{
        u8 half_x = field_size_x >> 1;
        u8 half_y = field_size_y >> 1;

        if(half_x >= x && half_y >= y){
                return 1;
        }else if(half_x >= x && half_y < y){
                return 4;
        }else if(half_x < x && half_y < y){
                return 3;
        }else if(half_x < x && half_y >= y){
                return 2;
        }

        puts("BUG? in Field::which_shougen");
        return 2;
}

/*
 * Field::cloneメソッド
 * 自らのクローンを生成し、そのポインタを返すメソッド
 * 引数
 * なし
 * 返り値
 * 動的確保された自分のクローン
 */
Field *Field::clone() const
{
        Field *clone = new Field();
        for(u8 i = 0;i < field.size();i++){
                clone->field[i] = this->field[i];
        }
        return clone;
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

FieldBuilder::FieldBuilder(i32 field_width, i32 field_height)
{
        double tmp;
        
	Field::field_size_x = field_width;
	Field::field_size_y = field_height;

        tmp = std::log2(field_width);
        Field::ac_shift_offset = (u64)(tmp + ((tmp - (u64)tmp) == 0.0 ? 0 : 1));
        
        Field::field_size = field_height << Field::ac_shift_offset;
        original_data = nullptr;
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
i64 Field::calc_mypanels_score()
{
        i16 tmp_score = 0;
        Panel panel;
        u8 i;
        
        for(i = 0;i < Field::field_size;i++){
                panel = field.at(i);
                if(panel.is_my_panel()){
                        tmp_score += panel.get_score_value();
                }
        }
        return tmp_score;
}

/*
 * 敵のパネルが置かれている部分の合計得点を返す
 */
i16 Field::calc_enemypanels_score()
{
        i16 tmp_score = 0;
        Panel panel;
        u8 i;
        
        for(i = 0;i < Field::field_size;i++){
                panel = field.at(i);
                if(panel.is_enemy_panel()){
                        tmp_score += panel.get_score_value();
                }
        }
        return tmp_score;
}

/*
 * 自分の合計と敵の合計の差を返す。上記２つの関数を使って差を求めるよりも高速
 */
i16 Field::calc_sumpanel_score()
{
        i16 tmp_score = 0;
        Panel panel;
        u8 i;
        
        for(i = 0;i < Field::field_size;i++){
                panel = field.at(i);
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


void Field::draw_status()
{
        for(u8 y = 0;y < field_size_y;y++){
                for(u8 x = 0;x < field_size_x;x++){
                        printf(" %c", (at(x, y).is_pure_panel() ? 'P' : at(x, y).is_my_panel() ? 'M' : 'E'));
                }
                printf("\n");
        }
}

std::string Field::dump_json()
{
        picojson::object root;
        picojson::array array;

        root.insert(std::make_pair("width", picojson::value((double)this->field_size_x)));
        root.insert(std::make_pair("height", picojson::value((double)this->field_size_y)));

        for(u8 y = 0;y < field_size_y;y++){
                for(u8 x = 0;x < field_size_x;x++){
                        picojson::object data;
                        data.insert(std::make_pair("x", picojson::value((double)x)));
                        data.insert(std::make_pair("y", picojson::value((double)y)));
                        data.insert(std::make_pair("score", picojson::value(
                                                           (double)(at(x, y).get_score_value()))));
                        data.insert(std::make_pair("attribute", [](const Panel panel)
                                                                        {
                                                                                if(panel.are_you(MINE_ATTR))
                                                                                        return "M";
                                                                                else if(panel.are_you(ENEMY_ATTR))
                                                                                        return "E";
                                                                                else
                                                                                        return "P";
                                                                        }(at(x, y))));
                        array.push_back(picojson::value(data));
                }
        }

        root.insert(std::make_pair("Field", picojson::value(array)));
        return picojson::value(root).serialize();
}

void Field::dump_json_file(const char *file_name)
{
        std::ofstream f(file_name);
        f << dump_json();
}

#define PUSH_AROUND(dst_queue, panel, point, list) { if (                    \
                        !panel.are_you(meta_data & EXTRACT_PLAYER_INFO) && \
                        std::find(std::begin(list), std::end(list), point) == std::end(list)) \
                {                                                       \
                        score += std::abs(panel.get_score_value());     \
                        dst_queue.push_back(std::make_pair(panel, point));            \
                        checking.push_back(point);       \
                }                                                       \
}                 

i16 FieldEvaluater::expand_to_arounds(const Field *field, u8 point, util::queue<std::pair<Panel, u8>> & queue, std::vector<u8> & done_list, std::vector<u8> & checking)
{
        const u8 x = point & 0x0f, y = point >> 4;
        i16 score = 0;

        if(field->is_within(x, y - 1)){
                if(std::find(std::begin(done_list), std::end(done_list), MAKE_POINT(x, y - 1)) != std::end(done_list))
                        return STOP_GET_SCORE;
                const Panel up = field->at(x, y - 1);
                PUSH_AROUND(queue, up, MAKE_POINT(x, y - 1), checking);
        }

        if(field->is_within(x + 1, y)){
                if(std::find(std::begin(done_list), std::end(done_list), MAKE_POINT(x + 1, y)) != std::end(done_list))
                        return STOP_GET_SCORE;
                const Panel right = field->at(x + 1, y);
                PUSH_AROUND(queue, right, MAKE_POINT(x + 1, y), checking);
        }
        
        if(field->is_within(x, y + 1)){
                if(std::find(std::begin(done_list), std::end(done_list), MAKE_POINT(x, y + 1)) != std::end(done_list))
                        return STOP_GET_SCORE;
                const Panel down = field->at(x, y + 1);
                PUSH_AROUND(queue, down, MAKE_POINT(x, y + 1), checking);
        }
        
        if(field->is_within(x - 1, y)){
                if(std::find(std::begin(done_list), std::end(done_list), MAKE_POINT(x - 1, y)) != std::end(done_list))
                        return STOP_GET_SCORE;
                const Panel left = field->at(x - 1, y);
                PUSH_AROUND(queue, left, MAKE_POINT(x - 1, y), checking);
        }
        
        return score;
}


i16 FieldEvaluater::calc_sub_local_area_score(const Field *field, const Panel panel, util::queue<std::pair<Panel, u8>> & queue, std::vector<u8> & done_list)
{
        i16 score = 0, tmp;
        std::vector<u8> checking;
        
        while(queue.size()){
                const std::pair<Panel, u8> panel_pair = queue.front();
                queue.pop_front();

                if(std::find(std::begin(checking), std::end(checking), panel_pair.second) == std::end(checking))
                        checking.push_back(panel_pair.second);
                if((field->is_edge(panel_pair.second & 0x0f, panel_pair.second >> 4)
                   && !panel_pair.first.are_you(meta_data & EXTRACT_PLAYER_INFO))
                   ||
                   std::find(std::begin(done_list), std::end(done_list), panel_pair.second) != std::end(done_list)){
                        queue.clear();
                        std::for_each(std::begin(checking), std::end(checking), [&done_list](u8 val){done_list.push_back(val);});
                        return 0;
                }

                tmp = expand_to_arounds(field, panel_pair.second, queue, done_list, checking);
                if(tmp == STOP_GET_SCORE){
                        std::for_each(std::begin(checking), std::end(checking), [&done_list](u8 val){done_list.push_back(val);});
                        queue.clear();
                        return 0;
                }
                score += tmp;
        }

        std::for_each(std::begin(checking), std::end(checking), [&done_list](u8 val){done_list.push_back(val);});
        queue.clear();
        score += std::abs(panel.get_score_value());
        return score;
}

i16 FieldEvaluater::calc_local_area(const Field *field)
{
        util::queue<std::pair<Panel, u8>> queue;
        std::vector<u8> done_list;
        i16 score = 0;

        const u8 y_range = Field::field_size_y - 1;
        const u8 x_range = Field::field_size_x - 1;
        u8 x, y;
        
        for(y = 1;y < y_range;y++){
                for(x = 1;x < x_range;x++){

                        const Panel panel = field->at(x, y);
                        
                        if(panel.are_you(meta_data & EXTRACT_PLAYER_INFO)){
                                continue;
                        }
                        
                        const u8 point = MAKE_POINT(x, y);
                        
                        if(std::find(std::begin(done_list), std::end(done_list), point) != std::end(done_list)){
                                continue;
                        }
                        
                        queue.push_back(std::make_pair(panel, point));

                        score += calc_sub_local_area_score(field, panel, queue, done_list);
                }
        }

        return score;
}

void FieldEvaluater::set_target(u8 flag)
{
        meta_data &= 0xfc;
        meta_data |= flag;
}

std::vector<ClosedFlag> Closed::closedFlag; 

/*
 * 一人のagentで閉路を作るコンストラクタ
 * agentの今の位置から(end_x, end_y)へ閉路を作ります
 */
Closed::Closed(Agent agent, Field & field, u8 end_x, u8 end_y) {
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
	if(buf == -1) {
		canMake = false;
		return;
	}
	
	// 閉路が作れる場合作成
	u8 locus_size = agent.locus.size();
	for(int i=buf; i<locus_size; i++) {
		this->closed.push_back(agent.locus[i]);	}
	canMake = true;
}

/*
 * 二人のagentで閉路を作るコンストラクタ
 * a1, a2 から Closed::closedFlag をもとに閉路を生成するコンストラクタ
 */
 Closed::Closed(Agent a1, Agent a2) {
	 // closedFlag[1][0]を持つエージェントがどちらか判定
	 //										0:index_me 1:index_pair
	 std::vector<u8> locusBuf;
	 std::copy(a1.locus.begin(), a1.locus.end(), back_inserter(locusBuf));
	 std::sort(locusBuf.begin(), locusBuf.end());	// agent.locus はぐちゃぐちゃにしてはいけないのでバッファに確保
	 std::vector<u8> agent1locus;
	 std::vector<u8> agent2locus ;
	if(std::binary_search(locusBuf.begin(), locusBuf.end(), Closed::closedFlag[1].indexme())==true) {
		std::copy(a1.locus.begin(), a1.locus.end(), back_inserter(agent1locus));
		std::copy(a2.locus.begin(), a2.locus.end(), back_inserter(agent2locus));
	} else {
		std::copy(a2.locus.begin(), a2.locus.end(), back_inserter(agent1locus));
		std::copy(a1.locus.begin(), a1.locus.end(), back_inserter(agent2locus));
	}
			
	// agent1 から closedFlag[0][1]を探す
	int zeroone_locusnum = 0;
	for(int i=0; i<(int)agent1locus.size(); i++) {
		if(agent1locus[i] == Closed::closedFlag[0].indexpair()) {
			zeroone_locusnum = i;
			break;
		}
	}
	
	// agent1 の closedFlag[1][0] から closedFlag[0][1] の経路を this->closed に push_back
	int onezero_locusnum = 0;
	for(int i=0; i<(int)agent1locus.size(); i++) {
		if(agent1locus[i] == Closed::closedFlag[1].indexme()) {
			onezero_locusnum = i;
			break;
		}
	}
	for(int i=std::min(zeroone_locusnum, onezero_locusnum); i<=std::max(zeroone_locusnum, onezero_locusnum); i++) {
		this->closed.push_back(agent1locus[i]);
	}
	
	// agent2 を辿り closedFlag[1][1] を探す
	int oneone_locusnum = 0;
	for(int i=0; i<(int)agent2locus.size(); i++) {
		if(agent2locus[i] == Closed::closedFlag[1].indexpair()) {
			oneone_locusnum = i;
			break;
		}
	}
	
	// agent2 の closedFlag[1][1] から closedFlag[0][0] の経路を this->closed に push_back
	int zerozero_locusnum = 0;
	for(int i=0; i<(int)agent2locus.size(); i++) {
		if(agent2locus[i] == Closed::closedFlag[0].indexme()) {
			zerozero_locusnum = i;
			break;
		}
	}

	for(int i=std::min(oneone_locusnum, zerozero_locusnum); i<=std::max(zerozero_locusnum, oneone_locusnum); i++) {
		this->closed.push_back(agent2locus[i]);
	}
 }

void Closed::Draw() {
	for(int i=0; i<(int)this->closed.size(); i++) {
		std::cout << (int)closed[i] << std::endl;
	}
}

/* 
 * 指定したパネルが経路でないとき、Directionの方向を見てclosedの閉路のパネルが存在したとき true を返す
 * ほぼ閉路スコア計算用
 */
bool Closed::CheckPanelLine(u8 x, u8 y, Direction direction) {
	u8 buf_coordinate;
	u8 buf;
	
	//checkPanelのdirectionの方向にこの経路の持つパネルが存在するか判定
	switch(direction) {
		case UP:
			buf_coordinate = y;
			do {
				buf = MAKE_HASH(x, buf_coordinate);
				for(u8 coordinate:this->closed) {
					if(coordinate == buf) {
						if(buf_coordinate == y) return false;	//指定した場所が経路のときは省く
						return true;
					}
				}
				buf_coordinate--;
			} while(buf_coordinate != 0);
			break;
			
		case RIGHT:
			buf_coordinate = x;
			do {
				buf = MAKE_HASH(buf_coordinate, y);
				for(u8 coordinate:this->closed) {
					if(coordinate == buf) {
						if(buf_coordinate == x) return false;	//指定した場所が経路のときは省く
						return true;
					}
				}
				buf_coordinate++;
			} while(buf_coordinate != Field::field_size_x);
			break;
		
		case DOWN:
			buf_coordinate = y;
			do {
				buf = MAKE_HASH(x, buf_coordinate);
				for(u8 coordinate:this->closed) {
					if(coordinate == buf) {
						if(buf_coordinate == y) return false;	//指定した場所が経路のときは省く
						return true;
					}
				}
				buf_coordinate++;
			} while(buf_coordinate != Field::field_size_y);
			break;

		case LEFT:
			buf_coordinate = x;
			do {
				buf = MAKE_HASH(buf_coordinate, y);
				for(u8 coordinate:this->closed) {
					if(coordinate == buf) {
						if(buf_coordinate == x) return false;	//指定した場所が経路のときは省く
						return true;
					}
				}
				buf_coordinate--;
			} while(buf_coordinate != 0);
			break;
			
		default:
			break;
		}
		
	return false;
}

/*
 * 閉路のスコアを返す関数
 * field:計算する盤面
 */
 u64 Closed::CalcScore(Field & field) {
	 u64 sum = 0;
	 
	 //枠のスコア計算
	 for(u8 coordinate:this->closed) {
		 sum += field.field[coordinate].get_score_value();
	 }
	 
	 //閉路の内側のスコア計算
	 bool ans;
	 /*
	  * 盤面の辺以外のパネルをすべて見てそれが閉路内に入っているか判定
	  * 上下左右の直線状全てに閉路の辺のパネルが存在した場合、そのパネルは閉路内と判定
	  */
	  for(u8 i=1; i<Field::field_size_y-1; i++) {
		  for(u8 j=1; j<Field::field_size_x-1; j++) {		  
			 //上を見る
			  ans = CheckPanelLine(j, i, UP);
			  if(ans == false) continue;
			 //右を見る
			  ans = CheckPanelLine(j, i, RIGHT);
			  if(ans == false) continue;
			 //下を見る
			  ans = CheckPanelLine(j, i, DOWN);
			  if(ans == false) continue;
			 //左を見る
			  ans = CheckPanelLine(j, i, LEFT);
			  
			/*
			 * 上下左右を見たあとの処理
			 * ans == true のときそのパネルは囲まれていると判断
			 * 得点を記録する
			 */
			 if(ans == true) sum += std::abs(field.at(j,i).get_score_value());
		}
	}
	return sum;
 }
