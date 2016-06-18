/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**                STMicroelectronics STM32F4xx Standard Peripherals Library
**
**  Distribution: The file is distributed "as is", without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

/* Includes */
#include "stm32f4xx.h"
#include "serial.h"
#include "wifi.h"
#include "adc.h"
#include "utilities.h"
#include <string.h>

const char* AccountId = "1000001";
const char* PortType = "TCP";
const char* PortAddress = "192.168.1.72";
const char* PortNumber = "8124";
char cmdBuffer[256];

/* Private macro */
/* Private variables */
/* Private function prototypes */
static void OpenSocket(void);
static void SendSocketData(char* buff);

WIFI_RSP RunWifiCommand(char* cmd);
int ms_itoa( char *buf, unsigned int d, int base);
float ReadClampSensor(void);
void SendUartData(volatile char* s);
void DelayMs(uint32_t ms);
void copy_cmd(volatile char* src, volatile char* dest);
uint16_t read_buff[1024];
int r_ptr = 0;
/* Private functions */
extern WIFI_AT* wifi_con;
extern uint32_t ms1,msread;
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{

	RCC_ClocksTypeDef RCC_Clocks;

	char buff[30];

	float read;

	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	DelayMs(500);

	ADC_Configuration();


	ConfigureUsart(115200);
	ConfigureWifiUsart(115200);

	DelayMs(5000);

	WiFiInit();


	while (1)
	{

		/* take a e-sensor reading */
		read = ReadClampSensor();
		/* convert to char array */
		ConvertToDecimal(buff,read);
		/* send to wifi */
		OpenSocket();
		SendSocketData(buff);

		RunWifiCommand("AT+CIPCLOSE=0\r");

		DelayMs(1000);


	}

}


/**
 * @brief Open socket on WiFi module
 */
static void OpenSocket(void)
{
	strcpy(cmdBuffer,"AT+CIPSTART=0,");
	strcat(cmdBuffer,"\"");
	strcat(cmdBuffer,PortType);
	strcat(cmdBuffer,"\",");
	strcat(cmdBuffer,"\"");
	strcat(cmdBuffer,PortAddress);
	strcat(cmdBuffer,"\",");
	strcat(cmdBuffer,PortNumber);
	strcat(cmdBuffer,"\r");

	RunWifiCommand(cmdBuffer);
}
/**
 * @brief Send char array socket data
 * @parameter The char array to send
 */
static void SendSocketData(char* buff)
{
	char data_out[100];
	char tmp[10];

	strcpy(data_out,AccountId);
	strcat(data_out,",E=");
	strcat(data_out,buff);
	strcat(data_out,"\r\n");
	DelayMs(100);
	strcpy(cmdBuffer,"AT+CIPSEND=0,");
	ms_itoa(tmp,strlen(data_out),10 );
	strcat(cmdBuffer,tmp);
	strcat(cmdBuffer,"\r");



	RunWifiCommand(cmdBuffer);

	SendAT(data_out);


}


void copy_cmd(volatile char* src, volatile char* dest)
{

	while(*src){
		*dest = *src;
		*src++;
		*dest++;
	}

}


void SendUartData(volatile char* s)
{
	while(*s){
		// wait until data register is empty
		while( !(USART1->SR & 0x00000040) );
		USART_SendData(USART1, *s);
		*s++;
	}
}



float ReadClampSensor(void)
{

	uint16_t read;
	float result = 0.0;
	int i;

	uint32_t tot = 0;

	r_ptr = 0;
	for(i = 0; i < 600; i++){
		read = adc_convert();
		if(read > tot)
			tot = read;
		DelayMs(1);
	}

	result = (double)tot * 0.024;


	return result;


}






