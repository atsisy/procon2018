#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include "types.hpp"

int main(int argc, char **argv)
{
        /*
         * コマンドライン引数の添字1にQRへのファイルパスが含まれているとする。
         */
        FieldBuilder builder(new QRFormatParser(argv[1]));
		Field mainField;	//メインとなるフィールドのインスタンス
		Search search;

        {
                /*
                 * 安田式アルゴリズムテストコード
                 */
                std::vector<Closed> myclosed;	//閉路を格納するベクター

	
                mainField.randSetPanel();
       
                Agent a1(2, 2,generate_agent_meta(MINE_ATTR));
                Agent a2(4,4,generate_agent_meta(MINE_ATTR));
					
                mainField.Draw();
                std::cout << "block: " << (int)a2.get_blockscore(mainField, UP) << std::endl;
                std::cout << "block: " << (int)a2.get_blockscore(mainField, RIGHT) << std::endl;
                std::cout << "block: " << (int)a2.get_blockscore(mainField, DOWN) << std::endl;
                std::cout << "block: " << (int)a2.get_blockscore(mainField, LEFT) << std::endl;
                std::cout << "best direction: " << (int)search.slantsearch(a2, mainField) << std::endl;
                
#ifdef __DEBUG_MODE
                std::cout << "myclosed.size() :" << myclosed.size() << std::endl;
                for(Closed closed:myclosed){
                        closed.print_closed(mainField);
                }
#endif
        }

#ifdef __DEBUG_MODE
        builder.print_status();
        test_generate_agent_meta();


                /* モンテカルロ法
        {
                Node *node = builder.create_root_node();
                node->draw();
                //Search search;
                //search.search(node)->draw();
                Montecarlo monte;
                monte.let_me_monte(node)->draw();
                delete node;
        }
                */

        Node *node = builder.create_root_node();
        Montecarlo monte;
        monte.let_me_monte(node)->draw();
        delete node;
#endif
        builder.release_resource();
        
        return 0;
}

        /*****
              FieldEvaluaterテストコード
        {
                mainField.randSetPanel();
                mainField.Draw();
                mainField.make_at(3, 3, MINE_ATTR);
                mainField.make_at(4, 3, MINE_ATTR);
                mainField.make_at(5, 4, MINE_ATTR);
                mainField.make_at(4, 5, MINE_ATTR);
                mainField.make_at(3, 5, MINE_ATTR);
                mainField.make_at(2, 4, MINE_ATTR);

                mainField.draw_status();
        
                std::chrono::system_clock::time_point  start, end;
                i16 score;
                start = std::chrono::system_clock::now();

                FieldEvaluater::set_target(generate_agent_meta(MINE_ATTR));
                score = FieldEvaluater::calc_local_area(&mainField);

                end = std::chrono::system_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

                std::cout << "score -> " << (int)score << std::endl;
                std::cout << elapsed << "us" << std::endl;
        }
        */
