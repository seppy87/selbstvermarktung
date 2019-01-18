#pragma once

#include<string>

namespace config {
	struct config {
		static std::string ip;
		static unsigned short port;
		static std::string logpath;
	};
}
