#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <cstring>
#include "types.hpp"

int main(int argc, char **argv)
{

        if(!strcmp(argv[1], "init")){
                
                /*
                 * コマンドライン引数の添字21にQRへのファイルパスが含まれているとする。
                 */
                FieldBuilder builder(new QRFormatParser(argv[2]));
                Node *node = builder.create_root_node();
                node->draw();
                Montecarlo monte;
                Node *ans = monte.let_me_monte(node);
                ans->draw();
                ans->dump_json_file("cdump.json");
                delete node;
                return 0;
        }else if(!strcmp(argv[1], "continue")){
                
                Node *json_node = new Node(argv[2]);
                Montecarlo monte;
                Node *ans = monte.let_me_monte(json_node);
                ans->draw();
                ans->dump_json_file("cdump.json");
                delete json_node;
                delete ans;
                return 0;
                
        }else if(!strcmp(argv[1], "score")){
                Node *json_node = new Node(argv[2]);
                std::cout << "SCORE: " << json_node->evaluate() << std::endl;
                delete json_node;
                return 0;
                
        }else if(!strcmp(argv[1], "debug")){
                FieldBuilder builder(new QRFormatParser(argv[2]));
                builder.print_status();
                test_generate_agent_meta();
                builder.release_resource();
                return 0;
        }else{
                std::cerr << "the command is missing: you may have experience a problem" << std::endl;
                
        }
        
		Field mainField;	//メインとなるフィールドのインスタンス
		Search search;

        {
                /*
                 * 安田式アルゴリズムテストコード
                 */
                std::vector<Closed> myclosed;	//閉路を格納するベクター

	
                mainField.randSetPanel();
                
                Agent a1(2,2,generate_agent_meta(MINE_ATTR));
                Agent a2(5,5,generate_agent_meta(MINE_ATTR));
	
// a1を動かす
                a1.move(&mainField,DOWN);
                a1.move(&mainField,DOWN);
                a1.move(&mainField,DOWN);
                a1.move(&mainField,RIGHT);
                a1.move(&mainField,RIGHT);
                Closed::closedFlag.emplace_back(MAKE_HASH(4,5),MAKE_HASH(5,5));
                
// a2を動かす
                a2.move(&mainField,UP);
                a2.move(&mainField,UP);
                a2.move(&mainField,UP);
                a2.move(&mainField,LEFT);
                a2.move(&mainField,LEFT);
                Closed::closedFlag.emplace_back(MAKE_HASH(3,2),MAKE_HASH(2,2));
	
                myclosed.emplace_back(Closed(a1, a2));
                myclosed[0].CalcScore(mainField);
                
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
