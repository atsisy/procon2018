#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include "include/types.hpp"

/*
* load_full_string関数
* 頂点データファイルを読み込んで、処理し易い用に編集する関数
* load_src関数のお手伝いさんを担ってる
*/
std::string load_full_string(std::string file_name)
{
	std::ifstream ifs(file_name);
	if (ifs.fail()) {
		std::cerr << "FILE NOT FOUND. PATH -> " << file_name << std::endl;
		exit(0);
	}

	std::istreambuf_iterator<char> last;
	std::string full_string(std::istreambuf_iterator<char>(ifs), last);

	return full_string;
}


std::vector<std::string> split(const std::string &&s, char delim) {
	std::vector<std::string> elms;
	size_t offset = 0;
	while (true) {
		size_t next = s.find_first_of(delim, offset);
		if (next == std::string::npos) {
			return elms;
		}
		elms.push_back(s.substr(offset, next - offset));
		offset = next + 1;
	}

	return std::vector<std::string>();
}


std::vector<u8> load_one_line(const std::string &part_str)
{
	i16 i;
	std::vector<u8> result;
	std::stringstream sstream(part_str);
        
	while (!sstream.eof()){
		sstream >> i;
		result.push_back(i);
	}

        return result;
}

std::pair<i16, i16> get_pair_numbers(const std::string &first_two_part)
{
        std::pair<i16, i16> pair_xy;
        i16 i;
	std::stringstream sstream(first_two_part);
        sstream >> pair_xy.first;
        sstream >> pair_xy.second;
        return pair_xy;
}

std::vector<u8> load_src(std::string file_name)
{
	std::vector<u8> result;
        std::vector<std::string> &&str_vec = split(load_full_string(file_name), ':');
        std::pair<i16, i16> pair_xy = get_pair_numbers(*std::begin(str_vec));
        std::vector<std::pair<i16, i16>> agent_point;

        for(int i = 0;i < 2;i++){
                std::vector<std::string>::iterator tmp = std::end(str_vec) - 1;
                agent_point.push_back(get_pair_numbers(*tmp));
                str_vec.erase(tmp);
        }
        
        str_vec.erase(std::begin(str_vec));

        for(const std::string & str : str_vec){
                auto &&val = load_one_line(str);
                std::copy(std::begin(val), std::end(val), std::back_inserter(result));
        }

        FieldBuilder builder(pair_xy.first, pair_xy.second);
        printf("%d %d\n", pair_xy.first, pair_xy.second);
        for(u8 val : result){
                printf("%d\n", (char)val);
        }
	
	return result;
}
