
#include "event_loop.h"
#include "init_support.h"
#include "scan_support.h"

#include "bsp_btn_ble.h"
#include "ble_hci.h"

void on_ble_gap_evt(ble_evt_t * p_ble_evt);

static ble_db_discovery_t       m_ble_db_discovery;
static ble_st_c_t               m_ble_sensortag_client;


// Main Application ----------------------------------------------------------------------------------------------------

void application_main_loop(void)
{
    scan_start(); 
    while (true) {
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}

// Event handlers: UART events -----------------------------------------------------------------------------------------

/**@brief       Function for handling UART events.
 *
 * @details     This handler will receive single bytes from the UART as and when they are ready.
 *              They are currently consumed without use; the UART is 'write only'
 */
void uart_event_handler(app_uart_evt_t * p_event)
{
    uint8_t data;

    switch (p_event->evt_type)
    {
        /**@snippet Simply consume each byte to prevent overflows */ 
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data));
            break;
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
        default:
            break;
    }
}

// Event handlers: BLE events -----------------------------------------------------------------------------------------

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *          been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{   
    // Forward to the middleware: Nordic stack BSP to turn the indication blink / solid
    bsp_btn_ble_on_ble_evt(p_ble_evt);

    // Forward to the middleware: Provided by the Nordic stack to process discovery events
    //  In turn: This middleware makes its own callback to the application
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);

    // Forward to the application: process BLE GAP events
    on_ble_gap_evt(p_ble_evt);

    // Forward to the application: Send TO Sensor Tag client
    ble_st_c_on_ble_evt(&m_ble_sensortag_client, p_ble_evt);
}


/**@brief   Process the GAP events from the BLE Stack.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void on_ble_gap_evt(ble_evt_t * p_ble_evt)
{
    uint32_t              err_code;

    const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
            const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
            ble_uuid_t target_uuid = { .uuid = BLE_UUID_ST_MVMT_SERVICE, .type = BLE_UUID_TYPE_UNKNOWN };
            if (is_uuid_present(&target_uuid, p_adv_report)) {
                connect_peer(&p_adv_report->peer_addr);
            }
            break;
        }

        case BLE_GAP_EVT_CONNECTED:
            printf("[GAP]: Connected to target\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            err_code = ble_db_discovery_start(&m_ble_db_discovery, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN) {
                printf("[GAP]: Scan timed out.\r\n");
            }
            else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
                printf("[GAP]: Connection Request timed out.\r\n");
            }
            scan_start();
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            printf("[GAP]: Received sec params request: Not supported.\r\n");
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            printf("[GAP]: Connection parameter update request");
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;
    
        default:
            break;
    }
}

/**@brief   Process events received FROM the SensorTag Client 
 *
 * @details This function processes the 'user events' from the client. The client handles the 
 *          details of the individual services, chrcs, etc being discovered, and receives
 *          the data, but this user function is what takes the final action. */
void ble_st_c_evt_handler( ble_st_c_t * p_ble_st_c, const ble_st_c_evt_t * p_st_c_evt)
{
    switch(p_st_c_evt->evt_type)
    {
        case BLE_ST_C_EVT_LUXO_READY:
            luxometer_enable(p_ble_st_c, true); 
            break;
        case BLE_ST_C_EVT_TEMP_READY:
            break;
        case BLE_ST_C_EVT_LX_DATA_EVT:
            ble_st_c_data_t value = extract_luxometer_data(p_st_c_evt); 
            if (value.valid) {
                printf("Lux value: %d\n", value.data);
            }
            break;
        case BLE_ST_C_EVT_DISCONNECTED:
            printf("Disconnected!\n");
            scan_start();
            break;
        default:
            break;
    }
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_st_c_on_db_disc_evt(&m_ble_sensortag_client, p_evt);
}


// Event handlers: Hardware events (buttons and inactivity timers --------------------------------------------------------------------------------

static void sleep_mode_enter(void);

void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;
        
        // If a disconnect hardware button is pressed, disconnect and inform the remote user:
        // From their perspective, we are the remote user that is terminating the connection
        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_ble_sensortag_client.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

/**@brief Function for putting the chip into sleep mode.
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

// Initialization ---------------------------------------------------------------------------------
// Located at the end for declarative convenience

void initialize_application()
{
    timer_init();
    uart_init(uart_event_handler);
    buttons_leds_init(bsp_event_handler);
    db_discovery_init(db_disc_handler);
    ble_stack_init(ble_evt_dispatch);
    st_c_init(&m_ble_sensortag_client, ble_st_c_evt_handler);
}
