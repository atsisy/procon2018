#include "include/types.hpp"
#include <iostream>

Agent::Agent(u8 x, u8 y, u8 meta)
{
        this->x = x;
        this->y = y;
        this->meta_info = meta;
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

        bool xj = ((u8)(x - (Field::field_size_x - 1)) <= (u8)(-Field::field_size_x + 1));
        bool yj = ((u8)(y - (Field::field_size_y - 1)) <= (u8)(-Field::field_size_y + 1));
        
        /*
         * ストップはどこでもできる
         */
        //dst.push_back(STOP);
        
        if(!xj && !yj){
                dst.push_back(UP);
                dst.push_back(RUP);
                dst.push_back(RIGHT);
                dst.push_back(RDOWN);
                dst.push_back(DOWN);
                dst.push_back(LDOWN);
                dst.push_back(LEFT);
                dst.push_back(LUP);
        }else if(!xj){
                dst.push_back(RIGHT);
                dst.push_back(LEFT);
        }else if(!yj){
                dst.push_back(UP);
                dst.push_back(DOWN);
        }
        
        return dst;
}
