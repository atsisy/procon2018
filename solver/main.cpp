#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include "types.hpp"

int main(int argc, char **argv)
{

        Node *node = new Node(argv[1]);
        node->draw();
        return 0;

        
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
<<<<<<< HEAD
				Direction go;
				
                Agent a1(2, 2,generate_agent_meta(MINE_ATTR));
                
                for(int i=0; i<20; i++) {
					go = search.slantsearch(a1, mainField, 10);
					std::cout << "moving " << go << "direction" << std::endl;
					a1.setblockdirection(go);
					a1.draw();
					a1.moveblock(mainField);
					a1.draw();
					a1.moveblock(mainField);
					a1.draw();
					a1.moveblock(mainField);
					a1.draw();
				}
                

=======

                mainField.dump_json_file("test.json");
                
                Agent a1(2,2,generate_agent_meta(MINE_ATTR));
                Agent a2(5,5,generate_agent_meta(MINE_ATTR));
	
// a1を動かす
                a1.move(&mainField,DOWN);
                a1.move(&mainField,DOWN);
                a1.move(&mainField,DOWN);
                a1.move(&mainField,RIGHT);
                a1.move(&mainField,RIGHT);
                Closed::closedFlag.emplace_back(MAKE_HASH(4,5),MAKE_HASH(5,5));
>>>>>>> origin/json
                
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
<<<<<<< HEAD
                //Search search;
                //search.search(node)->draw();
                Montecarlo monte;
                monte.let_me_monte(node)->draw();
=======
                Search search;
                
                Node *ans = search.search(node);
                ans->draw();
                ans->dump_json_file("dump.json");
>>>>>>> origin/json
                delete node;
        }
                */

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
