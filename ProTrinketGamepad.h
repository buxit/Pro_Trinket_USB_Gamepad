/*
This is the part of the ProTrinketGamepad code that is exposed to the user

Copyright (c) 2015 Adafruit Industries
All rights reserved.

ProTrinketGamepad is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

ProTrinketGamepad is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with ProTrinketGamepad. If not, see
<http://www.gnu.org/licenses/>.
*/

#ifndef _PROTRINKETGAMEPAD_H_
#define _PROTRINKETGAMEPAD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

class Trinket_Gamepad
{
	private:
	public:
		Trinket_Gamepad(); // empty constructor, ignore me
		void begin(); // starts the USB driver, causes re-enumeration
		void move(signed int x, signed int y, signed int rx, signed int ry, uint16_t buttonMask); // makes a gamepad movement, must be called at least once every 10ms, even if no movement
		char isConnected(); // checks if USB is connected, 0 if not connected
};

// an instance that the user can use
extern Trinket_Gamepad TrinketGamepad;

#endif
