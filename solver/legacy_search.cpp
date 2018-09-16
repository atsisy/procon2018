#include "lsearch.hpp"
#include "types.hpp"
#include <vector>
#include "picojson.h"
#include <algorithm>
#include <fstream>

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
                my_agent1.move(this->field, d1);
                my_agent2.move(this->field, d2);
        }else{
                enemy_agent1.move(this->field, d1);
                enemy_agent2.move(this->field, d2);
        }
}

std::string Node::dump_json() const
{
        picojson::object root;
        picojson::array array;

        root.insert(std::make_pair("width", picojson::value((double)Field::field_size_x)));
        root.insert(std::make_pair("height", picojson::value((double)Field::field_size_y)));

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
                        clone->enemy_agent1.protected_move(clone->field, (Direction)dir1);
                        clone->enemy_agent2.protected_move(clone->field, (Direction)dir2);
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
        //children.reserve(81);
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
                        clone->my_agent1.protected_move(clone->field, (Direction)dir1);
                        clone->my_agent2.protected_move(clone->field, (Direction)dir2);
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
                clone->my_agent1.move(clone->field, agent1);
                clone->my_agent2.move(clone->field, agent2);
        }else{
                clone->enemy_agent1.move(clone->field, agent1);
                clone->enemy_agent2.move(clone->field, agent2);  
        }
        return clone;
}

i16 Node::evaluate()
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
                return node->evaluate();
        }

        node->expand();

        for(Node *child : node->ref_children()){
                child->set_score(ab_min(child, depth - 1, a, b));
                if(child->get_score() >= b){
                        delete node;
                        return node->get_score();
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
                return node->evaluate();
        }
        
        node->expand();

        for(Node *child : node->ref_children()){
                child->set_score(ab_min(child, depth - 1, a, b));
                if(child->get_score() <= a){
                        delete node;
                        return child->get_score();
                }

                if(child->get_score() < b){
                        //better one
                        b = child->get_score();
                }
        }

        return b;
}

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

Node *Search::absearch(Node *root)
{
        ab_max(root, 4, -10000, 10000);
        std::sort(std::begin(root->ref_children()), std::end(root->ref_children()), [](const Node *n1, const Node *n2){ return n1->get_score() > n2->get_score();});
        std::for_each(std::begin(root->ref_children()), std::end(root->ref_children()), [](const Node *n){printf("%d\n", n->get_score());});
        /*
         * FIXME
         * 先頭に必ずぶっ壊れたデータが入っている
         */
        return root->ref_children().at(1);
}

i8 Search::slantsearch(Agent agent, Field & field) {
	Direction ret;
	slant(agent, field, 1, &ret);
	
	return ret;
}
