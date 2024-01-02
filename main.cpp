/*
 * main.c
 *
 *  Created on: 28 gru 2023
 *      Author: kwarc
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "usb_descriptors.h"
#include "fast_queue.h"
#include "midiXparser/midiXparser.h"

static void system_init(void);

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
static USB_ClassInfo_MIDI_Device_t midi_keyb_interface {
	{
		INTERFACE_ID_AudioStream,
		{ MIDI_STREAM_IN_EPADDR, MIDI_STREAM_EPSIZE, 1},
		{ MIDI_STREAM_OUT_EPADDR, MIDI_STREAM_EPSIZE, 1},
	}
};

static fast_queue<uint8_t, 64> uart_queue;
static midiXparser midiParser;

int main(void)
{
	system_init();

	/* Only basic MIDI messages are captured */
	midiParser.setMidiMsgFilter(midiXparser::channelVoiceMsgTypeMsk);

	for (;;)
	{
		unsigned char uart_char;
		while (uart_queue.pop(uart_char))
		{
			if (midiParser.parse(uart_char))
			{
				auto midi_msg = midiParser.getMidiMsg();

				/* Forward MIDI message to USB event */
				const MIDI_EventPacket_t midi_to_usb_evt
				{
					MIDI_EVENT(0, midi_msg[0]),

					midi_msg[0],
					midi_msg[1],
					midi_msg[2],
				};

				MIDI_Device_SendEventPacket(&midi_keyb_interface, &midi_to_usb_evt);
				MIDI_Device_Flush(&midi_keyb_interface);
			}
		}

		MIDI_EventPacket_t usb_to_midi_evt;
		while (MIDI_Device_ReceiveEventPacket(&midi_keyb_interface, &usb_to_midi_evt))
		{
			/* Ignore received events */
		}

		MIDI_Device_USBTask(&midi_keyb_interface);
		USB_USBTask();
	}
}

static void system_init(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Enable UART1 RX with interrupts, 31250 8n1 */
	uint16_t ubrr = F_CPU / (16UL * 31250UL) - 1;
	UBRR1H = static_cast<uint8_t>(ubrr >> 8);
	UBRR1L = static_cast<uint8_t>(ubrr);
	UCSR1B |= (1 << RXCIE1) | (1 << RXEN1);
	UCSR1C |= (3 << UCSZ10);

	/* Configure PD7 for button */
	DDRD &= ~(1 << 7);
	PORTD |= (1 << 7);

	/* Enable USB */
	USB_Init();

	/* Enable interrupts */
	sei();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool cfg_success = true;

	cfg_success &= MIDI_Device_ConfigureEndpoints(&midi_keyb_interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MIDI_Device_ProcessControlRequest(&midi_keyb_interface);
}

/** UART RX interrupt handler */
ISR(USART1_RX_vect)
{
	uart_queue.push(static_cast<uint8_t>(UDR1));
}

