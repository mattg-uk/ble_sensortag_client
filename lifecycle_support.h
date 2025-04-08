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

#ifndef LIFECYCLE_SUPPORT_H
#define LIFECYCLE_SUPPORT_H

#include "app_uart.h"
#include "bsp.h"
#include "ble_db_discovery.h"
#include "ble_stack_handler_types.h"
#include "ble_sensortag_client.h"


/**@brief Function for initializing the application timer
 */
void timer_init(void);


/**@brief Function for initializing the UART.
 *
 * @param[in] uart_event_handler        event loop function to handle UART events 
 */
void uart_init(app_uart_event_handler_t uart_event_handler);


/**@brief Function for initializing buttons and leds.
 *
 * @param[in] bsp_event_handler        event loop function to handle 'hardware' events 
 */
void buttons_leds_init(bsp_event_callback_t bsp_event_handler);


/**@brief Function for initializing the Database Discovery Module.
 *
 * @param[in] discovery_handler        event loop function to handle discovery events 
 */
void db_discovery_init(ble_db_discovery_evt_handler_t discovery_handler);


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 * @param[in] ble_event_handler        event loop function to handle GAP events 
 */
void ble_stack_init(ble_evt_handler_t ble_event_handler);


/**@brief Function for initializing the SensorTag Client
 * 
 * @param[in] sensor_tag_client         client structure to be initialized
 * @param[in] sensor_tag_event_handler  event loop function to handle CLIENT events 
 */
void st_c_init(st_client_t* sensor_tag_client, 
               st_client_evt_handler_t sensor_tag_event_handler);


/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice -
 *          effectively resets the device.
 * 
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);


/**@brief Function for putting the chip into sleep mode.
 * @note This function will not return.
 */
void sleep_mode_enter(void);

#endif // LIFECYCLE_SUPPORT_H
