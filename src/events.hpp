#pragma once


#include<Poco/Thread.h>
#include<Poco/ThreadPool.h>
#include<Poco/BasicEvent.h>
#include<Poco/Delegate.h>
#include<modbus.h>
#include"config.hpp"


namespace events {
	class PowerRequestInputEvent {
	public:
		Poco::BasicEvent<USHORT> theEvent;
		
		void InputReceived(USHORT n) {
			theEvent(this, n);
		}
	};

	struct EvArgs {
		modbus_t* target;
		unsigned short readPower;
		bool occuredError;
		char* ErrorMessage;
	};

	class PowerMeassureResultEvent {
	public:
		Poco::BasicEvent<EvArgs> theEvent;

		void InputReceived(EvArgs n) {
			theEvent(this, n);
		}
	};
}
