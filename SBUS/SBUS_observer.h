/*
 * SBUS_observer.h
 *
 *  Created on: Apr 11, 2023
 *      Author: sergey
 */

#ifndef INC_SBUS_OBSERVER_H_
#define INC_SBUS_OBSERVER_H_

#include "uart_ring.h"
#include <stdint.h>

typedef enum {
	SBUS_OK,
	SBUS_SYNC_ERR,
	SBUS_END_STREAM
} SBUS_status_t;

typedef struct
{
	ring_buffer *buff;
	uint8_t buffer_index;
	uint8_t temp[25];
	volatile int _channels[15];
	volatile int channels[15];
} SBUS_observer;

typedef struct
{
	uint8_t sticks[6];
	uint8_t aux;
	uint8_t flags;
} SBUS_compressed;

SBUS_status_t SBUS_read(SBUS_observer *observer);
SBUS_status_t SBUS_read_compressed(SBUS_observer *observer, SBUS_compressed *out_buff);


#endif /* INC_SBUS_OBSERVER_H_ */
