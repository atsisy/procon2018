#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <string.h>
#include "types.hpp"

int main(int argc, char **argv)
{
        
        /*
         * コマンドライン引数の添字1にQRへのファイルパスが含まれているとする。
         */
        Node *node;
    	FieldBuilder builder(1,1);
        if(!strcmp(argv[1], "-l")) {
			node = new Node(argv[2]);
		} else {
			builder = FieldBuilder(new QRFormatParser(argv[1]));
			node = builder.create_root_node();
		}
		
		Field mainField = *node->mitgetField();
		Search search;
		
		mainField.draw_status();
                        /*
                 * 安田式アルゴリズムテストコード
                 */
                std::vector<Closed> myclosed;	//閉路を格納するベクター
				
                Agent a1 = node->mitgetAgent(1);
                Agent a2 = node->mitgetAgent(2);
                Agent a3 = node->mitgetAgent(3);
                Agent a4 = node->mitgetAgent(4);
                       
    Direction search1, search2;	
	int tern = 0;
	FILE *save = fopen("slantsave.dat", "r");
	if(save == NULL) {
		save = fopen("slantsave.dat", "w");
		fprintf(save, "0,0,0");
	} else {
		fscanf(save, "%d,%d,%d", &tern, &search1, &search2);
	}
	fclose(save);
	
	if(tern == 0) {
		std::cout << "search direction..." << std::endl;
		search1 = search.slantsearch(a3, mainField, 10);
		search2 = search.slantsearch(a4, mainField, 10);
		save = fopen("slantsave.dat", "w");
		fprintf(save, "1,%d,%d", search1, search2);
		fclose(save);
		std::cout << "end searching!" << std::endl;
	}
	
	a3.setblockdirection(search1);
	a4.setblockdirection(search2);
	a3.moveblock_mytern(mainField, tern);
	a4.moveblock_mytern(mainField, tern);
	node->setAgentField(a1, a2, a3, a4, &mainField);
	node->dump_json_file("cdump.json");
	a3.draw();
	a4.draw();
	mainField.draw_status();
	
	save = fopen("slantsave.dat", "w");
	tern = (tern+1)%3;
	fprintf(save, "%d,%d,%d", tern, search1, search2);
	delete node;
                
#ifdef __DEBUG_MODE
                std::cout << "myclosed.size() :" << myclosed.size() << std::endl;
                for(Closed closed:myclosed){
                        closed.print_closed(mainField);
                }
#endif

#ifdef __DEBUG_MODE
	builder.print_status();
	test_generate_agent_meta();
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
