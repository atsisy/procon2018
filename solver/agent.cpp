#include "types.hpp"
#include <iostream>
#include <vector>

Agent::Agent(u8 x, u8 y, u8 meta):blockdirection(UP),blocktern(0)
{
        this->x = x;
        this->y = y;
        this->meta_info = meta;
        this->locus.push_back(MAKE_HASH(x,y));
}

Agent::Agent(u8 meta)
{
        this->meta_info = meta;
        this->locus.push_back(MAKE_HASH(x,y));
}

void Agent::just_move(Direction direction)
{
        switch(direction){
        case UP:
                move_up();
                break;
        case RUP:
                move_rup();
                break;
        case RIGHT:
                move_right();
                break;
        case RDOWN:
                move_rdown();
                break;
        case DOWN:
                move_down();
                break;
        case LDOWN:
                move_ldown();
                break;
        case LEFT:
                move_left();
                break;
        case LUP:
                move_lup();
                break;
        case STOP:
                move_stop();
                break;
        }
}

void Agent::turn_back(Direction direction)
{
        switch(direction){
        case UP:
                move_down();
                break;
        case RUP:
                move_ldown();
                break;
        case RIGHT:
                move_left();
                break;
        case RDOWN:
                move_lup();
                break;
        case DOWN:
                move_up();
                break;
        case LDOWN:
                move_rup();
                break;
        case LEFT:
                move_right();
                break;
        case LUP:
                move_rdown();
                break;
        case STOP:
                move_stop();
                break;
        }
}

void Agent::move(Field *field, Direction direction)
{
        just_move(direction);
#ifdef _ENABLE_YASUDA
        locus.push_back(MAKE_HASH(x,y));
#endif
        
        field->make_at(this->x, this->y, extract_player_info());
}

/*
 * protected_move関数
 * 移動先のパネルが移動するエージェントのチームのものではない場合、pureにする処理が行われる
 */
Panel Agent::protected_move(Field *field, Direction direction)
{
        // エージェントの座標が変わるだけ
        just_move(direction);

        // 新しくエージェントが移動した先のパネル情報
        const Panel panel = field->at(x, y);
        if(panel.get_meta() == ((extract_player_info() & MINE_ATTR) ? ENEMY_ATTR : MINE_ATTR)){
                /*
                 * 移動先のパネルが相手チームのものだった
                 */
                // PUREにする
                field->make_at(x, y, PURE_ATTR);
                // 座標を戻す
                turn_back(direction);
        }else{
                // 移動先にマーク
                field->make_at(this->x, this->y, extract_player_info());
        }
        
        return panel;
}

void Agent::draw() const
{
        printf("AGENT: (x, y) = (%d : %d)\n", (int)x, (int)y);
        std::cout << "ATTRIBUTE: " << ((this->meta_info & 0x01) ? "MINE" : "ENEMY") << std::endl; 
}

std::vector<Direction> Agent::movable_direction(Field *field) const
{
        std::vector<Direction> dst;
        
        /*
         * ストップはどこでもできる
         */
        dst.push_back(STOP);

        if(field->is_within(x - 1, y - 1))
                dst.push_back(LUP);
        if(field->is_within(x, y - 1))
                dst.push_back(UP);
        if(field->is_within(x + 1, y - 1))
                dst.push_back(RUP);
        if(field->is_within(x - 1, y))
                dst.push_back(LEFT);
        if(field->is_within(x + 1, y))
                dst.push_back(RIGHT);
        if(field->is_within(x - 1, y + 1))
                dst.push_back(LDOWN);
        if(field->is_within(x, y + 1))
                dst.push_back(DOWN);
        if(field->is_within(x + 1, y + 1))
                dst.push_back(RDOWN);
        
        return dst;
}

bool Agent::check_conflict(Direction mine, Agent enemy, Direction es)
{
        bool result;
        just_move(mine);
        enemy.just_move(es);
        result = same_location(enemy);
        enemy.turn_back(es);
        turn_back(mine);
        return result;
}

/*
 * direction の方向の隣接したマスがmineの色で塗られているか判定
 */
bool Agent::isMine_LookNear(Field & field, Direction direction) {
	u8 lookPoint_x = x;	//見る場所の座標
	u8 lookPoint_y = y;	
	
        switch(direction){
        case UP:
                lookPoint_y--;
                break;
        case RUP:
				lookPoint_x++;
                lookPoint_y--;
                break;
        case RIGHT:
				lookPoint_x++;
                break;
        case RDOWN:
				lookPoint_x++;
                lookPoint_y++;
                break;
        case DOWN:
                lookPoint_y++;
                break;
        case LDOWN:
				lookPoint_x--;
                lookPoint_y++;
                break;
        case LEFT:
				lookPoint_x--;
                break;
        case LUP:
				lookPoint_x--;
                lookPoint_y--;
                break;
         case STOP:
				break;
        }
        
        // lookPoint の位置が自分の色で塗られているか
	return (bool)field.at(lookPoint_x, lookPoint_y).is_my_panel();
}

bool Agent::is_movable(Field *field, Direction dir)
{
        i8 _x = this->x;
        i8 _y = this->y;
        
        switch(dir){
        case UP:
                _y--;
                break;
        case RUP:
                _x++, _y--;
                break;
        case RIGHT:
                _x++;
                break;
        case RDOWN:
                _x++, _y++;
                break;
        case DOWN:
                _y++;
                break;
        case LDOWN:
                _x--, _y++;
                break;
        case LEFT:
                _x--;
                break;
        case LUP:
                _x--, _y--;
                break;
        }

        return field->is_within(_x, _y);
}
