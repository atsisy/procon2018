#include <iostream>
#include "include/types.hpp"

int main()
{
	Field mainField;	// フィールドのインスタンス
        FieldBuilder builder(12, 12);
        
        mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
#endif
        
        return 0;
}
