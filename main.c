/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the MIDI demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "usb_descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

static void SetupHardware(void);
static void CheckButtonPress(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

/** Button mask for the first button on the board. */
#define BUTTONS_BUTTON1      (1 << 7)

/* Inline Functions */
static inline void Buttons_Init(void)
{
	DDRD  &= ~BUTTONS_BUTTON1;
	PORTD |=  BUTTONS_BUTTON1;
}

static inline void Buttons_Disable(void)
{
	DDRD  &= ~BUTTONS_BUTTON1;
	PORTD &= ~BUTTONS_BUTTON1;
}

ATTR_WARN_UNUSED_RESULT
static inline uint8_t Buttons_GetStatus(void)
{
	return ((PIND & BUTTONS_BUTTON1) ^ BUTTONS_BUTTON1);
}

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface =
	{
		.Config =
			{
				.StreamingInterfaceNumber = INTERFACE_ID_AudioStream,
				.DataINEndpoint           =
					{
						.Address          = MIDI_STREAM_IN_EPADDR,
						.Size             = MIDI_STREAM_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint          =
					{
						.Address          = MIDI_STREAM_OUT_EPADDR,
						.Size             = MIDI_STREAM_EPSIZE,
						.Banks            = 1,
					},
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		CheckButtonPress();

		MIDI_EventPacket_t ReceivedMIDIEvent;
		while (MIDI_Device_ReceiveEventPacket(&Keyboard_MIDI_Interface, &ReceivedMIDIEvent))
		{
			/* Ignore received events */
		}

		MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
static void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	Buttons_Init();
	USB_Init();
}

/** Checks for changes in the position of the board button, sending MIDI events to the host upon each change. */
static void CheckButtonPress(void)
{
	static uint16_t DebounceState;
	static bool Pressed,Released;

    // This works like FIFO of button states, shifts actual button state bit from LSB to MSB
    //          _      ____
    // MSB < __| |____|    |_______ < LSB
    //                  ^      ^
    //                  |      |
    //        pressed --+      +-- released

	#define IGNORE_MASK 0xFE00
    /* Release debounce time: 7 <bits> * <loop period> */
	#define RELEASE_MASK 0xFF80
    /* Press debounce time: 2 <bit> * <loop period> */
	#define PRESS_MASK 0xFE03

    DebounceState = (DebounceState << 1) | (bool)(Buttons_GetStatus() & BUTTONS_BUTTON1) | IGNORE_MASK;

    if (DebounceState == RELEASE_MASK)
    {
    	Released = true;
    }
    else if (DebounceState == PRESS_MASK)
    {
    	Pressed = true;
    }

	uint8_t MIDICommand = 0;
	const uint8_t MIDIPitch = 0x3B;

	if (Pressed)
	{
		Pressed = false;
		MIDICommand = MIDI_COMMAND_NOTE_ON;
	}

	if (Released)
	{
		Released = false;
		MIDICommand = MIDI_COMMAND_NOTE_OFF;
	}

	if (MIDICommand)
	{
		MIDI_EventPacket_t MIDIEvent = (MIDI_EventPacket_t)
		{
			.Event       = MIDI_EVENT(0, MIDICommand),

			.Data1       = MIDICommand | MIDI_CHANNEL(1),
			.Data2       = MIDIPitch,
			.Data3       = MIDI_STANDARD_VELOCITY,
		};

		MIDI_Device_SendEventPacket(&Keyboard_MIDI_Interface, &MIDIEvent);
		MIDI_Device_Flush(&Keyboard_MIDI_Interface);
	}
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
	bool ConfigSuccess = true;

	ConfigSuccess &= MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}

