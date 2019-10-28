/*
(C) 2015 dingus.dk J.�.N.

This file is part of ArduinoIHC.

SoftRTC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SoftRTC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SoftRTC.  If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------------
*/
#ifndef _SOFTRTC_h
#define _SOFTRTC_h

#include <time.h>
#include <WiFiUdp.h>

class SoftRTCClass {

protected:

	String NTPServerName;

	unsigned long SecsSince1900;
	unsigned long LastUpdateTime; // last update time in ms ( Millis function)
	unsigned long UpdateInterval;
	unsigned long LastSendTime;

	WiFiUDP Udp;

	void SendNTPpacket();
	void ProcessPacket();

public:
	
	SoftRTCClass();
	void Initialize( const char* timeserver = "pool.ntp.org",int port = 2390);
	void HandleLoop();
	void SetUpdateInterval(unsigned long minutes);
	bool WaitForNTPServer(int timeout = 10*1000);

	unsigned long GetTime();
	time_t GetUnixTime();
	void BuildTime(unsigned long time,tm& t);

	static int GetHours(unsigned long time);
	static int GetMinutes(unsigned long time);
	static int GetSeconds(unsigned long time);
};

extern SoftRTCClass SoftRTC;

#endif

