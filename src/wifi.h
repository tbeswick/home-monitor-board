/*
 * wifi.h
 *
 *  Created on: May 21, 2016
 *      Author: Tim
 */

#ifndef WIFI_H_
#define WIFI_H_


typedef enum{
	RSP_OK,
	RSP_TIMEOUT
}WIFI_RSP;

typedef struct{
	int8_t rsp[1024];
	uint32_t rx_index;
	uint8_t new_response;
}WIFI_AT;



void WiFiInit(void);
void ConfigureWifiUsart(uint32_t baudrate);
void SendAT(char* str);
void ProcessWiFiResponse(void);

WIFI_RSP RunWifiCommand(char* cmd);


#endif /* WIFI_H_ */
