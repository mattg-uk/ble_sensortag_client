
#ifndef BLE_ST_C_H__
#define BLE_ST_C_H__

/**@file
 *
 * @brief    SensorTag Client module.
 *
 * @details  This module contains the APIs and types exposed by the SensorTag Service Client
 *           module. These APIs and types can be used by the application to perform discovery of
 *           the Nordic UART Service at the peer and interact with it.
 *
 * @note     The application must propagate BLE stack events to this module by calling
 *           ble_st_c_on_ble_evt().
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "ble.h"
#include "ble_db_discovery.h"

// The SensorTag only broadcasts the UUID of the movement service in the advertising packet
// To identify other services, you must connect.

#define BLE_UUID_ST_BASE_UUID           {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x40, 0x51, 0x04, 0x00, 0x00, 0x00, 0xf0}} /**< Used vendor specific UUID. */
#define BLE_UUID_ST_MVMT_SERVICE        0xaa80                      /**< The UUID of the SensorTag Movement Service. */
#define BLE_UUID_ST_LUXO_SERVICE        0xaa70                      /**< The UUID of the SensorTag Luxometer Service. */
#define BLE_UUID_ST_TEMP_SERVICE        0xaa00
#define BLE_UUID_ST_LX_DATA_CHRC        0xaa71                      /**< The UUID of the SensorTag Luxometer Data Characteristic. */
#define BLE_UUID_ST_LX_CONF_CHRC        0xaa72                      /**< The UUID of the SensorTag Luxometer Configuration Characteristic. */
#define BLE_UUID_ST_LX_PERI_CHRC        0xaa73                      /**< The UUID of the SensorTag Luxometer Transmission Period Characteristic. */

#define BLE_ST_CONF_CHRC_MSG_LEN        1 


/* Client event object and event types, configuration object and its associated handler ------------------------------------------------------- */

/**@brief SensorTag Client event type. */
typedef enum
{
    BLE_ST_C_EVT_DISCOVERY_COMPLETE = 1, // Event indicating that the ST services and their characteristics are resolved 
    BLE_ST_C_EVT_LX_DATA_EVT,            // Event indicating that the central has received an LX characteristic notification
    BLE_ST_C_EVT_DISCONNECTED            // Event indicating that the ST has disconnected
} ble_st_c_evt_type_t;

/**@brief SensorTag data with validity flag
*/
typedef struct 
{
    bool                valid;
    uint16_t            data;
} ble_st_c_data_t;

/**@brief Structure containing the peer event type and data after processing by the ST module. */
typedef struct {
    ble_st_c_evt_type_t evt_type;
    uint16_t            conn_handle;
    uint8_t             *p_data;
    uint8_t             data_len;
} ble_st_c_evt_t;


/**@brief Handles on the connected peer device
*/
typedef struct {
    uint16_t            luxo_data_handle;       // handle for data characteristic of LX service (light level)
    uint16_t            luxo_data_cccd_handle;  // Client Characteristic Configuration Descriptor (0x2902) 
    uint16_t            luxo_conf_handle;       // handle for conf characteristic, to switch LX service ON/OFF 
    uint16_t            luxo_peri_handle;       // handle for period characteristic, controls data reading frequency
} ble_st_c_handles_t;


typedef struct ble_st_c_s ble_st_c_t;

/**@brief   BLE SensorTag Client event handler type  
 *
 * @details This is the type of the event handler that should be provided by the application
 *          to receive events from this module. The handler takes a pointer to its parent object
 */
typedef void (* ble_st_c_evt_handler_t)(ble_st_c_t * p_ble_st_c, const ble_st_c_evt_t * p_evt);

/**@brief BLE SensorTag Client structure.
 */
struct ble_st_c_s
{
    uint8_t                 uuid_type;          /*< Base UUID type. */
    uint16_t                conn_handle;        /*< Handle of the current connection. Set with @ref ble_nus_c_handles_assign when connected. */
    ble_st_c_handles_t      handles;            /*< Handles on the connected peer device needed to interact with it. */
    ble_st_c_evt_handler_t  evt_handler;        /*< Application event handler to be called when there is an event related to the NUS. */
};


/**@brief      Add the base uuid for discovery or advertising
 *
 * @details    Contains a pre-encoded list of UUIDs to add; does this in a single step 
 *
 */
uint32_t ble_st_c_add_vs_base_uuid(ble_st_c_t * p_ble_st_c);

/**@brief SensorTag Client initialization structure.
 */
typedef struct {
    ble_st_c_evt_handler_t  evt_handler;
} ble_st_c_init_t;


/**@brief     Function for initializing the SensorTag client module.
 *
 * @details   This function registers with the Database Discovery module
 *            for the ST. Doing so will make the Database Discovery
 *            module look for the presence of a SensorTag  instance at the peer when a
 *            discovery is started.
 *
 * @param[in] p_ble_st_c        Pointer to the ST client structure.
 * @param[in] p_ble_st_c_init   Pointer to the NUS initialization structure containing the
 *                              initialization information.
 *
 * @retval    NRF_SUCCESS If the module was initialized successfully. Otherwise, an error 
 *                        code is returned. This function
 *                        propagates the error code returned by the Database Discovery module API
 *                        @ref ble_db_discovery_evt_register.
 */
uint32_t ble_st_c_init(ble_st_c_t * p_ble_st_c, ble_st_c_init_t * p_ble_st_c_init);


/**@brief     Obtain a full typed UUID from the ST client based on a short UUID 
 *
 * @param[in] p_ble_st_c        Pointer to the ST client structure.
 * @param[in] uuid              2 byte short UUID
 */
ble_uuid_t ble_st_c_get_typed_uuid(ble_st_c_t* p_ble_st_c, uint16_t uuid);

/**@brief   Function for handling events from the database discovery module.
 *
 * @details This function will handle an event from the database discovery module, and determine
 *          if it relates to the discovery of an SensorTag at the peer. If so, it will
 *          call the application's event handler indicating that ST has been
 *          discovered at the peer. It also populates the event with the service related
 *          information before providing it to the application.
 *
 * TODO     ?? This isn't really quite fully the business of the client application, and
 *          this connection / service discovery type stuff could perhaps be handled internally
 *
 * @param[in] p_ble_st_c    Pointer to the SensorTag client structure.
 * @param[in] p_evt         Pointer to the event received from the database discovery module.
 */
void ble_st_c_on_db_disc_evt(ble_st_c_t * p_ble_st_c, ble_db_discovery_evt_t * p_evt);


/**@brief     Function for handling BLE events from the SoftDevice.
 *
 * @details   This function handles the BLE events received from the SoftDevice. If a BLE
 *            event is relevant to the ST module, it is used to update
 *            internal variables and, if necessary, send events to the application.
 *
 * @param[in] p_ble_st_c    Pointer to the ST client structure.
 * @param[in] p_ble_evt     Pointer to the BLE event.
 */
void ble_st_c_on_ble_evt(ble_st_c_t * p_ble_st_c, const ble_evt_t * p_ble_evt);


/**@brief   Function for requesting the peer to start sending notifications of data characteristic.
 *
 * @details This function enables notifications of the ST LUX DATA characteristic at the peer
 *          by writing to the CCCD of the ST LUX DATA characteristic.
 *
 * @param   p_ble_st_c      Pointer to the NUS client structure.
 *
 * @retval  NRF_SUCCESS If the SoftDevice has been requested to write to the CCCD of the peer.
 *                      Otherwise, an error code is returned. This function propagates the error  
 *                      code returned by the SoftDevice API @ref sd_ble_gattc_write.
 */
uint32_t ble_st_c_lux_data_start_notify(ble_st_c_t *p_ble_st_c, bool on);


/**@brief   Function for requesting the peer to enable / disable the Luxometer. 
 *          Not only must notifications for the data chrc be enabled, but also this function
 *          must be called to physically switch the service on.
 *
 * @details This function direct writes into the ST LUX CONF chrc.
 *
 * @param   p_ble_st_c  Pointer to the ST client structure.
 *
 * @retval  NRF_SUCCESS If the SoftDevice has been requested to write to the CCCD of the peer.
 *                      Otherwise, an error code is returned. This function propagates the error  
 *                      code returned by the SoftDevice API @ref sd_ble_gattc_write.
 */
uint32_t ble_st_c_lux_conf_enable(ble_st_c_t *p_ble_st_c, bool on);

/**@brief   Helper function to switch on the Luxometer service and enable notifications. Reports
 *          errors via printf.
 *
 * @details Calls ble_st_c_lux_conf_enable and ble_st_c_lux_data_start_notify 
 *
 * @param   p_ble_st_c  Pointer to the ST client structure.
 * @param   enable      true = enable, false = disable
 */
uint32_t luxometer_enable(ble_st_c_t *p_ble_st_c, bool enable);


/**@brief   Retreive data from a SensorTag Client event
 *
 * @details Converts the data in the data buffer 
 *
 * @param   p_st_c_evt  Pointer to the ST Event
 */
ble_st_c_data_t extract_luxometer_data(const ble_st_c_evt_t * p_st_c_evt);

#endif // BLE_ST_C_H__
