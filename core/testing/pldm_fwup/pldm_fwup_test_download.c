#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"
#include "pldm_fwup/pldm_fwup_commands.h"
#include "firmware_update.h"


TEST_SUITE_LABEL ("pldm_fwup_test_download");


static void pldm_fwup_test_download_good_request_firmware_data(CuTest *test) {

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

    fwup->update_mode = 1;
    fwup->current_fd_state = PLDM_FD_STATE_DOWNLOAD;

    /*
    int loop = 3;
    while (loop != 0) {
        status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, request_firmware_data);
        CuAssertIntEquals(test, 0, status);

        status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_request_firmware_data);
        CuAssertIntEquals(test, 0, status);
        CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);
        loop--;
    }
    */

    clean_up_and_reset_firmware_update(&mctp, fwup);
}


TEST_SUITE_START (pldm_fwup_test_download);

TEST (pldm_fwup_test_download_good_request_firmware_data);

TEST_SUITE_END;