#include"workers.hpp"
#include<iostream>
#include<exception>

#ifdef _RPi
	#include<wiringPi.h>
#endif


using namespace workers;

void Netzbetreiber::receiveMessage(const void* pSender,std::string& msg) {
	if (msg.substr(0, std::string("error").length()) == "error") {
		this->ErrorAbort = true;
		return;
	}
	if (msg.substr(0, std::string("quit").length()) == "quit") {
		this->quitRequest = true;
		return;
	}
}

void Netzbetreiber::initPins() {
#ifdef _RPi
	if (wiringPiSetup() == -1)
		throw new std::exception("Library wiringPi Init failed!");
	for (auto i : this->input) {
		pinMode(i, INPUT);
	}
	for (auto o : this->output) {
		pinMode(o, OUTPUT);
	}
#endif
}

void Netzbetreiber::run() {
	this->prEvent.theEvent += Poco::delegate(&this->redPower, &target::ReducePowerTarget::onEvent);
	std::array<bool, 4> temp;
	while (this->quitRequest == false && this->ErrorAbort == false) {
#ifdef _RPi
		//read All Inputs
		std::array<bool, 4> i;
		i[0] = digitalRead(input[0]) > 0 ? true : false;
		i[1] = digitalRead(input[1]) > 0 ? true : false;
		i[2] = digitalRead(input[2]) > 0 ? true : false;
		i[3] = digitalRead(input[3]) > 0 ? true : false;
		
		if (i != former) {
			this->prEvent.inputReceived(this->getPercentage(i));
			former = i;
		}
#endif
		Sleep(1500);
	}
	//auto x = this->former == temp ? true : false;
}

UINT16 Netzbetreiber::getPercentage(const std::array<bool, 4>& arg) {
	if (arg[0] == false) return 0;
	if (arg[1] == false) return 25;
	if (arg[2] == false) return 50;
	if (arg[3] == false) return 75;
	return 100;
}

NeXt::NeXt(void* arg) {
	
	this->redEvent.theEvent += Poco::delegate(&this->prTarget, &target::ReducePowerTarget::onEvent);
	//this->crTarget = new target::SendConsumptionRequestTarget(this->mb)
}

void NeXt::run() {
	using Poco::BasicEvent;
	using Poco::Delegate;
	modbus_t* mb;
	mb = modbus_new_tcp("127.0.0.1", 1502);
	this->crTarget = new target::SendConsumptionRequestTarget(mb);
	this->prResult.theEvent += Poco::delegate(this->crTarget, &target::SendConsumptionRequestTarget::onEvent);
	int s = -1;
	modbus_mapping_t *mb_mapping;
	int rc, i, header_length;
	uint8_t* query;
	query = (uint8_t*)malloc(MODBUS_MAX_ADU_LENGTH);
	header_length = modbus_get_header_length(mb);
	mb_mapping = modbus_mapping_new_start_address(
		UT_BITS_ADDRESS, UT_BITS_NB,
		UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
		UT_REGISTERS_ADDRESS, UT_REGISTERS_NB_MAX,
		UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
			modbus_strerror(errno));
		modbus_free(mb);
		return;
	}
	modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, UT_INPUT_BITS_NB,
		UT_INPUT_BITS_TAB);
	for (i = 0; i < UT_INPUT_REGISTERS_NB; i++) {
		mb_mapping->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];;
	}
	s = modbus_tcp_listen(mb, 1);
	modbus_tcp_accept(mb, &s);
	for (;;) {
		do {
			rc = modbus_receive(mb, query);
			if (query[12] == 1 || query[12] == 0) {
				if (query[12] == 1) {
					this->redEvent.InputReceived(query[10]);
					continue;
				}
				events::EvArgs args;
				args.target = mb;
				auto f = NeXt::measurekWh();
				if (f == 0) {
					args.occuredError = true;
				}
				else {
					args.occuredError = false;
				}
				args.readPower = f;
				this->prResult.InputReceived(args);
			}
			std::cout << "RC=" << rc << std::endl; //DEBUG
		} while (rc == 0);
		if (rc == -1 && errno != EMBBADCRC) break;
		if(query[header_length] == 0x03){
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
				modbus_reply_exception(mb, query,
					MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
				continue;
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE) {
				const int RAW_REQ_LENGTH = 5;
				uint8_t raw_req[] = {
					0xFF,
					0x03,
					0x02, 0x00, 0x00
				};

				printf("Reply with an invalid TID or slave\n");
				modbus_send_raw_request(mb, raw_req, RAW_REQ_LENGTH * sizeof(uint8_t));
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
				int w_s = modbus_get_socket(mb);
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

		rc = modbus_reply(mb, query, rc, mb_mapping);
		if (rc == -1) {
			break;
		}
	}
	modbus_mapping_free(mb_mapping);
	free(query);
	/* For RTU */
	modbus_close(mb);
	modbus_free(mb);
}

void NeXt::receiveMessage(const void* pSender, std::string& msg) {
	if (msg.substr(0, std::string("error").length()) == "error") {
		this->ErrorAbort = true;
		return;
	}
	if (msg.substr(0, std::string("quit").length()) == "quit") {
		this->quitRequest = true;
		return;
	}
}

float NeXt::measurekWh() {
	float erg = 0;
	//Script to read D0 here
	return 0;
}