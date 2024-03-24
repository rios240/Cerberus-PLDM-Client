#ifndef PLDM_FWUP_INTERFACE_H_
#define PLDM_FWUP_INTERFACE_H_

#include "mctp/mctp_interface.h"
#include "cmd_interface/cmd_channel.h"
#include "pldm_types.h"

#define PLDM_MAX_PAYLOAD_LENGTH 512

#define MS_TIMEOUT 30000

#define SRC_ADDR 0xDA
#define SRC_EID 0xDA

#define DEST_ADDR 0xDE
#define DEST_EID 0xDE

struct multipart_transfer {
    bool8_t transfer_in_progress;
    uint32_t last_transfer_handle;
};


struct pldm_fwup_error_testing {
    bool8_t unable_to_initiate_update;
    bool8_t retry_request_update;
};

struct pldm_fwup_interface {
    bool8_t update_mode;
    uint8_t current_fd_state;
    uint8_t current_command;
    uint8_t current_completion_code;
    uint8_t pass_component_table_transfer_flag;
    struct multipart_transfer multipart_transfer;
    struct pldm_fwup_error_testing error_testing;
    uint32_t package_data_size;
    uint32_t meta_data_size;
    uint8_t *package_data;
    uint8_t *meta_data;

};

struct pldm_fwup_interface *get_fwup_interface();


int initialize_firmware_update(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                        struct cmd_interface *cmd_mctp, 
                        struct cmd_interface *cmd_spdm, struct cmd_interface *cmd_cerberus,
                        struct device_manager *device_mgr, struct pldm_fwup_interface *fwup);

uint8_t *realloc_buf(uint8_t *ptr, size_t length);

void print_bytes(uint8_t *bytes, size_t length);

int generate_and_send_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, 
                                int (*generate_pldm)(uint8_t *, size_t *));

int process_and_receive_pldm_over_mctp(struct mctp_interface *mctp, struct cmd_channel *cmd_channel, struct pldm_fwup_interface *fwup,
                                int (*process_pldm)(struct cmd_interface *, struct cmd_interface_msg *));

void firmware_update_check_state(struct pldm_fwup_interface *fwup);

void clean_up_and_reset_firmware_update(struct mctp_interface *mctp, struct pldm_fwup_interface *fwup);


#endif /* PLDM_FWUP_INTERFACE_H_ */