#include <iostream>
#include "include/types.hpp"

int main()
{
        FieldBuilder builder(12, 12);
	Field mainField;	//メインとなるフィールドのインスタンス
	
	mainField.randSetPanel();
        mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
#endif
        
        return 0;
}
