
#include "init_support.h"

#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "softdevice_handler.h"


#define CENTRAL_LINK_COUNT      1                               /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT   0                               /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define UART_TX_BUF_SIZE        256                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                             /**< UART RX buffer size. */

#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define VS_UUID_COUNT           4

// Initialization -----------------------------------------------------------------------------------------

void timer_init(void)
{
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
}


void uart_init(app_uart_event_handler_t uart_event_handler)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_ENABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handler,
                        APP_IRQ_PRIORITY_MID,
                        err_code);

    APP_ERROR_CHECK(err_code);
}


void buttons_leds_init(bsp_event_callback_t bsp_event_handler)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);
}

void db_discovery_init(ble_db_discovery_evt_handler_t discovery_handler)
{
    uint32_t err_code = ble_db_discovery_init(discovery_handler);
    APP_ERROR_CHECK(err_code);
}


void ble_stack_init(ble_evt_handler_t ble_event_handler)
{
    uint32_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
   
    ble_enable_params.common_enable_params.vs_uuid_count = VS_UUID_COUNT;

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_event_handler);
    APP_ERROR_CHECK(err_code);
}

void st_c_init(ble_st_c_t* sensor_tag_client, 
               ble_st_c_evt_handler_t sensor_tag_event_handler)
{
    uint32_t        err_code;
    ble_st_c_init_t st_c_init_params;

    st_c_init_params.evt_handler = sensor_tag_event_handler;
    err_code = ble_st_c_init(sensor_tag_client, &st_c_init_params);
    APP_ERROR_CHECK(err_code);
}

