#include <iostream>
#include <vector>
#include "include/types.hpp"

int main()
{
	std::vector<Closed> myclosed;	//閉路を格納するベクター
	Closed SpaceClosed;	//空のClosed型(Closedを持つvectorにpush_backするときなどに使う)
	
    FieldBuilder builder(12, 12);
	Field mainField;	//メインとなるフィールドのインスタンス
	
	mainField.randSetPanel();
       
	Agent a1(2,2,generate_agent_meta(MINE_ATTR));
	
//a1を動かす
	a1.move(mainField,LDOWN);
	a1.move(mainField, RDOWN);
	a1.move(mainField, RUP);
	myclosed.push_back(SpaceClosed);
	myclosed[0].LoadClosed(a1,mainField,2,2);
	a1.move(mainField, RDOWN);
	a1.move(mainField, LDOWN);
	myclosed.push_back(SpaceClosed);
	myclosed[1].LoadClosed(a1, mainField, 2, 4);
	
	mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        for(Closed closed:myclosed) closed.print_closed(mainField);
        test_generate_agent_meta();
#endif
        
        return 0;
}
