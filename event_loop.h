/* MIT License
 *
 * Copyright (c) 2025 Matthew Nathan Green
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, andor sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

/**@brief   Call first to initialize all services needed by the application event loop 
 */
void initialize_application(void);

/**@brief   Call to start the main event loop. All functions are processed as events by the handlers.
 *
 * @note   Does not return. 
 */
void application_main_loop(void);

#endif // EVENT_LOOP_H
