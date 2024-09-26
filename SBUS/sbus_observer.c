/*
 * sbus_observer.c
 *
 *  Created on: Apr 11, 2023
 *      Author: sergey
 */
#include "SBUS_observer.h"

#define SBUS_STARTBYTE         0x0f
#define SBUS_ENDBYTE           0x00

SBUS_status_t SBUS_read(SBUS_observer *observer){
	ring_buffer *buff = observer->buff;
	while (ring_buffer_available(buff)){
		uint8_t rx = ring_buffer_read(buff);
		if (observer->buffer_index == 0 && rx != SBUS_STARTBYTE)
			continue;
		observer->temp[observer->buffer_index++] = rx;
		if (observer->buffer_index == 25){
			observer->buffer_index = 0;
			if (observer->temp[24] != SBUS_ENDBYTE){
				return SBUS_SYNC_ERR;
			}
			volatile int *_channels = observer->_channels;
			uint8_t *buffer = observer->temp;
			_channels[0]  = ((buffer[1]    |buffer[2]<<8)                 & 0x07FF);
			_channels[1]  = ((buffer[2]>>3 |buffer[3]<<5)                 & 0x07FF);
			_channels[2]  = ((buffer[3]>>6 |buffer[4]<<2 |buffer[5]<<10)  & 0x07FF);
			_channels[3]  = ((buffer[5]>>1 |buffer[6]<<7)                 & 0x07FF);
			_channels[4]  = ((buffer[6]>>4 |buffer[7]<<4)                 & 0x07FF);
			_channels[5]  = ((buffer[7]>>7 |buffer[8]<<1 |buffer[9]<<9)   & 0x07FF);
			_channels[6]  = ((buffer[9]>>2 |buffer[10]<<6)                & 0x07FF);
			_channels[7]  = ((buffer[10]>>5|buffer[11]<<3)                & 0x07FF);
			_channels[8]  = ((buffer[12]   |buffer[13]<<8)                & 0x07FF);
			_channels[9]  = ((buffer[13]>>3|buffer[14]<<5)                & 0x07FF);
			_channels[10] = ((buffer[14]>>6|buffer[15]<<2|buffer[16]<<10) & 0x07FF);
			_channels[11] = ((buffer[16]>>1|buffer[17]<<7)                & 0x07FF);
			_channels[12] = ((buffer[17]>>4|buffer[18]<<4)                & 0x07FF);
			_channels[13] = ((buffer[18]>>7|buffer[19]<<1|buffer[20]<<9)  & 0x07FF);
			_channels[14] = ((buffer[20]>>2|buffer[21]<<6)                & 0x07FF);
			_channels[15] = ((buffer[21]>>5|buffer[22]<<3)                & 0x07FF);

			for (int i = 0; i < 15; ++i) {
				observer->channels[i] = observer->_channels[i];
			}
			return SBUS_OK;
		}
	}
	return SBUS_END_STREAM;
}

SBUS_status_t SBUS_read_compressed(SBUS_observer *observer, SBUS_compressed *out_buff){
	ring_buffer *buff = observer->buff;
	while (ring_buffer_available(buff)){
		uint8_t rx = ring_buffer_read(buff);
		if (observer->buffer_index == 0 && rx != SBUS_STARTBYTE)
			continue;
		observer->temp[observer->buffer_index++] = rx;
		if (observer->buffer_index == 25){
			observer->buffer_index = 0;
			if (observer->temp[24] != SBUS_ENDBYTE){
				return SBUS_SYNC_ERR;
			}
			volatile int *_channels = observer->_channels;
			uint8_t *buffer = observer->temp;
			_channels[0]  = ((buffer[1]    |buffer[2]<<8)                 & 0x07FF);
			_channels[1]  = ((buffer[2]>>3 |buffer[3]<<5)                 & 0x07FF);
			_channels[2]  = ((buffer[3]>>6 |buffer[4]<<2 |buffer[5]<<10)  & 0x07FF);
			_channels[3]  = ((buffer[5]>>1 |buffer[6]<<7)                 & 0x07FF);
			_channels[4]  = ((buffer[6]>>4 |buffer[7]<<4)                 & 0x07FF);
			_channels[5]  = ((buffer[7]>>7 |buffer[8]<<1 |buffer[9]<<9)   & 0x07FF);
			_channels[6]  = ((buffer[9]>>2 |buffer[10]<<6)                & 0x07FF);
			_channels[7]  = ((buffer[10]>>5|buffer[11]<<3)                & 0x07FF);
			_channels[8]  = ((buffer[12]   |buffer[13]<<8)                & 0x07FF);
			_channels[9]  = ((buffer[13]>>3|buffer[14]<<5)                & 0x07FF);
			_channels[10] = ((buffer[14]>>6|buffer[15]<<2|buffer[16]<<10) & 0x07FF);
			_channels[11] = ((buffer[16]>>1|buffer[17]<<7)                & 0x07FF);
			_channels[12] = ((buffer[17]>>4|buffer[18]<<4)                & 0x07FF);
			_channels[13] = ((buffer[18]>>7|buffer[19]<<1|buffer[20]<<9)  & 0x07FF);
			_channels[14] = ((buffer[20]>>2|buffer[21]<<6)                & 0x07FF);
			_channels[15] = ((buffer[21]>>5|buffer[22]<<3)                & 0x07FF);

			for (int i = 0; i < 15; ++i) {
				observer->channels[i] = observer->_channels[i];
			}
			for (int i = 0; i < 6; ++i) {
				out_buff->sticks[i] = buffer[i+1];
			}
			out_buff->aux = 0;
			for (int i = 0; i < 8; ++i) {
				uint8_t ch = (_channels[i + 4] > 1000) ? 1 : 0;		// magic
				out_buff->aux |= ch << i;
			}
			out_buff->flags = buffer[23]; 
			return SBUS_OK;
		}
	}
	return SBUS_END_STREAM;
}
