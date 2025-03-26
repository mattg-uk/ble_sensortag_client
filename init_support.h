#include "app_uart.h"
#include "bsp.h"
#include "ble_db_discovery.h"
#include "ble_stack_handler_types.h"
#include "ble_sensortag_client.h"


/**@brief Function for initializing the application timer
 */
void timer_init(void);


/**@brief Function for initializing the UART.
 */
void uart_init(app_uart_event_handler_t uart_event_handler);


/**@brief Function for initializing buttons and leds.
 */
void buttons_leds_init(bsp_event_callback_t bsp_event_handler);


/** @brief Function for initializing the Database Discovery Module.
 */
void db_discovery_init(ble_db_discovery_evt_handler_t discovery_handler);


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(ble_evt_handler_t ble_event_handler);


/**@brief Function for initializing the SensorTag Client
 */
void st_c_init(ble_st_c_t* sensor_tag_client, 
               ble_st_c_evt_handler_t sensor_tag_event_handler);
