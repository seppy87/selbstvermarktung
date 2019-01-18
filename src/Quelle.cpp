#include"cxxopts.hpp"
#ifdef _WIN64
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#pragma comment(lib,"modbus.lib")
#pragma comment(lib,"PocoFoundation.lib")
#include<Windows.h>
#endif

#include"worm.hpp"


#include"events.hpp"
#include"targets.hpp"
#include"workers.hpp"
#include"simpleini.h"



int main(int argc, char** argv) {
	config::config tstruct;
	using cxxopts::value;
	cxxopts::Options options("Selbstvermarktung", "Programm zur Selbstvermarktung");
	options.add_options()
		("c,config", "set path to config file", value<std::string>(), "needs to be in the ini Format!")
		("l,log", "set path to log file", value<std::string>());
	auto result = options.parse(argc, argv);
	if (result["c"].count() == 0) {
		std::cout << "Config file not set. Please start the programm with -c path/to/config.ini or c:\\path\\to\\config.ini" << std::endl;
		return -1;
	}
	if (result["l"].count() > 0) {
		config::config::logpath = result["l"].as<std::string>();
	}
	else
		std::cout << "Log Path not set.!" << std::endl;
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(result["c"].as<std::string>().c_str());
	config::config::ip = ini.GetValue("modbus", "ip", "127.0.0.1");
	config::config::port = std::stoi(ini.GetValue("modbus", "port", "1502"));
	workers::NeXt nextWorker(NULL);
	Poco::ThreadPool::defaultPool().start(nextWorker);

	system("pause");
	return 0;
}