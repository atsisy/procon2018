#include <iostream>
#include <vector>
#include "include/types.hpp"

int main()
{
	std::vector<Closed> closed;	//閉路を格納するベクター
	
    FieldBuilder builder(12, 12);
	Field mainField;	//メインとなるフィールドのインスタンス
	
	mainField.randSetPanel();
        mainField.Draw();
       
	Agent a1(2,2,generate_agent_meta(MINE_ATTR));
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();
#endif
        
        return 0;
}
