#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <string.h>
#include "types.hpp"
#include <cmath>

const int depth = 12;

int mywise(Field &field, Agent agent, Direction search);

int main(int argc, char **argv) {
        FILE *save;
        Node *node;
    	FieldBuilder builder(1,1);
    	if(!strcmp(argv[1], "--init")) {
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
	int tern1 = 0, tern2 = 0;
	int wise1 = 0, wise2 = 0;
	save = fopen("slantsave.dat", "r");
	if(save == NULL) {
		std::cout << "[\x1b[31m*\x1b[39m] \"slantsave.dat\" was not founded. Please init system." << std::endl;
		exit(-1);
	} else {
		fscanf(save, "%d,%d,%d,%d,%d,%d", &tern1, &tern2, &search1, &search2, &wise1, &wise2);
		printf("tern1 = %d\ntern2 = %d\nwise1 = %d\nwise2 = %d\n", tern1, tern2, wise1, wise2);
	}
	fclose(save);
	
	if(tern1 == 0)  {
		// サーチ
		std::cout << "[\x1b[31m+\x1b[39m] search1 direction..." << std::endl;
		search1 = slant.search(a3, mainField, depth);
		wise1 = mywise(mainField, a3, search1);
		
		save = fopen("slantsave.dat", "w");
		fprintf(save, "%d,%d,%d,%d,%d,%d", tern1, tern2, search1, search2, wise1, wise2);
		fclose(save);
		std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
	}
	
	if(tern2 == 0) {
		// サーチ
		std::cout << "[\x1b[31m+\x1b[39m] search2 direction..." << std::endl;
		search2 = slant.search(a4, mainField, depth);
		wise2 = mywise(mainField, a4, search2);		
		
		save = fopen("slantsave.dat", "w");
		fprintf(save, "%d,%d,%d,%d,%d,%d", tern1, tern2, search1, search2, wise1, wise2);
		fclose(save);
		std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
	}
		
	
	Panel moved[2];
	a3.setblockdirection(search1);
	a4.setblockdirection(search2);
	
	if(tern1 == 2) {
		int myx = direction_to_dX(a3.getNextMove_mytern(tern1, wise1));
		int myy = direction_to_dY(a3.getNextMove_mytern(tern1, wise1));
		if(mainField.at(a3.mitgetX()+myx, a3.mitgetY()+myy).is_enemy_panel()) {
			tern1 = 0;
			// サーチ
			std::cout << "[\x1b[31m+\x1b[39m] search1 direction..." << std::endl;
			search1 = slant.search(a3, mainField, depth);
			wise1 = mywise(mainField, a3, search1);
		
			save = fopen("slantsave.dat", "w");
			fprintf(save, "%d,%d,%d,%d,%d,%d", tern1, tern2, search1, search2, wise1, wise2);
			fclose(save);
			std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
		}
		printf("2:tern1 = %d\n", tern1);
	}
	
	if(tern2 == 2) {
		int myx = direction_to_dX(a4.getNextMove_mytern(tern2, wise2));
		int myy = direction_to_dY(a4.getNextMove_mytern(tern2, wise2));
		if(mainField.at(a4.mitgetX()+myx, a4.mitgetY()+myy).is_enemy_panel()) {
			tern2 = 0;
			// サーチ
			std::cout << "[\x1b[31m+\x1b[39m] search2 direction..." << std::endl;
			search2 = slant.search(a4, mainField, depth);
			wise2 = mywise(mainField, a4, search2);		
		
			save = fopen("slantsave.dat", "w");
			fprintf(save, "%d,%d,%d,%d,%d,%d", tern1, tern2, search1, search2, wise1, wise2);
			fclose(save);
			std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;	
		}
		printf("2:tern2 = %d\n", tern2);
	}
	
	moved[0] = a3.moveblock_mytern(mainField, tern1, wise1);
	moved[1] = a4.moveblock_mytern(mainField, tern2, wise2);
	
	node->setAgentField(a1, a2, a3, a4, &mainField);
	node->dump_json_file("cdump.json");
	a3.draw();
	a4.draw();
	mainField.draw_status();
	
	save = fopen("slantsave.dat", "w");
	
	if(moved[0].is_not_pure_panel()) tern1 = (tern1+1)%3;
	if(moved[1].is_not_pure_panel()) tern2 = (tern2+1)%3;
	fprintf(save, "%d,%d,%d,%d,%d,%d", tern1, tern2, search1, search2, wise1, wise2);
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

int mywise(Field &field, Agent agent, Direction search) {
	// 時計回りで動いたときの一歩目がすでに自分のパネルであった場合半時計回りへ変更
	if(field.at(agent.mitgetX()+(((search/2)%3+1)/2)*2-1, agent.mitgetY()+(search/4)*2-1).is_enemy_panel()) {	
		std::cout << "agent:CounterClockWise" << std::endl;
		return CounterClockWise;
	} else {
		std::cout << "agent:ClockWise" << std::endl;
		return ClockWise;
	}
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
