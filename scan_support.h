
#include "ble_gap.h"

/**@brief   Begins scanning for connection targets.
 *
 * @details Scan parameters are configured in the .c file
 * 
 */
void scan_start(void);


/**@brief   Attempts to connect to the target.
 *
 * @details Connection parameters are configured in the .c file
 * 
 * @param[in]   p_gap_address The connection target address
 *
 */
void connect_peer(const ble_gap_addr_t* p_gap_address);


/**@brief Reads an advertising report and checks if a uuid is present in the service list.
 *
 * @details The function is able to search for 16-bit, 32-bit and 128-bit service uuids. 
 *          To see the format of a advertisement packet, see 
 *          https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
 *
 * @param[in]   p_target_uuid The uuid to search fir
 * @param[in]   p_adv_report  Pointer to the advertisement report.
 *
 * @retval      true if the UUID is present in the advertisement report. Otherwise false  
 */
bool is_uuid_present(const ble_uuid_t *p_target_uuid, 
                            const ble_gap_evt_adv_report_t *p_adv_report);
