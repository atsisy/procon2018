#include "include/types.hpp"

Agent::Agent(u8 x, u8 y, u8 meta)
{
        this->x = x;
        this->y = y;
        this->meta_info = meta;
}

void Agent::move(Field & field, Direction direction)
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

        field.make_at(this->x, this->y, extract_player_info());
}
