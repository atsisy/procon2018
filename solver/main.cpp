#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <string.h>
#include "types.hpp"

int main(int argc, char **argv) {
        FILE *save;
        Node *node;
    	FieldBuilder builder(1,1);
    	if(!strcmp(argv[1], "--init")) {
			builder = FieldBuilder(new QRFormatParser(argv[2]));
			node = builder.create_root_node();
			save = fopen("slantsave.dat", "w");
			fprintf(save, "-1,0,0,0");
			fclose(save);
			node->dump_json_file("cdump.json");
			node->mitgetField()->draw_status();
			std::cout << "\n[\x1b[31m+\x1b[39m] Field initialized\n" << std::endl;
			return 0;
		} else if(!strcmp(argv[1], "-l")) {
			int a[4];
			node = new Node(argv[2]);
			save = fopen("slantsave.dat", "r");
			fscanf(save, "%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3]);
			fclose(save);
			if(a[0] == -1) {
				save = fopen("slantsave.dat", "w");
				fprintf(save, "0,0,0,0");
				fclose(save);	
			}
		} else {
			int a[4];
			save = fopen("slantsave.dat", "r");
			fscanf(save, "%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3]);
			fclose(save);
			if(a[0] != -1) node = new Node(argv[1]);
			else {
				node = new Node("cdump.json");
				save = fopen("slantsave.dat", "w");
				fprintf(save, "0,0,0,0");
			}
			fclose(save);
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
	int tern1 = 0, tern2 = 0;
	save = fopen("slantsave.dat", "r");
	if(save == NULL) {
		std::cout << "[\x1b[31m*\x1b[39m] \"slantsave.dat\" was not founded. Please init system." << std::endl;
		exit(-1);
	} else {
		fscanf(save, "%d,%d,%d,%d", &tern1, &tern2, &search1, &search2);
		printf("tern1 = %d\ntern2 = %d\n", tern1, tern2);
	}
	fclose(save);
	
	if(tern1 == 0) {
		std::cout << "[\x1b[31m+\x1b[39m] search direction..." << std::endl;
		search1 = search.slantsearch(a3, mainField, 10);
		search2 = search.slantsearch(a4, mainField, 10);
		save = fopen("slantsave.dat", "w");
		fprintf(save, "0,0,%d,%d", search1, search2);
		fclose(save);
		std::cout << "[\x1b[31m+\x1b[39m] end searching!" << std::endl;
	}
	
	Panel moved[2];
	a3.setblockdirection(search1);
	a4.setblockdirection(search2);
	moved[0] = a3.moveblock_mytern(mainField, tern1);
	moved[1] = a4.moveblock_mytern(mainField, tern2);
	node->setAgentField(a1, a2, a3, a4, &mainField);
	node->dump_json_file("cdump.json");
	a3.draw();
	a4.draw();
	mainField.draw_status();
	
	save = fopen("slantsave.dat", "w");
	
	if(moved[0].is_not_pure_panel()) tern1 = (tern1+1)%3;
	if(moved[1].is_not_pure_panel()) tern2 = (tern2+1)%3;
	fprintf(save, "%d,%d,%d,%d", tern1, tern2, search1, search2);
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
