/* MIT License
 *
 * Copyright (c) 2025 Matthew Nathan Green
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, andor sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ble_sensortag_client.h"

#include "ble_gattc.h"
#include "sdk_macros.h"

// Specific default service definitions

static st_client_svc_t Luxometer = {     .uuid = BLE_UUID_ST_LUXO_SERVICE,
                                        .name = "LUXO",
                                        .events = { .discovered = ST_CLIENT_EVT_LUXO_DISCOVERED,
                                                    .data_ready = ST_CLIENT_EVT_LUXO_DATA},
                                  };

static st_client_svc_t Temperature = {   .uuid = BLE_UUID_ST_TEMP_SERVICE,
                                        .name = "TEMP",
                                        .events = { .discovered = ST_CLIENT_EVT_TEMP_DISCOVERED,
                                                    .data_ready = ST_CLIENT_EVT_TEMP_DATA},
                                    }; 

static const st_client_svc_t * default_services[] = { &Temperature, &Luxometer };


// Helper functions 

ble_uuid_t st_client_get_typed_uuid(st_client_t* p_client, uint16_t uuid)
{
    ble_uuid_t full_uuid = { .uuid = uuid, .type = BLE_UUID_TYPE_UNKNOWN};
    if (p_client) {
        full_uuid.type = p_client->uuid_type;
    }
    return full_uuid;
}

static st_client_svc_t* st_client_get_service(st_client_t *p_client, uint16_t uuid) {
    st_client_svc_t* service = p_client->services;
    while (service < p_client->services + p_client->service_count &&
           service->uuid != uuid) {
        ++service;
    }
    if (service == p_client->services + p_client->service_count)
        service = NULL;
    return service;
}

uint32_t st_clientheck_service(st_client_t *p_client, st_client_svc_t* p_service)
{   
    VERIFY_PARAM_NOT_NULL(p_client);
    VERIFY_PARAM_NOT_NULL(p_service);

    if (p_client->conn_handle == BLE_CONN_HANDLE_INVALID)
        return NRF_ERROR_INVALID_STATE;
   
    for (int i = 0; i < sizeof(p_service->handles) / sizeof(p_service->handles[0]); ++i) {
        if (p_service->handles[i] == BLE_CONN_HANDLE_INVALID) {
            return NRF_ERROR_INVALID_STATE;
        }
    }
    return NRF_SUCCESS;
}

// Initialization

uint32_t st_client_init(st_client_t * p_client, st_client_init_t * p_client_init)
{
    uint32_t        err_code = 0;
    ble_uuid_t      st_service_uuid;

    VERIFY_PARAM_NOT_NULL(p_client);
    VERIFY_PARAM_NOT_NULL(p_client_init);
    VERIFY_PARAM_NOT_NULL(p_client_init->evt_handler);

    ble_uuid128_t base_uuid = BLE_UUID_ST_BASE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_client->uuid_type);
    if (err_code)
    {
        return err_code;
    }

    p_client->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_client->evt_handler = p_client_init->evt_handler;
    
    const uint8_t total_services = sizeof(p_client->services) / sizeof(st_client_svc_t);
    static_assert(total_services == sizeof(default_services) / sizeof(st_client_svc_t*));
    p_client->service_count = total_services;

    for (uint8_t index = 0; index < total_services && !err_code; ++index) {
        st_client_svc_t* service = &p_client->services[index]; 
        *service = *default_services[index];

        st_service_uuid.uuid = service->uuid;
        st_service_uuid.type = p_client->uuid_type;
        err_code = ble_db_discovery_evt_register(&st_service_uuid);
    }
    printf("initialization complete: code %lx\n", err_code);

    return err_code;
}

// Discovery - must must placed in the discovery dispatcher

void st_client_on_db_disc_evt(st_client_t *p_client, ble_db_discovery_evt_t * p_evt)
{
    if (p_client == NULL) {
        return;
    }
    
    switch (p_evt->evt_type) {

    case BLE_DB_DISCOVERY_COMPLETE:
        uint16_t service_uuid = p_evt->params.discovered_db.srv_uuid.uuid;
        printf("[GATT] Service discovered: %x\n", service_uuid);
        if (p_evt->params.discovered_db.srv_uuid.type != p_client->uuid_type) {
            return;
        }
        
        p_client->conn_handle = p_evt->conn_handle; 
        st_client_evt_t st_c_evt;
        memset(&st_c_evt, 0, sizeof(st_client_evt_t));
       
        st_client_svc_t* service = st_client_get_service(p_client, service_uuid);
        if (service == NULL)
            return;
        
        printf("\tService is: %s\n", service->name); 
        
        ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

        for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; ++i)
        {   
            uint16_t chrc_uuid = p_chars[i].characteristic.uuid.uuid;
            uint16_t offset = chrc_uuid - service_uuid;
            switch (offset)
            {
            case DATA_UUID_OFFSET:
                service->handles[DATA] = p_chars[i].characteristic.handle_value;
                service->handles[DATA_CCCD] = p_chars[i].cccd_handle;
                break;
            case CONF_UUID_OFFSET:
                service->handles[CONF] = p_chars[i].characteristic.handle_value;
                break;
            case PERI_UUID_OFFSET:
                service->handles[PERI] = p_chars[i].characteristic.handle_value;
                break;
            default:
                printf("[GATT] Service %x discarded characteristic %x", service_uuid, chrc_uuid);
                break;
            }
        }
        // Call the user event handler so they can take post-discovery actions  
        if (p_client->evt_handler != NULL) 
        {
            st_c_evt.conn_handle = p_evt->conn_handle;
            st_c_evt.evt_type    = service->events.discovered;
            p_client->evt_handler(p_client, &st_c_evt);
        }
        break;
    case BLE_DB_DISCOVERY_SRV_NOT_FOUND:
        printf("Could not find the service!\n");
        break;
    case BLE_DB_DISCOVERY_AVAILABLE:
        printf("Discovery available\n");
    default:
        break;
    }
}

// BLE events post-discovery - st_client_on_ble_evt must be placed in the Application BLE dispatcher

/**@brief Handle Value Notification received from the softdevice
 *
 * @details     This function will check if the HVX is notification of LX DATA from the peer.
 *              Given that it is it will forward it to the application.
 *
 * @param[in]   p_client    pointer to the ST client structure
 * @param[in]   p_ble_evt   pointer to the BLE event received
 *
*/
static void on_hvx(st_client_t *p_client, const ble_evt_t *p_ble_evt)
{
    // Confirm and select relevant service 
    uint16_t hvx_handle = p_ble_evt->evt.gattc_evt.params.hvx.handle;
    st_client_svc_t* service = p_client->services;
    while (service < p_client->services + p_client->service_count &&
           service->handles[DATA] != hvx_handle) {
        ++service;
    }
    if (service == p_client->services + p_client->service_count)
        return;

    st_client_evt_t hvx_data_event  = {
        .evt_type = service->events.data_ready,
        .p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data,
        .data_len = p_ble_evt->evt.gattc_evt.params.hvx.len
    };
    p_client->evt_handler(p_client, &hvx_data_event);
}

void st_client_on_ble_evt(st_client_t* p_client, const ble_evt_t *p_ble_evt)
{
    // Check client is valid and event is relevant
    if ((p_client == NULL) || (p_ble_evt == NULL) || 
       (p_client->evt_handler == NULL) ||
       (p_client->conn_handle == BLE_CONN_HANDLE_INVALID) ||
       (p_client->conn_handle != p_ble_evt->evt.gap_evt.conn_handle))
    {
        return;
    }

    // The only things that our services really care about are data and disconnect
    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GATTC_EVT_HVX:
        on_hvx(p_client, p_ble_evt); 
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        st_client_evt_t client_disconnect_event = { .evt_type = ST_CLIENT_EVT_DISCONNECTED };
        p_client->conn_handle = BLE_CONN_HANDLE_INVALID;
        p_client->evt_handler(p_client, &client_disconnect_event);
        break; 
    }
}

// Application - used in the User's Application code

uint32_t st_client_data_notify(st_client_t *p_client, uint16_t service_uuid, bool enable)
{
    st_client_svc_t* service = st_client_get_service(p_client, service_uuid);
    VERIFY_SUCCESS(st_clientheck_service(p_client, service));
    
    uint8_t buf[BLE_CCCD_VALUE_LEN];
   
    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;
    
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = service->handles[DATA_CCCD],
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(p_client->conn_handle, &write_params); 
}

uint32_t st_client_conf_enable(st_client_t *p_client, uint16_t service_uuid, bool enable)
{
    st_client_svc_t* service = st_client_get_service(p_client, service_uuid);
    VERIFY_SUCCESS(st_clientheck_service(p_client, service));
    
    uint8_t buf[CONF_CHRC_MSG_LEN];
    buf[0] = enable ? 0x01 : 0x00;
   
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = service->handles[CONF],
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };
    
    return sd_ble_gattc_write(p_client->conn_handle, &write_params);
}

uint32_t service_enable(st_client_t *p_client, uint16_t service_uuid, bool enable) 
{
    VERIFY_SUCCESS(st_client_data_notify(p_client, service_uuid, enable));
    VERIFY_SUCCESS(st_client_conf_enable(p_client, service_uuid, enable));
    
    return NRF_SUCCESS;
}

// User helper functions to decode the ST data packets

st_client_data_t extract_luxometer_data(const st_client_evt_t * p_st_c_evt)
{
    st_client_data_t value = { .valid = false}; 
    if (p_st_c_evt && 
        p_st_c_evt->evt_type == ST_CLIENT_EVT_LUXO_DATA && 
        (p_st_c_evt->data_len > 1)) 
    {
        value.luxo_data = *(uint16_t*)p_st_c_evt->p_data;
        value.valid = true;
    }
    return value;
}

st_client_data_t extract_temperature_data(const st_client_evt_t * p_st_c_evt)
{
    st_client_data_t value = { .valid = false}; 
    if (p_st_c_evt && 
        p_st_c_evt->evt_type == ST_CLIENT_EVT_TEMP_DATA &&
        p_st_c_evt->data_len == 4) 
    {
        uint8_t *source = p_st_c_evt->p_data; 
        // four-bytes data field, made up of two SIGNED 16 bit ints.
        // The scaling factor 'ECG patch' is 0.0078125.
        int16_t ir    = *(int16_t*)(&source[0]);
        int16_t amb   = *(int16_t*)(&source[2]);
        value.temp_data.ir_data = 0.0078125 * ir;
        value.temp_data.amb_data= 0.0078125 * amb;
        value.valid = true;
    }
    return value;
}
