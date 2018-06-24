#include "include/types.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <random>
#include <iterator>

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
	std::cout << zerozero_locusnum << std::endl;
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
