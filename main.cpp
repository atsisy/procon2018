#include <iostream>
#include <chrono>
#include "include/types.hpp"

int main()
{
        FieldBuilder builder(12, 12);
	Field mainField;	//繝｡繧､繝ｳ縺ｨ縺ｪ繧九ヵ繧｣繝ｼ繝ｫ繝峨����繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ
	
	mainField.randSetPanel();
        mainField.Draw();

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();

        mainField.make_at(3, 3, MINE_ATTR);
        mainField.make_at(4, 4, MINE_ATTR);
        mainField.make_at(3, 5, MINE_ATTR);
        mainField.make_at(2, 4, MINE_ATTR);

        std::chrono::system_clock::time_point  start, end;
        i16 score;
        start = std::chrono::system_clock::now();

        score = mainField.calc_local_area_score();

        end = std::chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

        std::cout << "score -> " << (int)score << std::endl;
        std::cout << elapsed << "us" << std::endl;
#endif
        
        return 0;
}
