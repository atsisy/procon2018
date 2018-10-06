#include "lsearch.hpp"
#include "types.hpp"
#include <vector>
#include "picojson.h"
#include <algorithm>
#include <fstream>

constexpr u8 AB_DEPTH = 4;

/*
 * Nodeクラスのコンストラクタ
 * このコンストラクタはプライベートになっていて、原則、FieldBuilderのみ
 * からしかアクセスしてはならないコンストラクタ
 * ルートを生成するためだけのコンストラクタ
 * 引数
 * Field *field
 * ルート用のスコアだけ設定されたFieldオブジェクトへのポインタ
 * Rect<i16> agent1
 * 味方のエージェントの位置ひとつめ
 * Rect<i16> agent2
 * 味方のエージェントの位置ふたつめ
 */
Node::Node(Field *field, Rect<i16> agent1, Rect<i16> agent2)
        : my_agent1(agent1.width, agent1.height, generate_agent_meta(MINE_ATTR)),
          my_agent2(agent2.width, agent2.height, generate_agent_meta(MINE_ATTR)),
          enemy_agent1(agent1.width, agent2.height, generate_agent_meta(ENEMY_ATTR)),
          enemy_agent2(agent2.width, agent1.height, generate_agent_meta(ENEMY_ATTR))
{
        /*
         * ルートノードのために渡すからクローンを作る必要はない。
         */
        this->field = field;

        /*
         * 自分のエージェントを配置
         */
        field->make_at(my_agent1.x, my_agent1.y, MINE_ATTR);
        field->make_at(my_agent2.x, my_agent2.y, MINE_ATTR);

        /*
         * 敵のエージェントを配置
         */
        field->make_at(enemy_agent1.x, enemy_agent1.y, ENEMY_ATTR);
        field->make_at(enemy_agent2.x, enemy_agent2.y, ENEMY_ATTR);

        /*
         * ルートのノードは敵(デフォルト)
         */
#ifdef I_AM_ENEMY
        turn = ENEMY_TURN;
#endif
#ifdef I_AM_ME
        turn = MY_TURN;
#endif

        parent = NULL;
        
        score = 0;
}

/*
 * Nodeクラスのコンストラクタ
 * このコンストラクタはパブリックで利用できる。
 * 基本的に派生元になるNodeが必要となる。
 * このコンストラクタで生成されるNodeオブジェクトは引数parentのクローン
 * 引数
 * const Node *parent
 * クローン元になるNodeオブジェクト
 * 
 */
Node::Node(const Node *parent)
        : my_agent1(parent->my_agent1),
          my_agent2(parent->my_agent2),
          enemy_agent1(parent->enemy_agent1),
          enemy_agent2(parent->enemy_agent2)
{
        /*
         * クローン生成して即代入
         */
        this->field = parent->field->clone();
        score = -10000;
        turn = parent->toggled_turn();
        this->parent = parent;
}

Node::Node(const char *json_path)
        : my_agent1(MINE_ATTR), my_agent2(MINE_ATTR),
          enemy_agent1(ENEMY_ATTR), enemy_agent2(ENEMY_ATTR)
{
        std::ifstream ifs(json_path, std::ios::in);
        if (ifs.fail()) {
                std::cerr << "failed to read json file" << std::endl;
                exit(1);
        }
        const std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        picojson::value v;
        const std::string err = picojson::parse(v, json);
        if (err.empty() == false) {
                std::cerr << err << std::endl;
                exit(1);
        }
        
        picojson::object& obj = v.get<picojson::object>();
        my_agent1.x = (int)obj["agent_m1_x"].get<double>();
        my_agent1.y = (int)obj["agent_m1_y"].get<double>();
        my_agent2.x = (int)obj["agent_m2_x"].get<double>();
        my_agent2.y = (int)obj["agent_m2_y"].get<double>();
        enemy_agent1.x = (int)obj["agent_e1_x"].get<double>();
        enemy_agent1.y = (int)obj["agent_e1_y"].get<double>();
        enemy_agent2.x = (int)obj["agent_e2_x"].get<double>();
        enemy_agent2.y = (int)obj["agent_e2_y"].get<double>();

        picojson::array& array = obj["Field"].get<picojson::array>();
        FieldBuilder builer((i32)obj["width"].get<double>(), (i32)obj["height"].get<double>());
        field = new Field();
        
        for(picojson::value &e : array){
                picojson::object &object = e.get<picojson::object>();
                field->set_score_at(object["x"].get<double>(), object["y"].get<double>(), object["score"].get<double>());
                field->make_at(object["x"].get<double>(), object["y"].get<double>(), [](std::string key){
                                                                                                  if(key == "P")
                                                                                                          return PURE_ATTR;
                                                                                                  else if(key == "M")
                                                                                                          return MINE_ATTR;
                                                                                                  else
                                                                                                          return ENEMY_ATTR;
                                                                                          }(object["attribute"].get<std::string>()));
        }
#ifdef I_AM_ENEMY
        turn = ENEMY_TURN;
#endif
#ifdef I_AM_ME
        turn = MY_TURN;
#endif
}

void Node::first_step_enemy(Direction dir1, Direction dir2)
{
        last_action[0] = dir1;
        last_action[1] = dir2;

        enemy_agent1.protected_move(this->field, (Direction)dir1);
        enemy_agent2.protected_move(this->field, (Direction)dir2);
}


void Node::first_step_mine(Direction dir1, Direction dir2)
{
        last_action[0] = dir1;
        last_action[1] = dir2;

        my_agent1.protected_move(this->field, (Direction)dir1);
        my_agent2.protected_move(this->field, (Direction)dir2);
}

void Node::draw() const
{
        puts("Field Info");
        field->Draw();
        field->draw_status();
        puts("Agent Info");
        puts("Node::my_agent1");
        my_agent1.draw();
        puts("Node::my_agent2");
        my_agent2.draw();
        puts("Node::enemy_agent1");
        enemy_agent1.draw();
        puts("Node::enemy_agent2");
        enemy_agent2.draw();
}

void Node::play(std::array<Direction, 4> dirs)
{
        my_agent1.protected_move(this->field, dirs[0]);
        my_agent2.protected_move(this->field, dirs[1]);
        enemy_agent1.protected_move(this->field, dirs[2]);
        enemy_agent2.protected_move(this->field, dirs[3]);
}

void Node::play_half(Direction d1, Direction d2, u8 turn)
{
        if(IS_MYTURN(turn)){
                my_agent1.protected_move(this->field, d1);
                my_agent2.protected_move(this->field, d2);
        }else{
                enemy_agent1.protected_move(this->field, d1);
                enemy_agent2.protected_move(this->field, d2);
        }
}

i8 Node::check_panel_score(Direction d, Agent agent)
{
        agent.just_move(d);
        return field->at(agent.x, agent.y).get_score_value();
}

std::string Node::dump_json() const
{
        picojson::object root;
        picojson::array array;

        root.insert(std::make_pair("width", picojson::value((double)Field::field_size_x)));
        root.insert(std::make_pair("height", picojson::value((double)Field::field_size_y)));
        std::cout << (double)get_score() << std::endl;
        root.insert(std::make_pair("total_score", picojson::value((double)get_score())));
        

        for(u8 y = 0;y < Field::field_size_y;y++){
                for(u8 x = 0;x < Field::field_size_x;x++){
                        picojson::object data;
                        data.insert(std::make_pair("x", picojson::value((double)x)));
                        data.insert(std::make_pair("y", picojson::value((double)y)));
                        data.insert(std::make_pair("score", picojson::value(
                                                           (double)(field->at(x, y).get_score_value()))));
                        data.insert(std::make_pair("attribute", [](const Panel panel)
                                                                        {
                                                                                if(panel.are_you(MINE_ATTR))
                                                                                        return "M";
                                                                                else if(panel.are_you(ENEMY_ATTR))
                                                                                        return "E";
                                                                                else
                                                                                        return "P";
                                                                        }(field->at(x, y))));
                        array.push_back(picojson::value(data));
                }
        }

        root.insert(std::make_pair("agent_m1_x", picojson::value((double)my_agent1.x)));
        root.insert(std::make_pair("agent_m1_y", picojson::value((double)my_agent1.y)));
        root.insert(std::make_pair("agent_m2_x", picojson::value((double)my_agent2.x)));
        root.insert(std::make_pair("agent_m2_y", picojson::value((double)my_agent2.y)));
        root.insert(std::make_pair("agent_e1_x", picojson::value((double)enemy_agent1.x)));
        root.insert(std::make_pair("agent_e1_y", picojson::value((double)enemy_agent1.y)));
        root.insert(std::make_pair("agent_e2_x", picojson::value((double)enemy_agent2.x)));
        root.insert(std::make_pair("agent_e2_y", picojson::value((double)enemy_agent2.y)));
        root.insert(std::make_pair("turn", picojson::value((double)turn)));

        root.insert(std::make_pair("Field", picojson::value(array)));
        return picojson::value(root).serialize();
}

void Node::dump_json_file(const char *file_name) const
{
        std::ofstream f(file_name);
        f << dump_json();
}

bool Node::nobody(i8 x, i8 y) const
{
        return !my_agent1.same_location(x, y) && !my_agent2.same_location(x, y) &&
                !enemy_agent1.same_location(x, y) && !enemy_agent2.same_location(x, y);
}

std::array<Direction, 4> Node::agent_diff(const Node *node) const
{
        std::array<Direction, 4> ret;
        std::pair<i8, i8> tmp;

        tmp = my_agent1.diff(node->my_agent1);
        ret[0] = which_direction(tmp.first, tmp.second);
        tmp = my_agent2.diff(node->my_agent2);
        ret[1] = which_direction(tmp.first, tmp.second);
        tmp = enemy_agent1.diff(node->enemy_agent1);
        ret[2] = which_direction(tmp.first, tmp.second);
        tmp = enemy_agent2.diff(node->enemy_agent2);
        ret[3] = which_direction(tmp.first, tmp.second);
        
        return ret;
}

std::vector<action> Node::__generate_state_hash(std::vector<Agent> agents) const
{
        i8 x, y;
        std::vector<action> ret;
        
        for(Agent agent : agents){
                u64 hash = 0;
                for(x = agent.x - 1;x <= agent.x + 1;x++){
                        for(y = agent.y - 1;y <= agent.y + 1;y++){
                                hash <<= 5;
                                if(!field->is_within(x, y))
                                        continue;
                                hash |= field->at(x, y).simplified_hash(agent.extract_player_info(), !nobody(x, y));
                        }
                }
                ret.emplace_back(hash);
        }

        return ret;
}

std::vector<action> Node::generate_state_hash(u8 turn) const
{
        std::vector<Agent> agents;
        
        if(IS_MYTURN(turn)){
                agents.push_back(my_agent1);
                agents.push_back(my_agent2);
        }else{
                agents.push_back(enemy_agent1);
                agents.push_back(enemy_agent2);
        }

        return __generate_state_hash(agents);
}

std::vector<action> Node::generate_state_hash() const
{
        return __generate_state_hash({ my_agent1, my_agent2, enemy_agent1, enemy_agent2 });
}

void Node::expand_enemy_node()
{
        //children.reserve(81);
        std::vector<Direction> &&directions1 = enemy_agent1.movable_direction(this->field);
        std::vector<Direction> &&directions2 = enemy_agent2.movable_direction(this->field);
        
        for(const Direction dir1 : directions1){
                for(const Direction dir2 : directions2){

                        if(this->enemy_agent1.check_conflict(((Direction)dir1), this->my_agent1, STOP) ||
                           this->enemy_agent1.check_conflict(((Direction)dir1), this->my_agent2, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->my_agent1, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->my_agent2, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->enemy_agent1, (Direction)dir1)){
                                continue;
                        }

                        /** FIXME
                         * fieldがポインタ参照になってる。
                         * moveメソッドに直接fieldのポインタを渡したい
                         */
                        Node *clone = new Node(this);
                        clone->first_step_enemy(dir1, dir2);
                        children.push_back(clone);
                }
        }
}

bool Node::has_same_pos(const Node *node)
{
        return
                this->my_agent1.same_location(node->my_agent1) &&
                this->my_agent2.same_location(node->my_agent2) &&
                this->enemy_agent1.same_location(node->enemy_agent1) &&
                this->enemy_agent2.same_location(node->enemy_agent2);
                
}

void Node::expand_my_node()
{
        std::vector<Direction> &&directions1 = my_agent1.movable_direction(this->field);
        std::vector<Direction> &&directions2 = my_agent2.movable_direction(this->field);
        
        for(const Direction dir1 : directions1){
                for(const Direction dir2 : directions2){

                        if(this->my_agent1.check_conflict(((Direction)dir1), this->enemy_agent1, STOP) ||
                           this->my_agent1.check_conflict(((Direction)dir1), this->enemy_agent2, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->enemy_agent1, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->enemy_agent2, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->my_agent1, (Direction)dir1)){
                                continue;
                        }
                        
                        /** FIXME
                         * fieldがポインタ参照になってる。
                         * moveメソッドに直接fieldのポインタを渡したい
                         */
                        Node *clone = new Node(this);
                        clone->first_step_mine(dir1, dir2);
                        children.push_back(clone);
                }
        }
}

void Node::expand()
{
        this->children.reserve(81);
        if(turn){
                /*
                 * 敵のノードを展開
                 */
                expand_enemy_node();
        }else{
                /*
                 * 味方のノードを展開
                 */
                expand_my_node();
        }
}

Node *Node::get_specific_child(Direction agent1, Direction agent2)
{
        Node *clone = new Node(this);
        if(IS_MYTURN(turn)){
                clone->my_agent1.protected_move(clone->field, agent1);
                clone->my_agent2.protected_move(clone->field, agent2);
        }else{
                clone->enemy_agent1.protected_move(clone->field, agent1);
                clone->enemy_agent2.protected_move(clone->field, agent2);
        }
        return clone;
}

std::vector<Node *> Node::expand_my_specific_children(std::vector<Direction> &for_a1, std::vector<Direction> &for_a2)
{
        std::vector<Node *> nodes;
        nodes.reserve(36);
        
        for(const Direction dir1 : for_a1){
                for(const Direction dir2 : for_a2){

                        if(this->my_agent1.check_conflict(((Direction)dir1), this->enemy_agent1, STOP) ||
                           this->my_agent1.check_conflict(((Direction)dir1), this->enemy_agent2, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->enemy_agent1, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->enemy_agent2, STOP) ||
                           this->my_agent2.check_conflict(((Direction)dir2), this->my_agent1, (Direction)dir1)){
                                continue;
                        }

                        Node *clone = new Node(this);
                        clone->first_step_mine(dir1, dir2);
                        nodes.push_back(clone);
                }
        }

        return nodes;
}

std::vector<Node *> Node::expand_enemy_specific_children(std::vector<Direction> &for_a1, std::vector<Direction> &for_a2)
{
        std::vector<Node *> nodes;
        nodes.reserve(36);

        for(const Direction dir1 : for_a1){
                for(const Direction dir2 : for_a2){

                        if(this->enemy_agent1.check_conflict(((Direction)dir1), this->my_agent1, STOP) ||
                           this->enemy_agent1.check_conflict(((Direction)dir1), this->my_agent2, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->my_agent1, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->my_agent2, STOP) ||
                           this->enemy_agent2.check_conflict(((Direction)dir2), this->enemy_agent1, (Direction)dir1)){
                                continue;
                        }

                        Node *clone = new Node(this);
                        clone->first_step_enemy(dir1, dir2);
                        nodes.push_back(clone);
                }
        }

        return nodes;
}

std::vector<Node *> Node::expand_specific_children(std::vector<Direction> &&for_a1, std::vector<Direction> &&for_a2)
{
        if(IS_MYTURN(turn)){
                return expand_my_specific_children(for_a1, for_a2);
        }else{
                return expand_enemy_specific_children(for_a1, for_a2);
        }
}

i16 Node::evaluate(u8 turn)
{
        static FieldEvaluater evaluater;

        /*
         * 初期化
         */
        score = 0;
        
        /*
         * FIXME
         * 一発でenemyとmineか判定したい
         */
        evaluater.set_target(MINE_ATTR);
        score += evaluater.calc_local_area(field);
        evaluater.set_target(ENEMY_ATTR);
        score -= evaluater.calc_local_area(field);

        /*
         * 愚直なやつ
         */
        score += this->field->calc_sumpanel_score();

        if(!IS_MYTURN(turn))
                score = -score;
        
        return score;
}

void Node::put_score_info()
{
        puts("** SCORE INFORMATION **");
        std::cout << "*M Field*: " << field->calc_mypanels_score() << std::endl;
        std::cout << "*E Field*: " << field->calc_enemypanels_score() << std::endl;
        std::cout << "*Panel Field*: " << field->calc_sumpanel_score() << std::endl;
        std::cout << "*Total*: " << evaluate() << std::endl;
}

/*
  ab探索法
  擬似言語
 function alphabeta(node, depth, α, β)
 if node が終端ノード or depth = 0
 return node の評価値
 foreach child of node
 α := max(α, -alphabeta(child, depth-1, -β, -α))
 if α ≥ β
 return α // カット
 return α
 */
i64 Search::ab_max(Node *node, u8 depth, i16 a, i16 b)
{
        if(!depth){
                return node->evaluate(ENEMY_TURN);
        }

        node->expand();

        for(Node *n : node->ref_children())
                n->evaluate();
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()), [](const Node *n1, const Node *n2){ return n1->get_score() < n2->get_score();});
        
        
        for(Node *child : node->ref_children()){
                child->set_score(ab_min(child, depth - 1, a, b));
                if(child->get_score() >= b){
                        //delete node;
                        return child->get_score();
                }
                if(child->get_score() > a){
                        // better one
                        a = child->get_score();
                }
        }

        return a;
}

i64 Search::ab_min(Node *node, u8 depth, i16 a, i16 b)
{
        if(!depth){
                return node->evaluate(ENEMY_TURN);
        }
        
        node->expand();
/*

        for(Node *n : node->ref_children())
                n->evaluate();
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()), [](const Node *n1, const Node *n2){ return n1->get_score() > n2->get_score();});

*/
        for(Node *child : node->ref_children()){
                child->set_score(ab_min(child, depth - 1, a, b));
                if(child->get_score() <= a){
                        //delete node;
                        return child->get_score();
                }

                if(child->get_score() < b){
                        //better one
                        b = child->get_score();
                }
        }

        return b;
}

i64 Search::nega_alpha(Node *node, u8 depth, i16 a, i16 b)
{
        if(!depth){
                node->dump_json_file("debug2.json");
                getchar();
                return node->evaluate(ENEMY_TURN);
        }

        node->expand();
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()), [](const Node *n1, const Node *n2){ return n1->get_score() > n2->get_score();});

        for(Node *child : node->ref_children()){
                child->set_score(-nega_alpha(child, depth - 1, -b, -a));
                a = std::max(a, child->get_score());
                
                if(a >= b){
                        if(depth != AB_DEPTH)
                                node->delete_children();
                        return a;
                }
        }

        if(depth != AB_DEPTH)
                node->delete_children();
        return a;
}
/*
Node *Search::node_nega_alpha(Node *node, u8 depth, i16 a, i16 b)
{
        if(!depth){
                node->evaluate(ENEMY_TURN);
                return node;
        }

        node->expand();
        std::sort(std::begin(node->ref_children()), std::end(node->ref_children()), [](const Node *n1, const Node *n2){ return n1->get_score() > n2->get_score();});


        for(Node *child : node->ref_children()){
                child->set_score(-nega_alpha(child, depth - 1, -b, -a)->get_score());
                a = std::max(a, child->get_score());
                if(a >= b){
                        if(depth != AB_DEPTH)
                                node->delete_children();
                        return a;
                }
        }

        if(depth != AB_DEPTH)
                node->delete_children();
        return a;
}
*/

i8 Search::slant(Agent agent, Field &field, u8 depth, Direction *result) {
	
	int ds = -10000;
	int tmp;
	std::vector<i8> discore(4,0);			// up, right, down, left
	
	if(depth == 0) {
		// Up
		tmp = agent.get_blockscore(field, UP);
		if(ds < tmp) ds = tmp;
		
		// Right
		tmp = agent.get_blockscore(field, RIGHT);
		if(ds < tmp) ds = tmp;
		
		// Down
		tmp = agent.get_blockscore(field, DOWN);
		if(ds < tmp) ds = tmp;	
		
		// Left
		tmp = agent.get_blockscore(field, LEFT);
		if(ds < tmp) ds = tmp;
	
		return tmp;
	}
	
	Direction weast;
	discore[0] += slant(agent.aftermove_agent(1, -1), field, depth-1, &weast);
	discore[1] += slant(agent.aftermove_agent(1, 1), field, depth-1, &weast);
	discore[2] += slant(agent.aftermove_agent(-1, 1), field, depth-1, &weast);
	discore[3] += slant(agent.aftermove_agent(-1, -1), field, depth-1, &weast);
	
	i8 max = *std::max_element(discore.begin(), discore.end());
	
	for(int i=0;i<4; i++)
		if(discore[i] == max) *result = (Direction)(i*2); 
	
	return max;
}

void Node::delete_children()
{
        for(Node *c : children)
                delete c;
        children.resize(0);
}

dijkstra_node *dijkstra_node::get_max()
{
        i32 max = -1000000;
        dijkstra_node *ret = nullptr;
        
        for(dijkstra_node *n : neibor){
                if(n->visited)
                        continue;
                if(max < n->cost){
                        max = n->cost;
                        ret = n;
                }
        }

        return ret;
}

Direction Search::dijkstra(Node *node, Agent agent, std::pair<u8, u8> goal_place)
{
        std::vector<dijkstra_node *> done_list;
        std::deque<dijkstra_node *> queue;
        auto &&table = node->virtual_score_table(ENEMY_ATTR);
        dijkstra_node *dijk_node;
        u8 goal_x, goal_y;
        goal_x = goal_place.first;
        goal_y = goal_place.second;

        std::array<std::array<dijkstra_node *, 12>, 12> buf;

        for(u8 x = 0;x < Field::field_size_x;x++){
                for(u8 y = 0;y < Field::field_size_y;y++){
                        buf[x][y] = new dijkstra_node(x, y, table[x][y]);
                }
        }

        for(i8 x = 0;x < Field::field_size_x;x++){
                for(i8 y = 0;y < Field::field_size_y;y++){                        
                        for(i8 lx = x - 1;lx <= x + 1;lx++){
                                for(i8 ly = y - 1;ly <= y + 1;ly++){;
                                        if(!node->field->is_within(lx, ly) || (x == lx && y == ly))
                                                continue;
                                        buf[x][y]->neibor.push_back(buf[lx][ly]);
                                }
                        }
                }
        }
        
        dijk_node = buf[goal_x][goal_y];
        dijk_node->cost = dijk_node->score;;
        for(dijkstra_node *n : dijk_node->neibor){
                n->draw();
        }
        std::cout << "neibors size >>> " << dijk_node->neibor.size() << std::endl;
        while(1){
                dijk_node->visited = true;
                for(dijkstra_node *n : dijk_node->neibor){
                        if(!n->visited){
                                if(n->cost < n->score + dijk_node->cost - 2){
                                        n->cost = n->score + dijk_node->cost - 2;
                                        n->last_update = dijk_node;
                                }
                                queue.push_back(n);
                        }
                }

                done_list.push_back(dijk_node);
                dijk_node = dijk_node->get_max();
                if(dijk_node == nullptr){
                        do{
                                dijk_node = queue.front();
                                queue.pop_front();
                        }while(!dijk_node->visited && queue.size());
                        if(!queue.size())
                                break;
                }
        }

        dijkstra_node *current = buf[agent.x][agent.y];
        dijkstra_node *next = current->last_update;
        
        dijkstra_node *db = current;
        while(1){
                std::cout << (int)db->x << ":" << (int)db->y << " ===> ";
                if(db->x == goal_x && db->y == goal_y)
                        break;
                db = db->last_update;
        }
        printf("\n");
        
        return which_direction(next->x - current->x, next->y - current->y);
}

Node *Search::dijkstra_strategy(Node *node, u8 turn)
{
        Node *clone = new Node(node);
        if(IS_MYTURN(turn)){
                return nullptr;
        }else{
                Direction dir1, dir2;
                auto [goal1, goal2] = find_goal(node, IS_MYTURN(turn) ? MINE_ATTR : ENEMY_ATTR);
                write_out_goal("goal.txt", goal1, goal2);
                dir1 = dijkstra(node, node->enemy_agent1, goal1);
                clone->enemy_agent1.protected_move(clone->field, dir1);
                dir2 = dijkstra(node, node->enemy_agent2, goal2);
                clone->enemy_agent2.protected_move(clone->field, dir2);
                return clone;
        }
}

std::array<std::array<i32, 12>, 12> Node::virtual_score_table(u8 attribute)
{
        std::array<std::array<i32, 12>, 12> table;
        for(auto &elem : table)
                std::fill(std::begin(elem), std::end(elem), 0);
        i32 original_score = evaluate();

        for(u8 x = 0;x < Field::field_size_x;x++){
                for(u8 y = 0;y < Field::field_size_y;y++){
                        if(my_agent1.same_location(x, y) || my_agent2.same_location(x, y) ||
                           enemy_agent1.same_location(x, y) || enemy_agent2.same_location(x, y)){
                                table[x][y] = -1000;
                                continue;
                        }
                        const Panel p = field->at(x, y);
                        if(p.is_pure_panel()){
                                table[x][y] = p.get_score_value();
                                continue;
                        }
                        if(!p.are_you(attribute)){  
                                field->make_at(x, y, PURE_ATTR);
                                i32 changed = evaluate();
                                table[x][y] = original_score - changed;
                                field->make_at(x, y, p.get_meta());
                        }else{
                                table[x][y] = 0;
                        }
                }
        }

        return table;
}

std::pair<std::pair<u8, u8>, std::pair<u8, u8>> Search::read_goal_data(std::ifstream &ifs)
{
        char line[256];
        std::pair<std::pair<u8, u8>, std::pair<u8, u8>> ret;
        int x, y;
        
        ifs.getline(line, 255);
        sscanf(line, "%d %d", &x, &y);

        ret.first = std::make_pair(x, y);
        
        ifs.getline(line, 255);
        sscanf(line, "%d %d", &x, &y);
        ret.second = std::make_pair(x, y);

        return ret;
}

void Search::write_out_goal(const char *name, std::pair<u8, u8> goal1, std::pair<u8, u8> goal2)
{
        std::ofstream ofs(name);

        ofs << (int)goal1.first << " " << (int)goal1.second << "\n";
        ofs << (int)goal2.first << " " << (int)goal2.second;
}

std::pair<std::pair<u8, u8>, std::pair<u8, u8>> Search::find_goal(Node *node, u8 attribute)
{
        std::ifstream ifs("goal.txt");
        std::pair<u8, u8> goal1, goal2;
        
        if(ifs.fail()){
                std::cerr << "goal.txt was not found." << std::endl;
        }else{
                auto [a, b] = read_goal_data(ifs);
                goal1 = a;
                goal2 = b;
        }
        
        auto &&table = node->virtual_score_table(attribute);
        std::vector<point_and_value> candidate;

        
        for(u8 y = 0;y < Field::field_size_y;y++){
                for(u8 x = 0;x < Field::field_size_x;x++){
                        candidate.emplace_back(x, y, table[x][y]);
                        printf("%4d", table[x][y]);
                }
                printf("\n");
        }

        std::sort(std::begin(candidate), std::end(candidate),
                  [](const point_and_value &p1, const point_and_value &p2){
                          return p1.get_value() > p2.get_value();
                  });

        bool g1_done, g2_done;
        g1_done = g2_done = false;
        
        if(node->enemy_agent1.same_location(goal1.first, goal1.second)){
                goal1 = std::make_pair(candidate.at(0).get_x(), candidate.at(0).get_y());
                g1_done = true;
                if(node->enemy_agent2.same_location(goal2.first, goal2.second)){
                        for(auto &&it = std::begin(candidate) + 1;it != std::end(candidate);it++){
                                if(node->field->which_shougen((*it).get_x(), (*it).get_y())
                                   != node->field->which_shougen(goal1.first, goal1.second)){
                                        goal2 = std::make_pair((*it).get_x(), (*it).get_y());
                                        g2_done = true;
                                        break;
                                }
                        }
                }
        }else if(node->enemy_agent2.same_location(goal2.first, goal2.second)){
                for(auto &&it = std::begin(candidate);it != std::end(candidate);it++){
                        if(node->field->which_shougen((*it).get_x(), (*it).get_y())
                           != node->field->which_shougen(goal1.first, goal1.second)){
                                g2_done = true;
                                goal2 = std::make_pair((*it).get_x(), (*it).get_y());
                                break;
                        }
                }
        }

        if(!g1_done){
                if(g2_done){
                        for(auto &&it = std::begin(candidate) + 1;it != std::end(candidate);it++){
                                if(node->field->which_shougen((*it).get_x(), (*it).get_y())
                                   != node->field->which_shougen(goal2.first, goal2.second)){
                                        goal1 = std::make_pair((*it).get_x(), (*it).get_y());
                                        g1_done = true;
                                        break;
                                }
                        }
                }else{
                        goal1 = std::make_pair(candidate.at(0).get_x(), candidate.at(0).get_y());
                        g1_done = true;
                }
        }

        if(!g2_done){
                for(auto &&it = std::begin(candidate);it != std::end(candidate);it++){
                        if(node->field->which_shougen((*it).get_x(), (*it).get_y())
                           != node->field->which_shougen(goal1.first, goal1.second)){
                                g2_done = true;
                                goal2 = std::make_pair((*it).get_x(), (*it).get_y());
                                break;
                        }
                }
        }
        
        return std::make_pair(
                goal1,
                goal2
                ); 
}

std::vector<Node *> Search::absearch(Node *root)
{
        ab_max(root, AB_DEPTH, -10000, 10000);
        std::sort(std::begin(root->ref_children()), std::end(root->ref_children()),
                  [](const Node *n1, const Node *n2){ return n1->get_score() > n2->get_score();});
        /*
        for(Node *n : root->ref_children())
                std::cout << (int)n->get_score() << std::endl;
        */
        std::vector<Node *> result;
        i64 max_score = root->ref_children().at(0)->get_score();
        for(Node *n : root->ref_children()){
                if(max_score != n->get_score()){
                        break;
                }
                printf("%d\n", n->get_score());
                result.push_back(n);
        }

        puts("-------------------------------");
        for(Node *n : result)
                std::cout << (int)n->evaluate() << std::endl;
        std::sort(std::begin(result), std::end(result), [](const Node *n1, const Node *n2){ return n1->get_score() < n2->get_score();});
        
        return result;
}

i8 Search::slantsearch(Agent agent, Field & field) {
	Direction ret;
	slant(agent, field, 1, &ret);
	
	return ret;
}
