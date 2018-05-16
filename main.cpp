#include <iostream>
#include "include/types.hpp"

std::vector<u8> load_src(std::string file_name);

int main()
{
        FieldBuilder builder(12, 12);
	Field mainField;	//繝｡繧､繝ｳ縺ｨ縺ｪ繧九ヵ繧｣繝ｼ繝ｫ繝峨����繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ
	
	mainField.randSetPanel();
        mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();
        load_src("/home/takai/test.dat");
#endif
        
        return 0;
}
