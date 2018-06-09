#pragma once

#include "types.hpp"


class Node {

        friend FieldBuilder;
        
private:
        Field *field;
        i64 score;
        
        Agent my_agent1;
        Agent my_agent2;
        Agent enemy_agent1;
        Agent enemy_agent2;
        
        Node(Field *field, Rect<i16> agent1, Rect<i16> agent2);
public:
        Node(const Node *parent);

        void draw();
};
