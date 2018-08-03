#include "types.hpp"
#include <iostream>
#include <vector>

Agent::Agent(u8 x, u8 y, u8 meta)
{
        this->x = x;
        this->y = y;
        this->meta_info = meta;
        this->locus.push_back(MAKE_HASH(x,y));
}

void Agent::move(Field *field, Direction direction)
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

#ifdef _ENABLE_YASUDA
        locus.push_back(MAKE_HASH(x,y));
#endif
        
        field->make_at(this->x, this->y, extract_player_info());
}

void Agent::draw()
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
