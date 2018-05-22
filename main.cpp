#include <iostream>
#include "include/types.hpp"

int main()
{
        QRFormatParser parser("/home/takai/test.dat");
        FieldBuilder builder(parser);
	Field mainField;	//繝｡繧､繝ｳ縺ｨ縺ｪ繧九ヵ繧｣繝ｼ繝ｫ繝峨����繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ
	
	mainField.randSetPanel();
        mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();

#endif
        
        return 0;
}
