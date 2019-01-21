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
	mb = modbus_new_tcp(config::config::ip.c_str(), config::config::port);
	this->crTarget = new target::SendConsumptionRequestTarget(mb);
	this->prResult.theEvent += Poco::delegate(this->crTarget, &target::SendConsumptionRequestTarget::onEvent);
	modbus_connect(mb);
	uint16_t reg[32];
	while (this->ErrorAbort == false && this->quitRequest == false) {
		int rc = modbus_read_registers(mb, 0, 10, reg);
		if (rc < 0) {
			std::cout << "ERROR OCCURED" << std::endl;
			break;
		}
		if (reg[12] == 1) {
			// read input
			events::EvArgs args;
			args.target = mb;
			this->prResult.InputReceived(args);
		}
		Sleep(2000);
	}
	modbus_close(mb);
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