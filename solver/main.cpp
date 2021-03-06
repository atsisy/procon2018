#include <iostream>
#include "lsearch.hpp"
#include "utility.hpp"
#include <chrono>
#include <vector>
#include <fstream>
#include <cstring>
#include <future>
#include "types.hpp"
#include "learn.hpp"
#include <unordered_map>

constexpr u8 MT_NUM_OF_THREAD = 4;

std::unordered_map<u64, te_list *> analyze_learning_data(const char *file);
void command_switching(int argc, char **argv);

std::unordered_map<u64, te_list *> learning_map;

int main(int argc, char **argv)
{
        command_switching(argc, argv);

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

void write_learning_data(const Node *before, const Node *after)
{
        std::ofstream ofs("learning.dat", std::ios::app);
        if(!ofs){
                std::cerr << "Failed to open file." << std::endl;
        }

        std::array<Direction, 4> agent_directions = after->agent_diff(before);

#ifdef I_AM_ME
        std::vector<action> &&states = before->generate_state_hash(MY_TURN);
#endif
#ifdef I_AM_ENEMY
        std::vector<action> &&states = before->generate_state_hash(ENEMY_TURN);
#endif
        Direction d1 = after->get_last_action(0);
        Direction d2 = after->get_last_action(1);

        ofs << states.at(0).state_hash << "\t" << (int)d1 << std::endl;
        ofs << states.at(1).state_hash << "\t" << (int)d2 << std::endl;
}

void write_log_file(const Node *node)
{
        std::ofstream ofs("direction.dat");
        if(!ofs){
                std::cerr << "Failed to open file." << std::endl;
        }

        Direction d1 = node->get_last_action(0);
        Direction d2 = node->get_last_action(1);

        ofs << direction_to_str(d1) << " ";
        ofs << direction_to_str(d2);
}


Plan montecarlo_process(const char *name, u8 turn)
{
        Node *json_node = new Node(name);
        Montecarlo monte;
        u8 d = MONTE_DEPTH - turn;
        const Node *ans = monte.let_me_monte(json_node, d >= 20 ? 20 : d);
        ans->draw();
        //write_learning_data(json_node, ans);
        return Plan(ans->get_last_action(0), ans->get_last_action(1));
}

void multi_thread_carlo(const char *json_name, u8 turn)
{
        std::vector<std::future<Plan>> plans;
        std::vector<Plan> debug_voting;
        VoteBox<u64> box;
        
        std::cout << "**Multi Threading Montecarlo**\nNUMBER OF THREAD ==> " << (int)MT_NUM_OF_THREAD << std::endl;

        for(int i = 0;i < MT_NUM_OF_THREAD;i++){
                plans.push_back(std::async(std::launch::async, montecarlo_process, json_name, turn));
        }

        for(auto &plan : plans){
                Plan tmp = plan.get();
                box.vote(tmp.encode_hash());
                debug_voting.push_back(tmp);
        }

        for(Plan p : debug_voting){
                p.draw();
        }

        auto [d1, d2] = Plan::decode_hash(box.select()).get_direction();
        
        Plan(d1, d2).draw();

        std::ofstream ofs("direction.dat");
        ofs << direction_to_str(d1) << " ";
        ofs << direction_to_str(d2);
}

void command_switching(int argc, char **argv)
{
        if(argc >= 5 && std::string(argv[4]).find(".bin") != std::string::npos){
                learning_map = analyze_learning_data(argv[4]);
        }

        if(!strcmp(argv[1], "init")){
                /*
                 * コマンドライン引数の添字21にQRへのファイルパスが含まれているとする。
                 */
                FieldBuilder builder(new QRFormatParser(argv[2]));
                Node *node = builder.create_root_node();
                node->dump_json_file("root.json");
                node->draw();
                Montecarlo monte;
		u8 d = MONTE_DEPTH - std::atoi(argv[3]);
                const Node *ans = monte.let_me_monte(node, 7);
                ans->draw();
                ans->dump_json_file("cdump.json");
                write_log_file(ans);
                //write_learning_data(node, ans);
                monte.write_out_data_base("db.bin");
                delete node;
        }else if(!strcmp(argv[1], "db")){
                Node *node;
                if(std::string(argv[2]).find(".dat") != std::string::npos){
                        FieldBuilder builder(new QRFormatParser(argv[2]));
                        node = builder.create_root_node();
                }else{
                        node = new Node(argv[2]);
                }
                node->draw();
                Montecarlo monte;
                monte.create_database(node, std::atoi(argv[4]), std::atoi(argv[3]));
                monte.write_out_data_base("db.bin");
                delete node;
        }else if(!strcmp(argv[1], "convert")){
                Node *node;
                if(std::string(argv[2]).find(".dat") != std::string::npos){
                        FieldBuilder builder(new QRFormatParser(argv[2]));
                        node = builder.create_root_node();
                }else{
                        node = new Node(argv[2]);
                }
                node->dump_json_file("cdump.json");
                delete node;
        }else if(!strcmp(argv[1], "continue")){
                Node *json_node = new Node(argv[2]);
                json_node->evaluate();
                Montecarlo monte;
                u8 d = MONTE_DEPTH - std::atoi(argv[3]);
                const Node *ans = monte.let_me_monte(json_node, d >= 20 ? 20 : d);
                ans->draw();
                ans->dump_json_file("cdump.json");
                write_log_file(ans);
                //write_learning_data(json_node, ans);
                delete ans;
                delete json_node;
        }else if(!strcmp(argv[1], "mt")){
                multi_thread_carlo(argv[2], std::atoi(argv[3]));
        }else if(!strcmp(argv[1], "greedy")){
                Node *json_node = new Node(argv[2]);
                json_node->evaluate();
                Montecarlo monte;
                u8 d = MONTE_DEPTH - std::atoi(argv[3]);
                const Node *ans = monte.greedy(json_node);
                ans->draw();
                ans->dump_json_file("cdump.json");
                write_learning_data(json_node, ans);
                write_log_file(ans);
		ans->draw_unko();
                delete ans;
                delete json_node;
        }else if(!strcmp(argv[1], "random")){
                Node *json_node = new Node(argv[2]);
                json_node->evaluate();
                Montecarlo monte;
                const Node *ans = monte.random_play(json_node);
                ans->draw();
                ans->dump_json_file("cdump.json");
                delete ans;
                delete json_node;
        }else if(!strcmp(argv[1], "score")){
                Node *json_node = new Node(argv[2]);
                json_node->put_score_info();
                delete json_node;
        }else if(!strcmp(argv[1], "final-score")){
                Node *json_node = new Node(argv[2]);
                std::cout << json_node->evaluate() << std::endl;
                delete json_node;
        }else if(!strcmp(argv[1], "game-score")){
                Node *json_node = new Node(argv[2]);
                std::cout << json_node->get_green_score() << " ";
                std::cout << json_node->get_red_score() << std::endl;
                delete json_node;
        }else if(!strcmp(argv[1], "gnuscore")){
                Node *json_node = new Node(argv[2]);
                printf("%d %d\n", std::atoi(argv[3]), json_node->evaluate());
                delete json_node;
        }else if(!strcmp(argv[1], "who")){
                who();
        }else if(!strcmp(argv[1], "debug")){
                /*
                FieldBuilder builder(new QRFormatParser(argv[2]));
                builder.print_status();
                test_generate_agent_meta();
                builder.release_resource();
                */
                Node *json_node = new Node(argv[2]);
                std::vector<action> vec = json_node->generate_state_hash(ENEMY_TURN);
                printf("0x%lx, 0x%lx\n", vec.at(0).state_hash, vec.at(1).state_hash);
        }else{
                std::cerr << "the command is missing: you may have experience a problem" << std::endl;
                return;
        }

        exit(0);
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
