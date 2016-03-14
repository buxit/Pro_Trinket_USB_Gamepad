/*
This is the part of the ProTrinketGamepad code that is usually written in C
Placing it here does not mix it with the object oriented C++ code

Copyright (c) 2015 Adafruit Industries
All rights reserved.

ProTrinketGamepad is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

ProTrinketGamepad is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with ProTrinketGamepad. If not, see
<http://www.gnu.org/licenses/>.
*/

#include "cmdline_defs.h"
#include "ProTrinketGamepadC.h"
#include "usbconfig.h"
#include "usbdrv/usbdrv.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>

// preprocessor macro for compile time assert, see http://www.pixelbeat.org/programming/gcc/static_assert.html
#define ct_assert(e) extern char (*ct_assert(void)) [sizeof(char[1 - 2*!(e)])]

uint8_t report_buffer[16];
uint8_t idle_rate = 500 / 4;  // see HID1_11.pdf sect 7.2.4
uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6

void usbBegin()
{
    cli();

    // fake a disconnect to force the computer to re-enumerate
    PORTD &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT));

    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();

	// start the USB driver
	usbInit();
	sei();
}

void usbPollWrapper()
{
	usbPoll();
}

void usbReportSend()
{
	// packet size is 8 bytes max, send two packets
	// see http://vusb.wikidot.com/driver-api
	while(!usbInterruptIsReady())
		usbPoll();
	usbSetInterrupt((uint8_t*)report_buffer, 8);

	while(!usbInterruptIsReady())
		usbPoll();
	usbSetInterrupt((uint8_t*)report_buffer+8, 2);
}

// adapted from https://github.com/NicoHood/HID/blob/master/src/SingleReport/SingleGamepad.cpp
// see HID1_11.pdf sect 6.2 and appendix D
// USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH is defined in usbconfig, must match
const PROGMEM char usbHidReportDescriptor[] = {
	// Gamepad with 16 buttons and 4 axis
	0x05, 0x01,		// USAGE_PAGE (Generic Desktop)
	0x09, 0x04,		// USAGE (Joystick)
	0xa1, 0x01,		// COLLECTION (Application)
	// 16 Buttons
	0x05, 0x09,		//   USAGE_PAGE (Button)
	0x19, 0x01,		//   USAGE_MINIMUM (Button 1)
	0x29, 0x10,		//   USAGE_MAXIMUM (Button 16)
	0x15, 0x00,		//   LOGICAL_MINIMUM (0)
	0x25, 0x01,		//   LOGICAL_MAXIMUM (1)
	0x75, 0x01,		//   REPORT_SIZE (1)
	0x95, 0x10,		//   REPORT_COUNT (16)
	0x81, 0x02,		//   INPUT (Data,Var,Abs)
	// 4 16bit Axis
	0x05, 0x01,		//   USAGE_PAGE (Generic Desktop)
	0xa1, 0x00,		//   COLLECTION (Physical)
	0x09, 0x30,		//     USAGE (X)
	0x09, 0x31,		//     USAGE (Y)
	0x09, 0x33,		//     USAGE (Rx)
	0x09, 0x34,		//     USAGE (Ry)
	0x16, 0x00, 0x80,	//     LOGICAL_MINIMUM (-32768)
	0x26, 0xFF, 0x7F,	//     LOGICAL_MAXIMUM (32767)
	0x75, 0x10,		//     REPORT_SIZE (16)
	0x95, 0x04,		//     REPORT_COUNT (4)
	0x81, 0x02,		//     INPUT (Data,Var,Abs)

#if AXIS_8BIT
	// 2 8bit Axis
	0x09, 0x32,		//     USAGE (Z)
	0x09, 0x35,		//     USAGE (Rz)
	0x15, 0x80,		//     LOGICAL_MINIMUM (-128)
	0x25, 0x7F,		//     LOGICAL_MAXIMUM (127)
	0x75, 0x08,		//     REPORT_SIZE (8)
	0x95, 0x02,		//     REPORT_COUNT (2)
	0x81, 0x02,		//     INPUT (Data,Var,Abs)
#endif
	0xc0,			//   END_COLLECTION
#if HAT_SWITCH
	// 2 Hat Switches
	0x05, 0x01,		//   USAGE_PAGE (Generic Desktop)
	0x09, 0x39,		//   USAGE (Hat switch)
	0x09, 0x39,		//   USAGE (Hat switch)
	0x15, 0x01,		//   LOGICAL_MINIMUM (1)
	0x25, 0x08,		//   LOGICAL_MAXIMUM (8)
	0x95, 0x02,		//   REPORT_COUNT (2)
	0x75, 0x04,		//   REPORT_SIZE (4)
	0x81, 0x02,		//   INPUT (Data,Var,Abs)
#endif
	0xc0			// END_COLLECTION
};

// if the sizes don't match, compile fails here
ct_assert(sizeof(usbHidReportDescriptor)==USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

// see http://vusb.wikidot.com/driver-api
// constants are found in usbdrv.h
usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	// see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
		return 0; // ignore request if it's not a class specific request

	// see HID1_11.pdf sect 7.2
	switch (rq->bRequest)
	{
		case USBRQ_HID_GET_IDLE:
			usbMsgPtr = &idle_rate; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_IDLE:
			idle_rate = rq->wValue.bytes[1]; // read in idle rate
			return 0; // send nothing
		case USBRQ_HID_GET_PROTOCOL:
			usbMsgPtr = &protocol_version; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_PROTOCOL:
			protocol_version = rq->wValue.bytes[1];
			return 0; // send nothing
		case USBRQ_HID_GET_REPORT:
			usbMsgPtr = (uint8_t*)report_buffer; // send the report data
			return 3;
		case USBRQ_HID_SET_REPORT:
			return 0; // send nothing, gamepads don't do this
		default: // do not understand data, ignore
			return 0; // send nothing
	}
}

