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

void ReducePowerTarget::onEvent(const void* pSender, USHORT& arg){}