#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.h"
#include "pldm_fwup_commands.h"
#include "firmware_update.h"
#include "pldm_fwup_interface.h"


static struct pldm_firmware_data_transfer firmware_data_transfer = {.length = PLDM_FWUP_BASELINE_TRANSFER_SIZE, .offset = 0};


//Helper function that returns ComponentParameterTable
void get_comp_parameter_table(uint8_t *comp_parameter_table_buf, 
                        const char *active_comp_ver_str_arr, 
                        const char *pending_comp_ver_str_arr)
{

    bitfield16_t comp_activation_methods;
    comp_activation_methods.value = 0xDEAD;

    bitfield32_t capabilities_during_update;
    capabilities_during_update.value = 0;
    capabilities_during_update.bits.bit0 = 0;
    capabilities_during_update.bits.bit1 = 0;
    capabilities_during_update.bits.bit2 = 0;
    capabilities_during_update.bits.bit3 = 0;
    capabilities_during_update.bits.bit4 = 0;

    struct pldm_component_parameter_entry bios_fw_comp;
    bios_fw_comp.comp_classification = PLDM_COMP_FIRMWARE_OR_BIOS;
    bios_fw_comp.comp_identifier = 0xDEAD;
    bios_fw_comp.comp_classification_index = 0xDE;
    bios_fw_comp.active_comp_comparison_stamp = 0xDEADBEEF;
    bios_fw_comp.active_comp_ver_str_type = PLDM_STR_TYPE_ASCII;
    bios_fw_comp.active_comp_ver_str_len = strlen(active_comp_ver_str_arr);
    bios_fw_comp.active_comp_release_date[0] = 2;
    bios_fw_comp.active_comp_release_date[1] = 0;
    bios_fw_comp.active_comp_release_date[2] = 2;
    bios_fw_comp.active_comp_release_date[3] = 3;
    bios_fw_comp.active_comp_release_date[4] = 0;
    bios_fw_comp.active_comp_release_date[5] = 1;
    bios_fw_comp.active_comp_release_date[6] = 0;
    bios_fw_comp.active_comp_release_date[7] = 4;
    bios_fw_comp.pending_comp_comparison_stamp = 0xDEADF00D;
    bios_fw_comp.pending_comp_ver_str_type = PLDM_STR_TYPE_ASCII;
    bios_fw_comp.pending_comp_ver_str_len = strlen(pending_comp_ver_str_arr);
    bios_fw_comp.pending_comp_release_date[0] = 2;
    bios_fw_comp.pending_comp_release_date[1] = 0;
    bios_fw_comp.pending_comp_release_date[2] = 2;
    bios_fw_comp.pending_comp_release_date[3] = 4;
    bios_fw_comp.pending_comp_release_date[4] = 0;
    bios_fw_comp.pending_comp_release_date[5] = 1;
    bios_fw_comp.pending_comp_release_date[6] = 0;
    bios_fw_comp.pending_comp_release_date[7] = 4;
    bios_fw_comp.comp_activation_methods = comp_activation_methods;
    bios_fw_comp.capabilities_during_update = capabilities_during_update;


    memcpy(comp_parameter_table_buf, &bios_fw_comp, sizeof (struct pldm_component_parameter_entry));
    memcpy(comp_parameter_table_buf + sizeof (struct pldm_component_parameter_entry),
            active_comp_ver_str_arr, bios_fw_comp.active_comp_ver_str_len);
    memcpy(comp_parameter_table_buf + sizeof (struct pldm_component_parameter_entry)
            + bios_fw_comp.active_comp_ver_str_len, pending_comp_ver_str_arr,
            bios_fw_comp.pending_comp_ver_str_len);

}


int process_and_respond_query_device_identifiers(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_QUERY_DEVICE_IDENTIFIERS;


    uint32_t iana_number = 135;
    struct pldm_descriptor_tlv iana;
    iana.descriptor_type = PLDM_FWUP_IANA_ENTERPRISE_ID;
    iana.descriptor_length = PLDM_FWUP_IANA_ENTERPRISE_ID_LENGTH;
    memcpy(iana.descriptor_data, &iana_number, iana.descriptor_length);
    size_t iana_size = sizeof (iana.descriptor_type) + sizeof (iana.descriptor_length) + iana.descriptor_length;

    const char *uuid_string = "8d759454a78111eea5060242ac120002";
    struct pldm_descriptor_tlv uuid;
    uuid.descriptor_type = PLDM_FWUP_UUID;
    uuid.descriptor_length = PLDM_FWUP_UUID_LENGTH;
    memcpy(uuid.descriptor_data, uuid_string, uuid.descriptor_length);
    size_t uuid_size = sizeof (uuid.descriptor_type) + sizeof (uuid.descriptor_length) + uuid.descriptor_length;

    uint8_t instance_id = 1;
    uint8_t completion_code = PLDM_SUCCESS;
    uint32_t device_identifiers_len = iana_size + uuid_size;
    uint8_t descriptor_count = 2;

    uint8_t descriptors_buf[device_identifiers_len];
    memcpy(descriptors_buf, &iana, iana_size);
    memcpy(descriptors_buf + iana_size, &uuid, uuid_size);

    struct variable_field descriptors;
    descriptors.length = sizeof (descriptors_buf);
    descriptors.ptr = descriptors_buf;
    
    size_t payload_length = sizeof (struct pldm_msg_hdr) 
                + sizeof (struct pldm_query_device_identifiers_resp)
                + descriptors.length 
                + 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    int status = encode_query_device_identifiers_resp(instance_id, respMsg, 
                                    completion_code, device_identifiers_len, 
                                    descriptor_count, descriptors);

    fwup->current_completion_code = completion_code;
    return status;
    

}


int process_and_respond_get_firmware_parameters(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_GET_FIRMWARE_PARAMETERS;

    const char *active_comp_image_set_ver_str_arr = "cerberus_v1.0";
    const char *pending_comp_image_set_ver_str_arr = "cerberus_v2.0";

    bitfield32_t capabilities_during_update;
    capabilities_during_update.value = 0;
    capabilities_during_update.bits.bit0 = 0;
    capabilities_during_update.bits.bit1 = 0;
    capabilities_during_update.bits.bit2 = 0;
    capabilities_during_update.bits.bit3 = 0;
    capabilities_during_update.bits.bit4 = 0;

    struct pldm_get_firmware_parameters_resp resp_data;
    resp_data.completion_code = PLDM_SUCCESS;
    resp_data.capabilities_during_update = capabilities_during_update;
    resp_data.comp_count = 1;
    resp_data.active_comp_image_set_ver_str_type = PLDM_STR_TYPE_ASCII;
    resp_data.pending_comp_image_set_ver_str_type = PLDM_STR_TYPE_ASCII;
    resp_data.active_comp_image_set_ver_str_len = strlen(active_comp_image_set_ver_str_arr);
    resp_data.pending_comp_image_set_ver_str_len = strlen(pending_comp_image_set_ver_str_arr);

    struct variable_field active_comp_image_set_ver_str;
    active_comp_image_set_ver_str.ptr = (const uint8_t *)active_comp_image_set_ver_str_arr;
    active_comp_image_set_ver_str.length =  resp_data.active_comp_image_set_ver_str_len;

    struct variable_field pending_comp_image_set_ver_str;
    pending_comp_image_set_ver_str.ptr = (const uint8_t *)pending_comp_image_set_ver_str_arr;
    pending_comp_image_set_ver_str.length = resp_data.pending_comp_image_set_ver_str_len;

    const char *active_comp_ver_str_arr = "BIOS_v1.0";
    const char *pending_comp_ver_str_arr = "BIOS_v2.0";
    uint8_t comp_parameter_table_buf[sizeof (struct pldm_component_parameter_entry) + 
                            strlen(active_comp_ver_str_arr) +
                            strlen(pending_comp_ver_str_arr)];
    struct variable_field comp_parameter_table;
    comp_parameter_table.ptr = comp_parameter_table_buf;
    comp_parameter_table.length = sizeof (comp_parameter_table_buf);

    get_comp_parameter_table(comp_parameter_table_buf, active_comp_ver_str_arr, pending_comp_ver_str_arr);
   

    uint8_t instance_id = 1;
    size_t payload_length = sizeof (struct pldm_msg_hdr) 
                    + sizeof (struct pldm_get_firmware_parameters_resp)
                    + active_comp_image_set_ver_str.length
                    + pending_comp_image_set_ver_str.length
                    + comp_parameter_table.length 
                    + 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    int status = encode_get_firmware_parameters_resp(instance_id, respMsg, 
                                        &resp_data,
                                        &active_comp_image_set_ver_str,
                                        &pending_comp_image_set_ver_str,
                                        &comp_parameter_table);
    fwup->current_completion_code = resp_data.completion_code;
    return status;

}

int process_and_response_request_update(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_REQUEST_UPDATE;

    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);
    struct pldm_request_update_req req_data;
    struct variable_field comp_img_set_ver_str;

    int status = decode_request_update_req(reqMsg, &req_data, &comp_img_set_ver_str);

    fwup->package_data_size = req_data.pkg_data_len;
    fwup->package_data = realloc_buf(fwup->package_data, req_data.pkg_data_len);

    struct pldm_request_update_resp resp_data;

    if (req_data.pkg_data_len > 0) {
        resp_data.fd_will_send_pkg_data = 0x01;
    } else {
        resp_data.fd_will_send_pkg_data = 0x00;
    }
    resp_data.fd_meta_data_len = fwup->meta_data_size;
    
    if (fwup->current_fd_state != PLDM_FD_STATE_IDLE) {
        resp_data.completion_code = PLDM_FWUP_ALREADY_IN_UPDATE_MODE;
    }
    if (fwup->error_testing.retry_request_update == 1) {
        resp_data.completion_code = PLDM_FWUP_RETRY_REQUEST_UPDATE;
    }
    if (fwup->error_testing.unable_to_initiate_update == 1) {
        resp_data.completion_code = PLDM_FWUP_UNABLE_TO_INITIATE_UPDATE;
    }

    uint8_t instance_id = 1;
    size_t payload_length = sizeof (struct pldm_msg_hdr)
                        + sizeof (struct pldm_request_update_resp)
                        + 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    status = encode_request_update_resp(instance_id, respMsg, &resp_data);

    fwup->current_completion_code = resp_data.completion_code;
    fwup->update_mode = 1;

    return status;
}


int request_get_package_data(uint8_t *request, size_t *payload_length)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_GET_PACKAGE_DATA;

    struct pldm_multipart_transfer_req req_data;

    if (fwup->multipart_transfer.transfer_in_progress == 0) {
        req_data.transfer_operation_flag = PLDM_GET_FIRSTPART;
    } else {
        req_data.transfer_operation_flag = PLDM_GET_NEXTPART;
    }

    req_data.data_transfer_handle = fwup->multipart_transfer.last_transfer_handle;

    uint8_t instance_id = 1;
    *payload_length = sizeof (struct pldm_msg_hdr)
                + sizeof (struct pldm_multipart_transfer_req)
                + 1;
    
    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_get_package_data_req(instance_id, reqMsg, &req_data);

    return status;
}

int process_get_package_data(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();

    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    struct pldm_multipart_transfer_resp resp_data;
    struct variable_field portion_of_package_data;

    size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;
    int status = decode_get_package_data_resp(respMsg, &resp_data, &portion_of_package_data, payload_length);
    
    if (resp_data.transfer_flag == PLDM_END || resp_data.transfer_flag == PLDM_START_AND_END) {
        fwup->multipart_transfer.transfer_in_progress = 0;
    } else {
        fwup->multipart_transfer.transfer_in_progress = 1;
    }
    
    if (resp_data.transfer_flag == PLDM_START) {
        memcpy(fwup->package_data, portion_of_package_data.ptr, portion_of_package_data.length);
    } else {
        memcpy(fwup->package_data + fwup->multipart_transfer.last_transfer_handle, portion_of_package_data.ptr, 
                portion_of_package_data.length);
    }
    fwup->multipart_transfer.last_transfer_handle = resp_data.next_data_transfer_handle;

    response->length = 0;

    return status;
    
}


int process_and_respond_get_device_meta_data(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_GET_DEVICE_METADATA;
    static uint32_t portion_size = 10;

    size_t payload_length = request->length - sizeof (struct pldm_msg_hdr) - 1;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);

    struct pldm_multipart_transfer_req req_data;

    int status = decode_get_device_meta_data_req(reqMsg, payload_length, &req_data);
    if (status != 0) {
        return status;
    }

    struct pldm_multipart_transfer_resp resp_data = { 0 };
    resp_data.completion_code = PLDM_SUCCESS;
    struct variable_field portion_of_device_meta_data;
    portion_of_device_meta_data.length = portion_size;

    if ((req_data.transfer_operation_flag == PLDM_GET_FIRSTPART && fwup->multipart_transfer.transfer_in_progress == 1) ||
        (req_data.transfer_operation_flag == PLDM_GET_NEXTPART && fwup->multipart_transfer.transfer_in_progress == 0)) {
        resp_data.completion_code = PLDM_FWUP_INVALID_TRANSFER_OPERATION_FLAG;
    }
    if (req_data.transfer_operation_flag != PLDM_GET_FIRSTPART && req_data.data_transfer_handle != fwup->multipart_transfer.last_transfer_handle) {
        resp_data.completion_code = PLDM_FWUP_INVALID_TRANSFER_HANDLE;
    }
    if (fwup->meta_data_size == 0) {
        resp_data.completion_code = PLDM_FWUP_NO_DEVICE_METADATA;
    }
  
    if (req_data.transfer_operation_flag == PLDM_GET_FIRSTPART && fwup->multipart_transfer.transfer_in_progress == 0) {
        if (fwup->meta_data_size <= portion_size) {
            resp_data.transfer_flag = PLDM_START_AND_END;
        } else {
            resp_data.transfer_flag = PLDM_START;
            fwup->multipart_transfer.transfer_in_progress = 1;
        }
    }
    if (req_data.transfer_operation_flag == PLDM_GET_NEXTPART && fwup->multipart_transfer.transfer_in_progress == 1) {
        resp_data.transfer_flag = PLDM_MIDDLE;
    }
    if (fwup->multipart_transfer.last_transfer_handle + portion_size == fwup->meta_data_size) {
        resp_data.transfer_flag = PLDM_END;
        fwup->multipart_transfer.transfer_in_progress = 0;
    }

    portion_of_device_meta_data.ptr = fwup->meta_data + fwup->multipart_transfer.last_transfer_handle;
    fwup->multipart_transfer.last_transfer_handle += portion_size;
    resp_data.next_data_transfer_handle = fwup->multipart_transfer.last_transfer_handle;

    uint8_t instance_id = 1;
    payload_length = sizeof (struct pldm_msg_hdr) + sizeof (struct pldm_multipart_transfer_resp) + portion_of_device_meta_data.length + 1;
    size_t pldm_payload_length = sizeof (struct pldm_multipart_transfer_resp) + portion_of_device_meta_data.length;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);

    status = encode_get_device_meta_data_resp(instance_id, respMsg, pldm_payload_length, &resp_data, &portion_of_device_meta_data);
    fwup->current_completion_code = resp_data.completion_code;

    return status;
    

}


int process_and_respond_ua_pass_component_table(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_PASS_COMPONENT_TABLE;

    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);
    struct pldm_pass_component_table_req req_data;
    struct variable_field comp_ver_str;

    int status = decode_pass_component_table_req(reqMsg, &req_data, &comp_ver_str);
    fwup->pass_component_table_transfer_flag = req_data.transfer_flag;


    struct pldm_pass_component_table_resp resp_data;
    resp_data.comp_resp = PLDM_CR_COMP_CAN_BE_UPDATED;
    resp_data.comp_resp_code = PLDM_CRC_COMP_CAN_BE_UPDATED;
    if (fwup->update_mode != 1) {
        resp_data.completion_code = PLDM_FWUP_NOT_IN_UPDATE_MODE;
    } else if (fwup->current_fd_state != PLDM_FD_STATE_LEARN_COMPONENTS) {
        resp_data.completion_code = PLDM_FWUP_INVALID_STATE_FOR_COMMAND;
    } else if (status == PLDM_INVALID_TRANSFER_OPERATION_FLAG) {
        resp_data.comp_resp_code = PLDM_INVALID_TRANSFER_OPERATION_FLAG;
    } else {
        resp_data.completion_code = PLDM_SUCCESS;
    }

    uint8_t instance_id = 1;

    size_t payload_length = sizeof (struct pldm_msg_hdr)
                        + sizeof (struct pldm_pass_component_table_resp)
                        + 1;
    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    status = encode_pass_component_table_resp(instance_id, respMsg, &resp_data);

    fwup->current_completion_code = resp_data.completion_code;
    return status;
}

int process_and_respond_update_component(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_UPDATE_COMPONENT;

    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);
    struct pldm_update_component_req req_data;
    struct variable_field comp_ver_str;

    int status = decode_update_component_req(reqMsg, &req_data, &comp_ver_str);

    bitfield32_t update_option_flags_enabled;
    update_option_flags_enabled.value = 0xDEADBEEF;
    update_option_flags_enabled.bits.bit0 = 1;


    struct pldm_update_component_resp resp_data;
    resp_data.comp_compatibility_resp = PLDM_CCR_COMP_CAN_BE_UPDATED;
    resp_data.comp_compatibility_resp_code = PLDM_CCRC_NO_RESPONSE_CODE;
    resp_data.update_option_flags_enabled = update_option_flags_enabled;
    resp_data.time_before_req_fw_data = 0x64;
    if (fwup->update_mode != 1) {
        resp_data.completion_code = PLDM_FWUP_NOT_IN_UPDATE_MODE;
    } else {
        resp_data.completion_code = PLDM_SUCCESS;
    }

    size_t payload_length = sizeof (struct pldm_msg_hdr)
                        + sizeof (struct pldm_update_component_resp)
                        + 1;
    
    uint8_t instance_id = 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    status = encode_update_component_resp(instance_id, respMsg, &resp_data);

    fwup->current_completion_code = resp_data.completion_code;
    return status;
}

int request_firmware_data(uint8_t *request, size_t *payload_length)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    fwup->current_command = PLDM_REQUEST_FIRMWARE_DATA;

    struct pldm_request_firmware_data_req req_data;
    req_data.offset = firmware_data_transfer.offset;
    req_data.length = firmware_data_transfer.length;

    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg_hdr)
                + sizeof (struct pldm_request_firmware_data_req)
                + 1;
    
    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_request_firmware_data_req(instance_id, reqMsg, &req_data);

    firmware_data_transfer.offset += firmware_data_transfer.length;

    return status;
    
}

int process_request_firmware_data(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_fwup_interface *fwup = get_fwup_interface();
    
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    size_t payload_length = response->length - sizeof (struct pldm_msg_hdr) - 1;

    uint8_t completion_code = 0;
    struct variable_field comp_image_portion;

    int status = decode_request_firmware_data_resp(respMsg, payload_length, &completion_code, &comp_image_portion);

    fwup->current_completion_code = completion_code;
    response->length = 0;

    print_bytes((uint8_t *)comp_image_portion.ptr, comp_image_portion.length);
    return status;
}

int request_transfer_complete(uint8_t *request, size_t *payload_length)
{
    uint8_t transfer_result = PLDM_FWUP_TRANSFER_SUCCESS;
    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg) + 1;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_transfer_complete_req(instance_id, reqMsg, transfer_result);

    return status;

}

int process_transfer_complete(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    uint8_t completion_code = 0;

    int status = decode_transfer_complete_resp(respMsg, &completion_code);

    response->length = 0;

    return status;
}

int request_verify_complete(uint8_t *request, size_t *payload_length)
{
    uint8_t verify_result = PLDM_FWUP_VERIFY_SUCCESS;
    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg) + 1;

    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_verify_complete_req(instance_id, reqMsg, verify_result);
    
    return status;
}

int process_verify_complete(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    uint8_t completion_code = 0;

    int status = decode_verify_complete_resp(respMsg, &completion_code);

    response->length = 0;

    return status;

}

int request_apply_complete(uint8_t *request, size_t *payload_length)
{
    bitfield16_t comp_activation_methods_modification;
    comp_activation_methods_modification.value = 0;

    struct pldm_apply_complete_req req_data;
    req_data.apply_result = PLDM_FWUP_APPLY_SUCCESS;
    req_data.comp_activation_methods_modification = comp_activation_methods_modification;
    
    uint8_t instance_id = 1;

    *payload_length = sizeof (struct pldm_msg_hdr) 
                + sizeof (struct pldm_apply_complete_req)
                + 1;
    
    request[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *reqMsg = (struct pldm_msg *)(request + 1);
    int status = encode_apply_complete_req(instance_id, reqMsg, &req_data);

    return status;
}

int process_apply_complete(struct cmd_interface *intf, struct cmd_interface_msg *response)
{
    struct pldm_msg *respMsg = (struct pldm_msg *)(&response->data[1]);
    uint8_t completion_code = 0;

    int status = decode_apply_complete_resp(respMsg, &completion_code);

    response->length = 0;

    return status;   
}

int process_and_respond_activate_firmware(struct cmd_interface *intf, struct cmd_interface_msg *request)
{
    struct pldm_msg *reqMsg = (struct pldm_msg *)(&request->data[1]);
    struct pldm_activate_firmware_req req_data;

    int status = decode_activate_firmware_req(reqMsg, &req_data);

    struct pldm_activate_firmware_resp resp_data;
    resp_data.completion_code = PLDM_SUCCESS;
    resp_data.estimated_time_activation = 0x64;

    size_t payload_length = sizeof (struct pldm_msg_hdr)
                        + sizeof (struct pldm_activate_firmware_resp)
                        + 1;

    uint8_t instance_id = 1;

    request->length = payload_length;
    request->data[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
    struct pldm_msg *respMsg = (struct pldm_msg *)(&request->data[1]);
    status = encode_activate_firmware_resp(instance_id, respMsg, &resp_data);

    return status;

}