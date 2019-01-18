#pragma once

#include"events.hpp"
#include<Poco/Runnable.h>

namespace target {
	class Target {
	public:
		virtual void onEvent(const void* pSender, USHORT& arg) = 0;
	};

	class ReducePowerTarget : public Target {
	private:
		//
	public:

		void onEvent(const void* pSender, USHORT& arg);
	};

	class SendConsumptionRequestTarget{
	private:
		modbus_t* mb;
	public:
		SendConsumptionRequestTarget(modbus_t* connected) {
			mb = connected;
		}

		void onEvent(const void* pSender, events::EvArgs& arg);
	};
}
