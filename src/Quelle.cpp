#include"cxxopts.hpp"
#ifdef _WIN64
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#pragma comment(lib,"modbus.lib")
#pragma comment(lib,"PocoFoundation.lib")
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>

#endif

#include"worm.hpp"


#include"events.hpp"
#include"targets.hpp"
#include"workers.hpp"
#include"simpleini.h"


enum {
	TCP,
	TCP_PI,
	RTU
};

int main(int argc, char*argv[])
{
	modbus_t *ctx;
	uint16_t reg[32];
	modbus_mapping_t *mb_mapping;
	int rc, i, header_length;
	uint8_t *query;
	ctx = modbus_new_tcp("127.0.0.1", 1502);
	query = (uint8_t*)malloc(MODBUS_MAX_ADU_LENGTH);
	header_length = modbus_get_header_length(ctx);
	std::cout << header_length << std::endl;
	mb_mapping = modbus_mapping_new_start_address(
		UT_BITS_ADDRESS, UT_BITS_NB,
		UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
		UT_REGISTERS_ADDRESS, UT_REGISTERS_NB_MAX,
		UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB);
	if (mb_mapping == nullptr) {
		std::cout << "FEHLER" << std::endl;
		return -1;
	}
	std::cout << "Bis hier hat er es geschafft" << std::endl;
	modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, UT_INPUT_BITS_NB,
		UT_INPUT_BITS_TAB);
	for (i = 0; i < UT_INPUT_REGISTERS_NB; i++) {
		mb_mapping->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];;
	}
	auto s = modbus_tcp_listen(ctx, 1);
	modbus_tcp_accept(ctx, &s);
	for (;;) {
		do {
			rc = modbus_receive(ctx, query);
			std::cout << "RC=" << rc << std::endl;
			/* Filtered queries return 0 */
		} while (rc == 0);

		/* The connection is not closed on errors which require on reply such as
		   bad CRC in RTU. */
		if (rc == -1 && errno != EMBBADCRC) {
			/* Quit */
			break;
		}

		/* Special server behavior to test client */
		if (query[header_length] == 0x03) {
			/* Read holding registers */

			if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
				== UT_REGISTERS_NB_SPECIAL) {
				printf("Set an incorrect number of values\n");
				MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
					UT_REGISTERS_NB_SPECIAL - 1);
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_SPECIAL) {
				printf("Reply to this special register address by an exception\n");
				modbus_reply_exception(ctx, query,
					MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
				continue;
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE) {
				const int RAW_REQ_LENGTH = 5;
				uint8_t raw_req[] = {0xFF,0x03,	0x02, 0x00, 0x00
				};

				printf("Reply with an invalid TID or slave\n");
				modbus_send_raw_request(ctx, raw_req, RAW_REQ_LENGTH * sizeof(uint8_t));
				continue;
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_SLEEP_500_MS) {
				printf("Sleep 0.5 s before replying\n");
				Sleep(500);
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS) {
				/* Test low level only available in TCP mode */
				/* Catch the reply and send reply byte a byte */
				uint8_t req[] = "\x00\x1C\x00\x00\x00\x05\xFF\x03\x02\x00\x00";
				int req_length = 11;
				int w_s = modbus_get_socket(ctx);
				if (w_s == -1) {
					fprintf(stderr, "Unable to get a valid socket in special test\n");
					continue;
				}

				/* Copy TID */
				req[1] = query[1];
				for (i = 0; i < req_length; i++) {
					printf("(%.2X)", req[i]);
					Sleep(500);
					rc = send(w_s, (const char*)(req + i), 1, 0);
					if (rc == -1) {
						break;
					}
				}
				continue;
			}
		}

		rc = modbus_reply(ctx, query, rc, mb_mapping);
		if (rc == -1) {
			break;
		}
	}
	modbus_mapping_free(mb_mapping);
	free(query);
	/* For RTU */
	modbus_close(ctx);
	modbus_free(ctx);
	system("pause");
	return 0;
}

/*
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

	system("pause");
	return 0;
}*/