#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"
#include "pldm_fwup/pldm_fwup_commands.h"
#include "firmware_update.h"


TEST_SUITE_LABEL ("pldm_fwup_test_idle");


static void pldm_fwup_test_idle_good_responses (CuTest *test) {

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

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_query_device_identifiers);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_IDLE, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_get_firmware_parameters);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_IDLE, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_response_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    clean_up_and_reset_firmware_update(&mctp, fwup);

}

static void pldm_fwup_test_idle_retry_request_update (CuTest *test) {

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

    fwup->error_testing.retry_request_update = 1;
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_response_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_IDLE, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_FWUP_RETRY_REQUEST_UPDATE, fwup->current_completion_code);

    fwup->error_testing.retry_request_update = 0;
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_response_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    clean_up_and_reset_firmware_update(&mctp, fwup);


}

static void pldm_fwup_test_idle_unable_to_initiate_update (CuTest *test) {

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

    fwup->error_testing.unable_to_initiate_update = 1;
    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_response_request_update);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_IDLE, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_FWUP_UNABLE_TO_INITIATE_UPDATE, fwup->current_completion_code);

    clean_up_and_reset_firmware_update(&mctp, fwup);


}



TEST_SUITE_START (pldm_fwup_test_idle);

TEST (pldm_fwup_test_idle_good_responses);
TEST (pldm_fwup_test_idle_retry_request_update);
TEST (pldm_fwup_test_idle_unable_to_initiate_update);

TEST_SUITE_END;