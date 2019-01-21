#pragma once

#include<string>
#include<array>

namespace config {
	typedef unsigned short USHORT;
	struct config {
		static std::string ip;
		static unsigned short port;
		static std::string logpath;
		static std::array<USHORT, 4> input;
		static std::array<USHORT, 4> output;
	};
	std::array<unsigned short, 4> splitToStdArray(std::string str);
}
