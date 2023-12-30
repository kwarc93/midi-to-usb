/*
 * usart.h
 *
 *  Created on: 23 gru 2017
 *      Author: Kwarc
 */

#ifndef USART_H
#define USART_H

#if defined(__cplusplus)
	extern "C" {
#endif

void USART_Init(unsigned long int baud);
void USART_TransmitChar(unsigned char data);
void USART_TransmitString(const char* str);
unsigned char USART_ReceiveChar(void);
unsigned char USART_GetCharFromISR(void);
void USART_Flush(void);

#if defined(__cplusplus)
}
#endif

#endif

