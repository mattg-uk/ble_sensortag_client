#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ble_sensortag_client.h"

#include "ble_gattc.h"
#include "sdk_macros.h"

// ALL of these UUIDs will have the same base .type because they have a common UUID 14-byte signature

uint32_t ble_st_c_add_vs_base_uuid(ble_st_c_t * p_ble_st_c)
{
    ble_uuid128_t base_uuid = BLE_UUID_ST_BASE_UUID;
     
    VERIFY_PARAM_NOT_NULL(p_ble_st_c);
    uint32_t error_code = sd_ble_uuid_vs_add(&base_uuid, &p_ble_st_c->uuid_type);
    return error_code;
}

uint32_t ble_st_c_init(ble_st_c_t * p_ble_st_c, ble_st_c_init_t * p_ble_st_c_init)
{
    uint32_t        err_code = 0;
    ble_uuid_t      st_lx_uuid;

    VERIFY_PARAM_NOT_NULL(p_ble_st_c);
    VERIFY_PARAM_NOT_NULL(p_ble_st_c_init);
    
    ble_uuid128_t base_uuid = BLE_UUID_ST_BASE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_ble_st_c->uuid_type);
    if (err_code)
    {
        return err_code;
    }

    p_ble_st_c->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_ble_st_c->evt_handler = p_ble_st_c_init->evt_handler;
    p_ble_st_c->handles.luxo_data_handle = BLE_CONN_HANDLE_INVALID;
    p_ble_st_c->handles.luxo_conf_handle = BLE_CONN_HANDLE_INVALID;
    p_ble_st_c->handles.luxo_peri_handle = BLE_CONN_HANDLE_INVALID;

    st_lx_uuid.type = p_ble_st_c->uuid_type;
    st_lx_uuid.uuid = BLE_UUID_ST_LUXO_SERVICE;

    return ble_db_discovery_evt_register(&st_lx_uuid);
}

ble_uuid_t ble_st_c_get_typed_uuid(ble_st_c_t* p_ble_st_c, uint16_t uuid)
{
    ble_uuid_t full_uuid = { .uuid = uuid, .type = BLE_UUID_TYPE_UNKNOWN};
    if (p_ble_st_c) {
        full_uuid.type = p_ble_st_c->uuid_type;
    }
    return full_uuid;
}

void ble_st_c_on_db_disc_evt(ble_st_c_t *p_ble_st_c, ble_db_discovery_evt_t * p_evt)
{
    if (p_ble_st_c == NULL) {
        return;
    }

    ble_st_c_evt_t st_c_evt;
    memset(&st_c_evt, 0, sizeof(ble_st_c_evt_t));
    
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_ST_LUXO_SERVICE &&
        p_evt->params.discovered_db.srv_uuid.type == p_ble_st_c->uuid_type)
    {
        printf("LUXO Service discovered\n"); 
        
        p_ble_st_c->conn_handle = p_evt->conn_handle; 
        ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

        for (uint8_t i = 0; i < p_evt->params.discovered_db.char_count; ++i)
        {   
            switch (p_chars[i].characteristic.uuid.uuid)
            {
            case BLE_UUID_ST_LX_DATA_CHRC:
                p_ble_st_c->handles.luxo_data_handle = p_chars[i].characteristic.handle_value;
                p_ble_st_c->handles.luxo_data_cccd_handle = p_chars[i].cccd_handle;
                break;
            case BLE_UUID_ST_LX_CONF_CHRC:
                p_ble_st_c->handles.luxo_conf_handle = p_chars[i].characteristic.handle_value;
                break;
            case BLE_UUID_ST_LX_PERI_CHRC:
                p_ble_st_c->handles.luxo_peri_handle = p_chars[i].characteristic.handle_value;
                break;
            }
        }
        // Call the user event handler so they can take post-discovery actions  
        if (p_ble_st_c->evt_handler != NULL) 
        {
            st_c_evt.conn_handle = p_evt->conn_handle;
            st_c_evt.evt_type    = BLE_ST_C_EVT_DISCOVERY_COMPLETE;
            p_ble_st_c->evt_handler(p_ble_st_c, &st_c_evt);
        }
    }
}
/**@brief Handle Value Notification received from the softdevice
 *
 * @details     This function will check if the HVX is notification of LX DATA from the peer.
 *              Given that it is it will forward it to the application.
 *
 * @param[in]   p_ble_st_c  pointer to the ST client structure
 * @param[in]   p_ble_evt   pointer to the BLE event received
 *
*/
void on_hvx(ble_st_c_t *p_ble_st_c, const ble_evt_t *p_ble_evt)
{
    if ( (p_ble_st_c->handles.luxo_data_handle != BLE_GATT_HANDLE_INVALID)
       &&(p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_st_c->handles.luxo_data_handle)
       &&(p_ble_st_c->evt_handler != NULL) )
    {
        ble_st_c_evt_t ble_st_c_evt  = {
            .evt_type = BLE_ST_C_EVT_LX_DATA_EVT,
            .p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data,
            .data_len = p_ble_evt->evt.gattc_evt.params.hvx.len
        };

        p_ble_st_c->evt_handler(p_ble_st_c, &ble_st_c_evt);
    }
}

void ble_st_c_on_ble_evt(ble_st_c_t* p_ble_st_c, const ble_evt_t *p_ble_evt)
{
    if ((p_ble_st_c == NULL) || (p_ble_evt == NULL)) {
        return;
    }
    // Check event is relevant
    // every possible member of the evt union has a conn_handle first member
    if ( (p_ble_st_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_st_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle))
    {
        return;
    }

    // The only things that our service really cares about are handling data
    // events and forwarding disconnection events
    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GATTC_EVT_HVX:
        on_hvx(p_ble_st_c, p_ble_evt); 
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        if (   (p_ble_evt->evt.gap_evt.conn_handle == p_ble_st_c->conn_handle)
            && (p_ble_st_c->evt_handler != NULL) )
        {
            ble_st_c_evt_t st_c_evt = { .evt_type = BLE_ST_C_EVT_DISCONNECTED };
            p_ble_st_c->conn_handle = BLE_CONN_HANDLE_INVALID;
            p_ble_st_c->evt_handler(p_ble_st_c, &st_c_evt);
        }
        break; 
    }
}

static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    // So the buffer has to be a 2 byte null-terminated array 
    uint8_t buf[BLE_CCCD_VALUE_LEN];
   
    // In principal, BLE_GATT_HVX_INDICATION could also be sent
    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;
    
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}

uint32_t ble_st_c_lux_data_start_notify(ble_st_c_t *p_ble_st_c, bool on)
{
    VERIFY_PARAM_NOT_NULL(p_ble_st_c);
    if ((p_ble_st_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       || (p_ble_st_c->handles.luxo_data_cccd_handle == BLE_GATT_HANDLE_INVALID)) {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_st_c->conn_handle, p_ble_st_c->handles.luxo_data_cccd_handle, on);
}

uint32_t ble_st_c_lux_conf_enable(ble_st_c_t *p_ble_st_c, bool on)
{
    // The command for writing a one to the conf parameter is very similar to writing it to the cccd
    VERIFY_PARAM_NOT_NULL(p_ble_st_c);
    if ((p_ble_st_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       || (p_ble_st_c->handles.luxo_conf_handle == BLE_GATT_HANDLE_INVALID)) {
        return NRF_ERROR_INVALID_STATE;
    }
    
    uint8_t buf[BLE_ST_CONF_CHRC_MSG_LEN];
    buf[0] = on ? 0x01 : 0x00;
   
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_st_c->handles.luxo_conf_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };
    
    return sd_ble_gattc_write(p_ble_st_c->conn_handle, &write_params);
}

uint32_t luxometer_enable(ble_st_c_t *p_ble_st_c, bool on) 
{
    uint32_t err_code = ble_st_c_lux_data_start_notify(p_ble_st_c, on);
    if (!err_code) {
        err_code = ble_st_c_lux_conf_enable(p_ble_st_c, on);
    }
    if (!err_code) {
        printf("Luxometer configured.\n");
    } else {
        printf("Error code: %u", err_code);
    }
    return err_code;
}

ble_st_c_data_t extract_luxometer_data(const ble_st_c_evt_t * p_st_c_evt)
{
    ble_st_c_data_t value = { .valid = false}; 
    if (p_st_c_evt && p_st_c_evt->evt_type == BLE_ST_C_EVT_LX_DATA_EVT && (p_st_c_evt->data_len > 1)) {
        value.data = p_st_c_evt->p_data[1] * 256 + p_st_c_evt->p_data[0];
        value.valid = true;
    }
    return value;
}
