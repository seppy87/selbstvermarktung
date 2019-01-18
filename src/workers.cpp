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
	//Lese die GPIO vom Raspberry ein!
}

NeXt::NeXt(const char* _ip, uint16_t _port) {
	this->ip = std::string(_ip);
	this->port = _port;
	this->redEvent.theEvent += Poco::delegate(&this->prTarget, &target::ReducePowerTarget::onEvent);
	//this->crTarget = new target::SendConsumptionRequestTarget(this->mb)
}

void NeXt::run() {
	using Poco::BasicEvent;
	using Poco::Delegate;
	modbus_t* mb;
	mb = modbus_new_tcp(this->ip.c_str(), this->port);
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