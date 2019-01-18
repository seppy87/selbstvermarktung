#include"workers.hpp"
#include<iostream>


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

void Netzbetreiber::run() {
	this->prEvent.theEvent += Poco::delegate(&this->redPower, &target::ReducePowerTarget::onEvent);
	std::array<bool, 4> temp;
	while (this->quitRequest == false && this->ErrorAbort == false) {
		//Lese die GPIO vom Raspberry ein!
		std::array<bool, 4> reg;
		/*
		reg = _CODE_FOR_READING_GPIO!*/
		/*
		if(reg != temp){
			auto value = this->getPercentage(reg);
			this->prEvent.InputReceived(value);
			temp = reg;
		}
		*/
		Sleep(1500);
	}
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