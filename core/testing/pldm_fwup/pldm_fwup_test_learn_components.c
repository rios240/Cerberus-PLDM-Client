#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"
#include "pldm_fwup/pldm_fwup_mctp.h"
#include "pldm_fwup/pldm_fwup_commands.h"
#include "firmware_update.h"


TEST_SUITE_LABEL ("pldm_fwup_test_learn_components");


static void pldm_fwup_test_learn_components_good_responses (CuTest *test) {

    struct mctp_interface mctp;
    struct device_manager device_mgr;
    struct cmd_interface cmd_mctp;
    struct cmd_interface cmd_spdm;
    struct cmd_interface cmd_cerberus;
    struct cmd_channel cmd_channel;
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    TEST_START;

    int status = initialize_firmware_update(&mctp, &cmd_channel, &cmd_mctp, &cmd_spdm, &cmd_cerberus, &device_mgr, fwup);
    CuAssertIntEquals(test, 0, status);

    fwup->current_fd_state = PLDM_FD_STATE_LEARN_COMPONENTS;
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_respond_query_device_identifiers);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_response_get_firmware_parameters);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);

    /*
    while (fwup->multipart_transfer.transfer_status != MPT_END) {
        status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, generate_get_package_data);
        CuAssertIntEquals(test, 0, status);

        status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, process_get_package_data);
        CuAssertIntEquals(test, 0, status);
        CuAssertIntEquals(test, PLDM_SUCCESS, fwup->completion_code);
        CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    }
    CuAssertIntEquals(test, 50, fwup->package_data_size);
    */

    clean_up_and_reset_firmware_update(&mctp, fwup);
}


TEST_SUITE_START (pldm_fwup_test_learn_components);

TEST (pldm_fwup_test_learn_components_good_responses);

TEST_SUITE_END;