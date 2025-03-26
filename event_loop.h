
/**@brief   Call first to initialize all services needed by the application event loop 
 */
void initialize_application(void);

/**@brief   Call to start the main event loop. All functions are processed as events by the handlers.
 *
 * @note   Does not return. 
 */
void application_main_loop(void);




