/*
 * uart_ring.c
 *
 *  Created on: Apr 6, 2023
 *      Author: sergey
 */

#include "uart_ring.h"
#include "main.h"
#include "stm32f7xx.h"

void ring_buffer_init(ring_buffer *buffer) {
	ring_buffer_clear(buffer);
}

void ring_buffer_clear(ring_buffer *buffer) {
	buffer->head = 0;
	buffer->tail = 0;
}

uint16_t ring_buffer_available(ring_buffer *buffer){
	return ((uint16_t)(UART_RING_BUFFER_SIZE + buffer->head - buffer->tail)) % UART_RING_BUFFER_SIZE;
}
uint8_t ring_buffer_read(ring_buffer *buffer){
	if (buffer->head == buffer->tail)
		return 0;
	else {
		uint8_t c = buffer->data[buffer->tail];
		buffer->tail = (uint16_t)(buffer->tail + 1) % UART_RING_BUFFER_SIZE;
		return c;
	}
}

void ring_buffer_callback(ring_buffer *buffer){
	if ((buffer->huart->Instance->ISR & USART_ISR_RXNE) != RESET){
		uint8_t rbyte = (uint8_t)(buffer->huart->Instance->RDR & 0xFF);
		uint16_t i = (uint16_t)(buffer->head + 1) % UART_RING_BUFFER_SIZE;
		if (i != buffer->tail){
			buffer->data[buffer->head] = rbyte;
			buffer->head = i;
		}
	}
}

void ring_buffer_push(ring_buffer *buffer, uint8_t rbyte){
	uint16_t i = (uint16_t)(buffer->head + 1) % UART_RING_BUFFER_SIZE;
	if (i != buffer->tail){
		buffer->data[buffer->head] = rbyte;
		buffer->head = i;
	}
}
void ring_buffer_restore_after_error(ring_buffer *buffer);

