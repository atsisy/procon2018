#include <iostream>
#include <vector>
#include "lsearch.hpp"

bool check_sen(std::array<std::vector<Rect<i8>>, 3> history)
{
        if(history[0] == history[2])
}

void slide(std::array<std::vector<Rect<i8>>, 3> &history)
{
        for(int i = 1;i < history.size();;)
                history[i - 1] = history[i];
}

int main(int argc, char **argv)
{
        std::array<std::vector<Rect<i8>>, 3> history;
        Node *current, *prev;

        current = new Node(argv[1]);
        
        while(1){
                getchar();
                prev = current;
                current = new Node(argv[1]);
                slide(history);
                history[history.size() - 1] = current->agent_diff(prev);
                check_sen(history);
        }
}
