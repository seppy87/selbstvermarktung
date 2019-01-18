#pragma once

#include"targets.hpp"
#include"events.hpp"

namespace workers {

	class basic : public Poco::Runnable{
	public:
		virtual void receiveMessage(std::string msg) = 0;
		virtual void run() = 0;
	};

	class Netzbetreiber : public basic {
	protected:
		bool ErrorAbort = false;
		bool quitRequest = false;
	public:
		void receiveMessage(const void* pSender, std::string& msg);
		void run();
	};

	class NeXt : public Poco::Runnable {
	private:
		bool ErrorAbort = false;
		bool quitRequest = false;
		std::string ip;
		uint16_t port;
		events::PowerMeassureResultEvent prResult;
		target::SendConsumptionRequestTarget* crTarget;

		events::PowerRequestInputEvent redEvent;
		target::ReducePowerTarget prTarget;
	public:
		NeXt(const char* _ip, uint16_t _port);
		void receiveMessage(const void* pSender, std::string& msg);
		void run();
	};
}