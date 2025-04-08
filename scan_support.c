/* Copyright Matthew Nathan Green 2025 */

/* This file is partly derived from the Nordic SDK and must only be  */
/* redistributed, modified or unmodified, with the Nordic copyright notice  */
/* intact and is ONLY FOR USE ON A NORDIC SEMICONDUCTOR device. */

/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "scan_support.h"

#include "app_util.h"
#include "bsp.h"
#include "ble.h"
#include "ble_gap.h"


#define SCAN_INTERVAL           0x00A0                          /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW             0x0050                          /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_ACTIVE             1                               /**< If 1, performe active scanning (scan requests). */
#define SCAN_SELECTIVE          0                               /**< If 1, ignore unknown devices (non whitelisted). */
#define SCAN_TIMEOUT            0x0000                          /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS) /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                               /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Determines supervision time-out in units of 10 millisecond. */

#define UUID16_SIZE             2                               /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                               /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                              /**< Size of 128 bit UUID */
#define VS_UUID_COUNT           4


/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
  {
    (uint16_t)MIN_CONNECTION_INTERVAL,  // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,  // Maximum connection
    (uint16_t)SLAVE_LATENCY,            // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT       // Supervision time-out
  };

/**
 * @brief Parameters used when scanning.
 */
static const ble_gap_scan_params_t m_scan_params = 
  {
    .active      = SCAN_ACTIVE,
    .selective   = SCAN_SELECTIVE,
    .p_whitelist = NULL,
    .interval    = SCAN_INTERVAL,
    .window      = SCAN_WINDOW,
    .timeout     = SCAN_TIMEOUT
  };


void scan_start(void)
{
    uint32_t err_code;
    
    err_code = sd_ble_gap_scan_start(&m_scan_params);
    APP_ERROR_CHECK(err_code);
    
    err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
    APP_ERROR_CHECK(err_code);
}

void connect_peer(const ble_gap_addr_t* p_gap_address)
{
    uint32_t              err_code;
    err_code = sd_ble_gap_connect(p_gap_address,
                                  &m_scan_params,
                                  &m_connection_param);

    // NRF_SUCCESS == 'connecting' not 'connected' 
    if (err_code == NRF_SUCCESS)
    {
        // scan is automatically stopped by the connect
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        
        APP_ERROR_CHECK(err_code);
        printf("Connecting to target %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                 p_gap_address->addr[0],
                 p_gap_address->addr[1],
                 p_gap_address->addr[2],
                 p_gap_address->addr[3],
                 p_gap_address->addr[4],
                 p_gap_address->addr[5]
                 );
    }

}

bool compare_uuid(uint8_t *p_data, const ble_uuid_t* p_target_uuid, uint8_t index, uint8_t field_length, uint8_t stride_size, uint8_t decode_size)
{
    ble_uuid_t extracted_uuid; 
    uint32_t error_code;

    for (uint32_t u_index = 0; u_index < (field_length/stride_size); u_index++)
    {
        error_code = sd_ble_uuid_decode(decode_size, 
                                        &p_data[u_index * stride_size + index + 2], 
                                        &extracted_uuid);
        if (error_code == NRF_SUCCESS)
        {  
            if ((extracted_uuid.uuid == p_target_uuid->uuid)
                && (extracted_uuid.type == BLE_UUID_TYPE_BLE ))
            {
                return true;
            }
        }
    }
    return false;
}

bool is_uuid_present(const ble_uuid_t *p_target_uuid, 
                     const ble_gap_evt_adv_report_t *p_adv_report)
{
    uint32_t index = 0;
    uint8_t *p_data = (uint8_t *)p_adv_report->data;
    while (index < p_adv_report->dlen)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index+1];
        bool found = false;

        switch (field_type) {
            case BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE:
            case BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE:
                found = compare_uuid(p_data, p_target_uuid, index, field_length, 
                                     UUID16_SIZE, UUID16_SIZE);
                break;
            case BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE:
            case BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_COMPLETE:
                found = compare_uuid(p_data, p_target_uuid, index, field_length, 
                                     UUID32_SIZE, UUID16_SIZE);
                break;
            case BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE:
            case BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE:
                found = compare_uuid(p_data, p_target_uuid, index, field_length, 
                                     UUID128_SIZE, UUID128_SIZE);
                break;
            case BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME:
            case BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME:
                printf("Local name: ");
                for (int i = index + 2; i < index + field_length + 1; ++i) {
                    putc(p_data[i], stdout);
                }
                putc('\n', stdout);
                break;
        }
        if (found) {
            return true;
        }
        // The field_length byte does not contribute to the field_length value, the field_type byte does
        index += field_length + 1;
    }
    return false;
}
