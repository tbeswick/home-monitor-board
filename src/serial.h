/*
 * serial.h
 *
 *  Created on: May 21, 2016
 *      Author: Tim
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#define 	STX 				0x7F
#define 	SERIAL_ADDRESS		0x00
#define 	CRC_SSP_SEED		0xFFFF
#define 	CRC_SSP_POLY		0x8005

typedef struct{
	uint8_t data_in[256];
	uint8_t data_out[256];
	uint8_t rx_index;
	uint8_t rx_length;
	uint8_t tx_length;
	uint8_t tx_index;
	uint8_t stuffed;
	uint8_t sync;
}SERIAL_DATA;

typedef enum{
	R_OK = 0xF0
}GEN_RESPONSE;


typedef enum{
	SYNC = 0x11
}SERIAL_COMMAND;

#define NULL (void*)0


void ConfigureUsart(uint32_t baudrate);
void ProcessSerialIn(int8_t ch);


#endif /* SERIAL_H_ */
