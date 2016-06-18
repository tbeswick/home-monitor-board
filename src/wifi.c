/*
 * wifi.c
 *
 *  Created on: May 21, 2016
 *      Author: Tim
 */
#include "stm32f4xx.h"
#include "wifi.h"
#include <stdlib.h>
#include "utilities.h"

WIFI_AT* wifi_con;




/**
 *@brief Send command array to module and wait for response (blocking)
 */
WIFI_RSP RunWifiCommand(char* cmd)
{
	wifi_con->new_response = 0;
	wifi_con->rx_index = 0;
	SendAT(cmd);
	while(wifi_con->new_response == 0);

	return RSP_OK;

}

/**
 *@brief Send the character array buffer to WiFi Usart (blocking)
 *@parameter str the char array to send
 */
void SendAT(char* str)
{

	while (*str) {
		while( !(USART2->SR & 0x00000040) );
		USART_SendData(USART2,(uint16_t)*str);
		/* Increase string pointer */
		str++;
	}



}


void ProcessWiFiResponse(void)
{



}

/**
 * @brief Configure hardware and pins for WiFi module
 * @parameter baudrate the module communication baud rate
 */
void ConfigureWifiUsart(uint32_t baudrate)
{


	wifi_con = (WIFI_AT*)malloc(sizeof(WIFI_AT));
	wifi_con->rx_index = 0;
	wifi_con->new_response = 0;

	//structures used configure the hardware
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	//enable the clocks for the GPIOB and the USART
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//Initialise pins GPIOB 6 and GPIOB 7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; //we are setting the pin to be alternative function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Connect the TX and RX pins to their alternate function pins
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	//configure USART
	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //enable send and receive (Tx and Rx)
	USART_Init(USART2, &USART_InitStruct);

	//Enable the interupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// finally this enables the complete USART2 peripheral
	USART_Cmd(USART2, ENABLE);
}

/**
 *@brief Initialise the WiFi module (esp8266)
 */
void WiFiInit(void)
{

	RunWifiCommand("AT\r");
	DelayMs(40);
	RunWifiCommand("AT+CIPMUX=1\r");
	DelayMs(40);
	RunWifiCommand("AT+CIFSR\r");
	DelayMs(40);

}

