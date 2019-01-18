
#ifdef _WIN64
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#pragma comment(lib,"modbus.lib")
#pragma comment(lib,"PocoFoundation.lib")
#include<Windows.h>
#endif

//#include"cxxopts.hpp"
#include"events.hpp"
#include"targets.hpp"
#include"workers.hpp"

int main(int argc, char** argv) {
	workers::NeXt nextWorker("127.0.0.1", 1502);
	Poco::ThreadPool::defaultPool().start(nextWorker);
	system("pause");
	return 0;
}