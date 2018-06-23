#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include "include/types.hpp"

/*
* load_full_stringメソッド
* ファイルを文字列形式ですべて読み込むメソッド
*/
std::string QRFormatParser::load_full_string(std::string file_name)
{
	std::ifstream ifs(file_name);
        /*
         * ファイルパスが不正であった場合
         */
	if (ifs.fail()) {
		std::cerr << "FILE NOT FOUND. PATH -> " << file_name << std::endl;
		exit(0);
	}

        /*
         * ファイル読み込み
         */
	std::istreambuf_iterator<char> last;
	std::string full_string(std::istreambuf_iterator<char>(ifs), last);

	return full_string;
}


/*
 * splitメソッド
 * 渡された文字列を指定された文字で分割し、vectorで返却するメソッド
 * 引数
 * s: 文字列
 * delim: 分割する文字
 * 返り値
 * 分割された文字列
 */
std::vector<std::string> QRFormatParser::split(const std::string &&s, char delim)
{
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

/*
 * load_one_lineメソッド
 * 文字列で表現されるマップ一行分のスコアを読み取り、vector<i8>で返却するメソッド
 * 引数
 * part_str: 一行分の情報を含む文字列
 * 返り値
 * 一行分のスコアデータvector<i8>で返却
 */
std::vector<i8> QRFormatParser::load_one_line(const std::string &part_str)
{
	i16 i;
	std::vector<i8> result;
	std::stringstream sstream(part_str);
        
	while (!sstream.eof()){
		sstream >> i;
		result.push_back(i);
	}

        return result;
}

/*
 * get_pair_numbersメソッド
 * "n m"の文字列をpair(n, m)で返却するメソッド
 * first_two_part: "n m"の文字列
 * 返り値
 * nとmが入ったstd::pair
 */
Rect<i16> QRFormatParser::get_pair_numbers(const std::string &first_two_part)
{
        Rect<i16> rect;
	std::stringstream sstream(first_two_part);
        sstream >> rect.height;
        sstream >> rect.width;
        return rect;
}

Rect<i16> QRFormatParser::translate_to_agent_point(const Rect<i16> point)
{
        return Rect<i16>(point.width - 1, point.height - 1);
}

/*
 * QRFormatParserクラスのコンストラクタ
 * 引数
 * file_name: パーサ用のデータが記述されたファイル
 */
QRFormatParser::QRFormatParser(std::string file_name)
{
        /*
         * 文字列をすべて読みこんで':'で分割
         */
        std::vector<std::string> &&str_vec = split(load_full_string(file_name), ':');

        /*
         * フィールドのサイズを取得
         */
        width_height = get_pair_numbers(*std::begin(str_vec));

        /*
         * お尻からエージェントの初期位置を得る
         */
        for(int i = 0;i < 2;i++){
                std::vector<std::string>::iterator tmp = std::end(str_vec) - 1;
                my_agent_point.push_back(translate_to_agent_point(get_pair_numbers(*tmp)));
                str_vec.erase(tmp);
        }
        
        str_vec.erase(std::begin(str_vec));

        /*
         * スコアデータ読み込み
         */
        for(const std::string & str : str_vec){
                auto &&val = load_one_line(str);
                std::copy(std::begin(val), std::end(val), std::back_inserter(scores));
        }

#ifdef __DEBUG_MODE
        _DEBUG_PUTS_SEPARATOR();
        puts("*QRFormatParser DEBUG* : Load Result\n");
        printf("Field: (width : height) = (%d : %d)\n", (int)width_height.width, (int)width_height.height);

        printf("Agent1: (x, y) = (%d, %d)\n", (int)my_agent_point.at(0).width, (int)my_agent_point.at(0).height);
        printf("Agent2: (x, y) = (%d, %d)\n", (int)my_agent_point.at(1).width, (int)my_agent_point.at(1).height);

        puts("Score");
        for(int y = 0;y < width_height.height;y++){
                for(int x = 0;x < width_height.width;x++){
                        printf("%3d", (char)scores.at((y * width_height.width) + x));
                }
                printf("\n");
        }
        _DEBUG_PUTS_SEPARATOR();
#endif
}
