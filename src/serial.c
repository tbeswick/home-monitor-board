/*
 * serial.c
 *
 *  Created on: May 21, 2016
 *      Author: Tim
 */
#include "stm32f4xx.h"
#include "serial.h"
#include <stdlib.h>

SERIAL_DATA* ser;



static uint16_t CalculateCRC(uint8_t* data, uint16_t length);
static uint32_t check_packet_crc(uint8_t* data, uint16_t length);
static void ParsePacketCommand(SERIAL_DATA* pkt);
static void SendResponse(SERIAL_DATA* pkt, GEN_RESPONSE gen, uint8_t * data, uint16_t length);


void ProcessSerialIn(int8_t ch)
{


	// packet start
	if(STX == ch && 0 == ser->rx_index ){
		ser->data_in[ser->rx_index++] = ch;
	}else{
		// check for stuffed byte
		if(ser->stuffed){
			// not STX so restart the packet
			if(ch != STX){
				ser->data_in[0] = STX;
				ser->data_in[1] = ch;
				ser->rx_index = 2;
			}else{
				// byte stuffed so add just one to data
				ser->data_in[ser->rx_index++] = ch;
			}
			// reset stuffed flag
			ser->stuffed = 0;
		}else{
			// set stuffed for next byte in check
			if(STX == ch){
				ser->stuffed = 1;
			}else{
				ser->data_in[ser->rx_index++] = ch;
				// check for length
				if(3 == ser->rx_index){
					ser->rx_length = ser->data_in[2] + 5;
				}
			}

			// check for end of packet data
			if(ser->rx_index == ser->rx_length){
				// check packet address
				if((ser->data_in[1] & STX) == SERIAL_ADDRESS ){
					// check packet CRC
					if(check_packet_crc(ser->data_in, ser->rx_length)){

						ParsePacketCommand(ser);
					}
				}
				// reset parameters for next packet
				ser->rx_length = 255;
				ser->rx_index = 0;
				ser->stuffed = 0;
			}

		}


	}

}


static void ParsePacketCommand(SERIAL_DATA* pkt)
{


	SERIAL_COMMAND cmd = (SERIAL_COMMAND)pkt->data_in[3];

	// check sync bit
	if(cmd != SYNC)
	{
		// syncs not the same so re-send last packet
		if(pkt->sync != (pkt->data_in[1] & 0x80)){
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
			return;
		}
	}


	switch(cmd)
	{
	case SYNC:
		pkt->sync = 0x80;
		SendResponse(pkt, R_OK, NULL, 0);
		break;


	}

	// set the sync bit for next time

		if(pkt->sync == 0x00){
			pkt->sync = 0x80;
		}else{
			pkt->sync = 0;
		}


}


static void SendResponse(SERIAL_DATA* pkt, GEN_RESPONSE gen, uint8_t * data, uint16_t length)
{

	int i,j;
	uint16_t crc;
	uint8_t tbuffer[256];

	// compile packet data
	pkt->stuffed = 0;
	pkt->tx_index = 0;
	pkt->tx_length = 0;
	pkt->data_out[ser->tx_length++] = STX;
	pkt->data_out[ser->tx_length++] = (SERIAL_ADDRESS | pkt->sync);
	pkt->data_out[ser->tx_length++] = length + 1;
	pkt->data_out[ser->tx_length++] = (uint8_t)gen;
	for(i = 0; i < length; i++){
		pkt->data_out[ser->tx_length++] = data[i];
	}
	crc = CalculateCRC(&ser->data_out[1],ser->tx_length - 1);
	pkt->data_out[ser->tx_length++] = (crc & 0xFF);
	pkt->data_out[ser->tx_length++] = ((crc >> 8) & 0xFF);

	// stuff the packet
	j = 0;
	tbuffer[j++] = ser->data_out[0];
	for(i = 1; i < ser->tx_length; i++){
		tbuffer[j] = ser->data_out[i];
		if(tbuffer[j] == STX){
			tbuffer[++j] = STX;
		}
		j++;
	}
	// copy stuffed buffer back over
	ser->tx_length = j;
	for(i = 0; i < j; i++){
		ser->data_out[i] = tbuffer[i];
	}
	// trigger the TX interrupt to start transmission
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

}



static uint32_t check_packet_crc(uint8_t* data, uint16_t length)
{

	uint16_t crc = CalculateCRC(&data[1], length - 3);

	if((crc & 0xFF) == data[length - 2] && ((crc >> 8) & 0xFF) == data[length - 1]){
		return 1;
	}else{
		return 0;
	}


}


static uint16_t CalculateCRC(uint8_t* data, uint16_t length)
{
	int i, j;
	uint16_t crc = CRC_SSP_SEED;

	for ( i = 0; i < length; ++i )
	{
		crc ^= (data[i] << 8 );
		for ( j = 0; j < 8; ++j )
		{
			if ( crc & 0x8000 )
				crc = ( crc << 1 ) ^ CRC_SSP_POLY;
			else
				crc <<= 1;
		}
	}
	return crc;
}



void ConfigureUsart(uint32_t baudrate)
{

	ser = (SERIAL_DATA*)malloc(sizeof(SERIAL_DATA));
	ser->rx_index = 0;
	ser->rx_length = 255;
	ser->tx_length = 0;
	ser->tx_index = 0;
	ser->stuffed = 0;
	ser->sync = 0;

	//structures used configure the hardware
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	//enable the clocks for the GPIOB and the USART
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	//Initialise pins GPIOB 6 and GPIOB 7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; //we are setting the pin to be alternative function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//Connect the TX and RX pins to their alternate function pins
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	//configure USART
	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //enable send and receive (Tx and Rx)
	USART_Init(USART1, &USART_InitStruct);

	//Enable the interupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// finally this enables the complete USART1 peripheral
	USART_Cmd(USART1, ENABLE);
}
