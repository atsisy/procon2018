#include <iostream>
#include <vector>
#include "include/types.hpp"

int main()
{
	std::vector<Closed> myclosed;	//閉路を格納するベクター
	
    FieldBuilder builder(12, 12);
	Field mainField;	//メインとなるフィールドのインスタンス
	
	mainField.randSetPanel();
       
	Agent a1(2,2,generate_agent_meta(MINE_ATTR));
	Agent a2(5,5,generate_agent_meta(MINE_ATTR));
	
// a1を動かす
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,RIGHT);
	a1.move(mainField,RIGHT);
	Closed::closedFlag.emplace_back(MAKE_HASH(4,5),MAKE_HASH(5,5));
	
// a2を動かす
	a2.move(mainField,UP);
	a2.move(mainField,UP);
	a2.move(mainField,UP);
	a2.move(mainField,LEFT);
	a2.move(mainField,LEFT);
	Closed::closedFlag.emplace_back(MAKE_HASH(3,2),MAKE_HASH(2,2));
	
	myclosed.emplace_back(Closed(a1, a2));
	myclosed[0].CalcScore(mainField);
	mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        for(Closed closed:myclosed) closed.print_closed(mainField);
        test_generate_agent_meta();
#endif
        
        return 0;
}
