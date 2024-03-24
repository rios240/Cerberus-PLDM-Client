#ifndef PLDM_FWUP_COMMAND_CHANNEL_H_
#define PLDM_FWUP_COMMAND_CHANNEL_H_


#include <stdint.h>
#include "cmd_interface/cmd_channel.h"
#include "cmd_interface/cmd_interface.h"

int send_packet(struct cmd_channel *channel, struct cmd_packet *packet);
int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout);

int generate_error_packet(struct cmd_interface *intf, struct cmd_interface_msg *request,
		uint8_t error_code, uint32_t error_data, uint8_t cmd_set);

#endif /* PLDM_FWUP_COMMAND_CHANNEL_H_ */