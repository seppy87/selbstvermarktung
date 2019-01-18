#pragma once

#include"targets.hpp"
#include"events.hpp"
#include<array>

namespace workers {

	class basic : public Poco::Runnable{
	public:
		virtual void receiveMessage(std::string msg) = 0;
		virtual void run() = 0;
	};

	class Netzbetreiber : public basic {
	private:
		events::PowerRequestInputEvent prEvent;
		target::ReducePowerTarget redPower;
	protected:
		bool ErrorAbort = false;
		bool quitRequest = false;
	protected:
		UINT16 getPercentage(const std::array<bool, 4> &arg);
	public:
		void receiveMessage(const void* pSender, std::string& msg);
		void run();
	};

	class NeXt : public Poco::Runnable {
	private:
		bool ErrorAbort = false;
		bool quitRequest = false;
		events::PowerMeassureResultEvent prResult;
		target::SendConsumptionRequestTarget* crTarget;

		events::PowerRequestInputEvent redEvent;
		target::ReducePowerTarget prTarget;
	
	public:
		NeXt(void* arg);
		void receiveMessage(const void* pSender, std::string& msg);
		void run();
	};
}