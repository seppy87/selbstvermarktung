#include"targets.hpp"
#include"workers.hpp"

using namespace target;

void SendConsumptionRequestTarget::onEvent(const void* pSender, events::EvArgs& arg) {
	uint16_t reg[32];
	reg[0] = arg.readPower;
	if (arg.occuredError)
		reg[6] = 0;
	else
		reg[6] = 1;
	int rc = modbus_write_registers(mb, 0, 10, reg);
	if (rc < 0)
		((workers::basic*)pSender)->receiveMessage(arg.ErrorMessage);
}

void ReducePowerTarget::onEvent(const void* pSender, USHORT& arg)
{
#ifdef _RPi
	switch (arg) {
	case 100:
		digitalWrite(config::config::output[3], 1);
	case 75:
		digitalWrite(config::config::output[2], 1);
	case 50:
		digitalWrite(config::config::output[1], 1);
	case 25:
		digitalWrite(config::config::output[0], 1);
		break;
	default:
		digitalWrite(config::config::output[3], 0);
		digitalWrite(config::config::output[2], 0);
		digitalWrite(config::config::output[1], 0);
		digitalWrite(config::config::output[0], 0);
	}
#endif
}