#include "types.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

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

Panel Agent::protected_move(Field  *field, Direction direction) {
	Panel p = field->at(x, y);
	
    just_move(direction);
    if(field->at(x, y).are_you(((extract_player_info() & MINE_ATTR) ? ENEMY_ATTR : MINE_ATTR))){
            field->make_at(x, y, PURE_ATTR);
            p = field->at(x, y);
            turn_back(direction);
    }else
            field->make_at(this->x, this->y, extract_player_info());
    return p;
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

bool Agent::checkblock(Field &field, Direction direction) {
	Agent buf(0,0,MINE_ATTR);
	std::vector<Direction> movable = this->movable_direction(&field);
	std::sort(movable.begin(), movable.end());
	if(!std::binary_search(movable.begin(), movable.end(), direction)) return false;
	
	buf = this->aftermove_agent(((direction/2+1)%4-1)%2, (direction/2-1)%2);
	movable = buf.movable_direction(&field);
	if(movable.size() != 9) return false;
	
	return true;
}
