#include"cxxopts.hpp"
#ifdef _WIN64
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#pragma comment(lib,"modbus.lib")
#pragma comment(lib,"PocoFoundation.lib")
#pragma comment(lib,"PocoNet.lib")
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>

#endif

#include"worm.hpp"


#include"events.hpp"
#include"targets.hpp"
#include"workers.hpp"
#include"simpleini.h"
#include"httpserver.hpp"


enum {
	TCP,
	TCP_PI,
	RTU
};


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
	auto inputPins = config::splitToStdArray(ini.GetValue("raspberry", "input", "0,1,2,3"));
	auto outputPins = config::splitToStdArray(ini.GetValue("raspberry", "output", "4,5,6,7"));
	config::config::output = outputPins;
	workers::NeXt nextWorker(NULL);
	workers::Netzbetreiber netzBetreiber(inputPins, outputPins);
	Poco::ThreadPool::defaultPool().start(nextWorker);
	http::MyServerApp app;
	app.run(argc, argv);

	system("pause");
	return 0;
}