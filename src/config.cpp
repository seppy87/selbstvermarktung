#include"config.hpp"

using namespace std;

string config::config::ip = "";
unsigned short config::config::port = 0;
string config::config::logpath = "";
std::array<config::USHORT, 4> config::config::input = { 1,2,3,4 };
std::array<config::USHORT, 4> config::config::output = { 5,6,7,8 };

std::array<unsigned short, 4> config::splitToStdArray(std::string str) {
	std::array<USHORT, 4> result;
	unsigned short i = 0;
	std::string temp;
	for (auto x : str) {
		if (x == ',') {
			result[i++] = std::stoi(temp);
			temp.clear();
			continue;
		}
		temp += x;
	}
	result[3] = std::stoi(temp);
	return result;
}