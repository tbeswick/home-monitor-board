/*
 * utilities.c
 *
 *  Created on: May 29, 2016
 *      Author: Tim
 */
#include "stm32f4xx.h"
#include "utilities.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

extern uint32_t ms1;

/**
 *@brief Wait for ms (blocking) uses ms1 global
 */
void DelayMs(uint32_t ms)
{

	ms1 = 0;
	while(ms1 < ms);

}

/**
 *@brief Convert floating point number to character array
 *@parameter res the pointer to the char array result
 *@parameter num the floating point number to convert
 */
void ConvertToDecimal(char* res, float num)
{
	char tmp[10];

	int d1 = num;            // Get the integer part (678).
	float f2 = num - d1;     // Get fractional part (678.0123 - 678 = 0.0123).
	int d2 = trunc(f2 * 1000);   // Turn into integer (123).
	sprintf(tmp,"%d",d1);
	strcpy(res,tmp);
	strcat(res,".");
	sprintf(tmp,"%d",d2);
	strcat(res,tmp);

}


/**
 *@brief Covert integer to character array
 *@parameter buff pointer to the result buffer
 *@parameter d the integer to convert
 *@parameter base the number base to use for conversion e.g. 10 dec or 16 hex
 *@return integer giving the length of the buffer converted
 */
int ms_itoa( char *buf, unsigned int d, int base)
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

	buf[ind] = '\0';

	return ind;
}
