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

	class Netzbetreiber : public Poco::Runnable {
	private:
		events::PowerRequestInputEvent prEvent;
		target::ReducePowerTarget redPower;
		const std::array<USHORT, 4> input;
		const std::array<USHORT, 4> output;
		std::array<bool, 4> former = { 0,0,0,0 };
	protected:
		bool ErrorAbort = false;
		bool quitRequest = false;
	protected:
		UINT16 getPercentage(const std::array<bool, 4> &arg);
		void initPins();
	public:
		Netzbetreiber(std::array<USHORT,4> in, std::array<USHORT,4> out) : input(in) , output(out){}
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
		static float measurekWh();
		void run();
	};
}