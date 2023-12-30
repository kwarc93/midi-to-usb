/*
 * usart.c
 *
 *  Created on: 23 gru 2017
 *      Author: Kwarc
 */
#include <avr/io.h>

void USART_Init(unsigned long int baud)
{
	unsigned int ubrr = (F_CPU + (16 * baud) / 2) / (16 * baud) - 1;

	/* Set baud rate */
	UBRR1 = (unsigned char)(ubrr >> 8);
	UBRR1 = (unsigned char)ubrr;
	/* Double usart speed */
	UCSR1A |= (1 << U2X1);
	/* Enable RX interrupt */
	UCSR1B |= (1 << RXCIE1);
	/* Enable receiver */
	UCSR1B = (1 << RXEN1);
	/* Set frame format: 8data, 1stop bit */
	UCSR1C = (3 << UCSZ10);
}

void USART_TransmitChar(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR1A & (1 << UDRE1)) );
	/* Put data into buffer, sends the data */
	UDR1 = data;
}

void USART_TransmitString(const char* str)
{
	while(*str)
	{
		USART_TransmitChar(*str++);
	}
}

unsigned char USART_ReceiveChar(void)
{
	/* Wait for data to be received */
	while ( !(UCSR1A & (1 << RXC1)) );
	/* Get and return received data from buffer */
	return UDR1;
}

unsigned char USART_GetCharFromISR(void)
{
	return UDR1;
}

void USART_Flush(void)
{
	unsigned char dummy;
	while ( UCSR1A & (1 << RXC1) ) dummy = UDR1;
}
