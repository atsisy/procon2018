#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <string.h>
#include "types.hpp"
#include <cmath>

const int depth = 11;

int main(int argc, char **argv) {
        FILE *save;
        Node *node;
    	FieldBuilder builder(1,1);
    	if(!strcmp(argv[1], "init")) {
			builder = FieldBuilder(new QRFormatParser(argv[2]));
			node = builder.create_root_node();
			save = fopen("slantsave.dat", "w");
			fprintf(save, "-1,0,0,0,0,0");
			fclose(save);
			node->dump_json_file("cdump.json");
			node->mitgetField()->draw_status();
			std::cout << "\n[\x1b[31m+\x1b[39m] Field initialized\n" << std::endl;
			return 0;
		} else if(!strcmp(argv[1], "-l")) {
			int a[6];
			node = new Node(argv[2]);
			save = fopen("slantsave.dat", "r");
			fscanf(save, "%d,%d,%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
			fclose(save);
			if(a[0] == -1) {
				save = fopen("slantsave.dat", "w");
				fprintf(save, "0,0,0,0");
				fclose(save);	
			}
		} else {
			int a[6];
			save = fopen("slantsave.dat", "r");
			fscanf(save, "%d,%d,%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
			fclose(save);
			if(a[0] != -1) node = new Node(argv[1]);
			else {
				node = new Node("cdump.json");
				save = fopen("slantsave.dat", "w");
				fprintf(save, "0,0,0,0,0,0");
			}
			fclose(save);
		}
		
		Field mainField = *node->mitgetField();
		Slant slant;
		
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
	save = fopen("slantsave.dat", "r");
	if(save == NULL) {
		std::cerr << "[\x1b[31m*\x1b[39m] \"slantsave.dat\" was not founded. Please init system." << std::endl;
		exit(-1);
	} else {
		fscanf(save, "%d,%d,%d,%d,%d,%d", &a3.turn, &a4.turn, &search1, &search2, &a3.wise, &a4.wise);
		printf("a3.turn = %d\na4.turn = %d\na3.wise = %d\na4.wise = %d\n", a3.turn, a4.turn, a3.wise, a4.wise);
	}
	fclose(save);
	
	if(a3.turn == 0)  {
		// サーチ
		std::cout << "[\x1b[31m+\x1b[39m] search1 direction..." << std::endl;
		search1 = slant.search(a3, mainField, depth, &a3.wise);
		
		save = fopen("slantsave.dat", "w");
		fprintf(save, "%d,%d,%d,%d,%d,%d", a3.turn, a4.turn, search1, search2, a3.wise, a4.wise);
		fclose(save);
		std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
	}
	
	if(a4.turn == 0) {
		// サーチ
		std::cout << "[\x1b[31m+\x1b[39m] search2 direction..." << std::endl;
		search2 = slant.search(a4, mainField, depth, &a4.wise);
		
		save = fopen("slantsave.dat", "w");
		fprintf(save, "%d,%d,%d,%d,%d,%d", a3.turn, a4.turn, search1, search2, a3.wise, a4.wise);
		fclose(save);
		std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
	}
		
	
	Panel moved[2];
	a3.setblockdirection(search1);
	a4.setblockdirection(search2);
	
	if(a3.turn == 2 || a3.turn == 1) {
		int myx = direction_to_dX(a3.getNextMove_mytern(a3.turn, a3.wise));
		int myy = direction_to_dY(a3.getNextMove_mytern(a3.turn, a3.wise));
		if(mainField.at(a3.mitgetX()+myx, a3.mitgetY()+myy).is_enemy_panel()) {
			a3.turn = 0;
			// サーチ
			std::cout << "[\x1b[31m+\x1b[39m] search1 direction..." << std::endl;
			search1 = slant.search(a3, mainField, depth, &a3.wise);

			save = fopen("slantsave.dat", "w");
			fprintf(save, "%d,%d,%d,%d,%d,%d", a3.turn, a4.turn, search1, search2, a3.wise, a4.wise);
			fclose(save);
			std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
		}
		printf("2:a3.turn = %d\n", a3.turn);
	}
	
	if(a4.turn == 2 || a4.turn == 1) {
		int myx = direction_to_dX(a4.getNextMove_mytern(a4.turn, a4.wise));
		int myy = direction_to_dY(a4.getNextMove_mytern(a4.turn, a4.wise));
		if(mainField.at(a4.mitgetX()+myx, a4.mitgetY()+myy).is_enemy_panel()) {
			a4.turn = 0;
			// サーチ
			std::cout << "[\x1b[31m+\x1b[39m] search2 direction..." << std::endl;
			search2 = slant.search(a4, mainField, depth, &a4.wise);
			
			save = fopen("slantsave.dat", "w");
			fprintf(save, "%d,%d,%d,%d,%d,%d", a3.turn, a4.turn, search1, search2, a3.wise, a4.wise);
			fclose(save);
			std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
		}
		printf("2:a4.turn = %d\n", a4.turn);
	}
	
	moved[0] = a3.moveblock_mytern(mainField, a3.turn, a3.wise);
	moved[1] = a4.moveblock_mytern(mainField, a4.turn, a4.wise);
	
	node->setAgentField(a1, a2, a3, a4, &mainField);
	node->dump_json_file("cdump.json");
	mainField.draw_status();
	
	save = fopen("slantsave.dat", "w");
	
	if(moved[0].is_not_pure_panel()) a3.turninc();
	if(moved[1].is_not_pure_panel()) a4.turninc();
	fprintf(save, "%d,%d,%d,%d,%d,%d", a3.turn, a4.turn, search1, search2, a3.wise, a4.wise);
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
