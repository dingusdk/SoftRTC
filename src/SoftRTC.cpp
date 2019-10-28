/*
(C) 2015 dingus.dk J.Ø.N.

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
#include <ESP8266WiFi.h>
#include <debugtrace.h>
#include <SoftRTC.h>


SoftRTCClass SoftRTC;

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static  const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; 


SoftRTCClass::SoftRTCClass() {

	SecsSince1900 = 0;
	LastUpdateTime = 0;
	UpdateInterval = 1000 * 60 * 60; // default 1 hour update interval
}

void SoftRTCClass::Initialize(const char* timeserver,int port) {

	NTPServerName = timeserver;
	Udp.begin(port);
	SendNTPpacket();
}

bool SoftRTCClass::WaitForNTPServer(int timeout) {
	uint32 time = millis() + timeout;
	while (millis() < time) {
		if (SecsSince1900 != 0) return true;
		HandleLoop();
	}
	return false;
}


void SoftRTCClass::HandleLoop() {

	ProcessPacket();
	// Do we have a millis overflow
	uint64 now = millis();
	uint64 nextupdate = uint64(LastUpdateTime) + UpdateInterval;
	if (now < LastUpdateTime) now += 0x100000000;
	if (now > nextupdate) {
		// Wait at least 10 sec before we send a packet again.
		if (now >= LastSendTime && now < LastSendTime + 1000*10)
			return;
		SendNTPpacket();
	}
}

void SoftRTCClass::ProcessPacket() {

	int cb = Udp.parsePacket();
	if (!cb) return;

	// We've received a packet, read the data from it
	if (Udp.read(packetBuffer, NTP_PACKET_SIZE) != NTP_PACKET_SIZE) {
		debug_println( "NTP packet size does not match");
		return;
	}
	debug_print("NTP packet recieved ");
	LastUpdateTime = millis();

	//the timestamp starts at byte 40 of the received packet and is four bytes,
	// or two words, long. First, esxtract the two words:
	unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
	unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
	// combine the four bytes (two words) into a long integer
	// this is NTP time (seconds since Jan 1 1900):
	SecsSince1900 = highWord << 16 | lowWord;
	debug_print("Seconds since Jan 1 1900 = ");
	debug_println(SecsSince1900);
}



void SoftRTCClass::SendNTPpacket() {

	IPAddress timeServerIP; 
	WiFi.hostByName(NTPServerName.c_str(), timeServerIP);

	debug_print("Sending NTP packet to: ");
	debug_println(timeServerIP.toString());

	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
													 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
	LastSendTime = millis();
}


/*!
Get the time in seconds since 1 jan 1900
*/

unsigned long SoftRTCClass::GetTime() {

	if (SecsSince1900 == 0) return 0;
	unsigned long now = millis();
	// overflow?
	if (now < LastUpdateTime) {
		return SecsSince1900 + (0x100000000 - LastUpdateTime + now) / 1000;
	}
	return SecsSince1900 + (now - LastUpdateTime) / 1000;
}

time_t SoftRTCClass::GetUnixTime() {

	// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	const unsigned long seventyYears = 2208988800UL;
	// subtract seventy years:
	return GetTime() - seventyYears;
}

int SoftRTCClass::GetHours( unsigned long time) {

	return (time % 86400L) / 3600;
}

int SoftRTCClass::GetMinutes(unsigned long time) {
	
	return (time % 3600) / 60;
}

int SoftRTCClass::GetSeconds(unsigned long time) {

	return time % 60;
}


void SoftRTCClass::BuildTime(unsigned long time, tm& t) {

	uint8_t year;
	uint8_t month, monthLength;
	unsigned long days;

	t.tm_sec = time % 60;
	time /= 60; // now it is minutes
	t.tm_min = time % 60;
	time /= 60; // now it is hours
	t.tm_hour = time % 24;
	time /= 24; // now it is days
	t.tm_wday = ((time + 4) % 7);  // Sunday is day 0

	year = 0;
	days = 0;
	while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	t.tm_year = year; 
	days -= LEAP_YEAR(year) ? 366 : 365;
	time -= days; // now it is days in this year, starting at 0
	t.tm_yday = days;

	days = 0;
	month = 0;
	monthLength = 0;
	for (month = 0; month<12; month++) {
		if (month == 1) { // february
			if (LEAP_YEAR(year)) {
				monthLength = 29;
			}
			else {
				monthLength = 28;
			}
		}
		else {
			monthLength = monthDays[month];
		}

		if (time >= monthLength) {
			time -= monthLength;
		}
		else {
			break;
		}
	}
	t.tm_mon = month + 1;  // jan is month 1  
	t.tm_mday = time + 1;     // day of month
	t.tm_isdst = -1;
}

void SoftRTCClass::SetUpdateInterval(unsigned long minutes) {

	UpdateInterval = 1000 * 60 * minutes;
}
