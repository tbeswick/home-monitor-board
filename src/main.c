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
/* Private macro */
/* Private variables */
/* Private function prototypes */
WIFI_RSP RunWifiCommand(char* cmd);
void ms_itoa( char *buf, unsigned int d, int base);
void DelayMs(uint32_t ms);
/* Private functions */
extern WIFI_AT* wifi_con;
extern uint32_t ms1;
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	int i = 0;
	uint32_t count;
	RCC_ClocksTypeDef RCC_Clocks;

	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	ConfigureUsart(115200);
	ConfigureWifiUsart(115200);


	RunWifiCommand("AT\r");

	DelayMs(5000);

	RunWifiCommand("AT+CIPMUX=1\r");

	DelayMs(20);




	DelayMs(2000);
	RunWifiCommand("AT+CIFSR\r");


  /* Infinite loop */
  count = 1;
  while (1)
  {

	DelayMs(5000);
	RunWifiCommand("AT+CIPSTART=0,\"TCP\",\"192.168.1.72\",21\r");

	DelayMs(5000);

	RunWifiCommand("AT+CIPSEND=0,21\r");
	DelayMs(20);

	if(count == 1){
		RunWifiCommand("Hello from client A\r\n");
		count = 0;
	}else{
		RunWifiCommand("Hello from client B\r\n");
		count = 1;
	}

	RunWifiCommand("AT+CIPCLOSE=0\r");


  }
}

WIFI_RSP RunWifiCommand(char* cmd)
{
	wifi_con->new_response = 0;
	wifi_con->rx_index = 0;
	SendAT(cmd);
	while(wifi_con->new_response == 0);

	return RSP_OK;

}

void DelayMs(uint32_t ms)
{

	ms1 = 0;
	while(ms1 < ms);

}

void ms_itoa( char *buf, unsigned int d, int base)
{
	int div = 1;
	int ind = 0;
	while (d/div >= base)
		div *= base;

	while (div != 0)
	{
		int num = d/div;
		d = d%div;
		div /= base;
		if (num > 9)
			buf[ind] = (num-10) + 'A';
		else
			buf[ind] = num + '0';
		ind++;
	}
}

