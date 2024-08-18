/*
 * clock.h
 *
 *  Created on: Oct 9, 2023
 *      Author: Sx107
 */

#include <main.h>

#ifndef CLOCK_H_
#define CLOCK_H_

void system_init();
void _delayMs(uint32_t del);

void spi_clock_init();
void usb_clock_init();
#endif /* CLOCK_H_ */
