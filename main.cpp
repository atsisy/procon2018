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
	
//a1を動かすだけ
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,DOWN);
	a1.move(mainField,RDOWN);
	a1.move(mainField, RUP);
	a1.move(mainField,RDOWN);
	a1.move(mainField, RUP);
	a1.move(mainField,RDOWN);
	a1.move(mainField, UP);
	a1.move(mainField, UP);
	a1.move(mainField, UP);
	a1.move(mainField, UP);
	a1.move(mainField, UP);
	a1.move(mainField,LUP);
	a1.move(mainField,LEFT);
	a1.move(mainField,LEFT);
	a1.move(mainField,LEFT);
	
	myclosed.push_back(SpaceClosed);
	myclosed[0].LoadClosed(a1, mainField, 2, 2);
	        mainField.Draw();
	myclosed[0].Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();
#endif
        
        return 0;
}
