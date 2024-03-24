#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "testing.h"
#include "platform_io.h"
#include "pldm_fwup/pldm_fwup_interface.h"
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
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup,  process_and_respond_get_firmware_parameters);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);

    fwup->package_data_size = 50;
    fwup->package_data = realloc_buf(fwup->package_data, fwup->package_data_size);

    do {
        status = generate_and_send_pldm_over_mctp(&mctp, &cmd_channel, request_get_package_data);
        CuAssertIntEquals(test, 0, status);

        status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_get_package_data);
        CuAssertIntEquals(test, 0, status);
        CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);
        CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    } while (fwup->multipart_transfer.transfer_in_progress != 0);
    
    printf("Received the following Package Data: \n");
    print_bytes(fwup->package_data, fwup->package_data_size);

    
    fwup->multipart_transfer.last_transfer_handle = 0;
    fwup->multipart_transfer.transfer_in_progress = 0;

    printf("Sending the following Device  MetaData: \n");
    print_bytes(fwup->meta_data, (size_t)fwup->meta_data_size);
    
    do {
        status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_get_device_meta_data);
        CuAssertIntEquals(test, 0, status);
        CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);
        CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);
    } while (fwup->multipart_transfer.transfer_in_progress != 0);
    


    clean_up_and_reset_firmware_update(&mctp, fwup);


}


static void pldm_fwup_test_learn_components_good_pass_component_table(CuTest *test) {
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
    fwup->current_fd_state = PLDM_FD_STATE_LEARN_COMPONENTS;

    do {
        status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_ua_pass_component_table);
        CuAssertIntEquals(test, 0, status);
        CuAssertIntEquals(test, PLDM_SUCCESS, fwup->current_completion_code);
    } while (fwup->pass_component_table_transfer_flag == PLDM_START ||fwup->pass_component_table_transfer_flag == PLDM_MIDDLE);
    CuAssertIntEquals(test, PLDM_FD_STATE_READY_XFER, fwup->current_fd_state);

     clean_up_and_reset_firmware_update(&mctp, fwup);

}

static void pldm_fwup_test_learn_components_bad_pass_component_table(CuTest *test) {
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

    fwup->update_mode = 0;
    fwup->current_fd_state = PLDM_FD_STATE_LEARN_COMPONENTS;

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_ua_pass_component_table);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FWUP_NOT_IN_UPDATE_MODE, fwup->current_completion_code);
    CuAssertIntEquals(test, PLDM_FD_STATE_LEARN_COMPONENTS, fwup->current_fd_state);

    fwup->update_mode = 1;
    fwup->current_fd_state = PLDM_FD_STATE_IDLE;

    status = process_and_receive_pldm_over_mctp(&mctp, &cmd_channel, fwup, process_and_respond_ua_pass_component_table);
    CuAssertIntEquals(test, 0, status);
    CuAssertIntEquals(test, PLDM_FWUP_INVALID_STATE_FOR_COMMAND, fwup->current_completion_code);
    CuAssertIntEquals(test, PLDM_FD_STATE_IDLE, fwup->current_fd_state);

    clean_up_and_reset_firmware_update(&mctp, fwup);

}



TEST_SUITE_START (pldm_fwup_test_learn_components);

TEST (pldm_fwup_test_learn_components_good_responses);
TEST (pldm_fwup_test_learn_components_good_pass_component_table);
TEST (pldm_fwup_test_learn_components_bad_pass_component_table);

TEST_SUITE_END;