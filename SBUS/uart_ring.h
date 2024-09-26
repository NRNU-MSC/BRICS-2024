/*
 * uart_ring.h
 *
 *  Created on: Apr 6, 2023
 *      Author: sergey
 */

#ifndef INC_UART_RING_H_
#define INC_UART_RING_H_
#include "main.h"
#define UART_RING_BUFFER_SIZE 1024

typedef struct
{
	UART_HandleTypeDef *huart;
	volatile uint16_t head;
	volatile uint16_t tail;
	uint8_t data[UART_RING_BUFFER_SIZE];
} ring_buffer;

void ring_buffer_init(ring_buffer *buffer);
void ring_buffer_clear(ring_buffer *buffer);
uint16_t ring_buffer_available(ring_buffer *buffer);
uint8_t ring_buffer_read(ring_buffer *buffer);
void ring_buffer_callback(ring_buffer *buffer);
void ring_buffer_restore_after_error(ring_buffer *buffer);
void ring_buffer_push(ring_buffer *buffer, uint8_t rbyte);

#endif /* INC_UART_RING_H_ */
