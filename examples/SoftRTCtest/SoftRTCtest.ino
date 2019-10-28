/*
(C) 2016 dingus.dk J.Ø.N.

This file is part of the SoftRTC library

SoftRTC is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SoftRTC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with IHCSoapClient.If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------------

This example will connect to your wifi, initialize the SoftRTC and print the
time with a 10 seconds interval.
*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <debugtrace.h>
#include "SoftRTC.h"

// The define below if just to make it easier for me to have a "private" settings file
// You should open the settings.h file and change your wifi/ihc settings
#ifdef MYSETTINGSFILE
#include "mysettings.h"
#else
#include "settings.h"
#endif

void setup() {
	Serial.begin(115200);
	Serial.println();

	// We start by connecting to a WiFi network
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, wifipassword);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");

	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	SoftRTC.Initialize();
}

uint32 t;

void loop() {
	SoftRTC.HandleLoop();

	uint32 now = millis();
	if (now / (1000 * 10) == t) return;
	t = now / (1000 * 10);

	unsigned long secsSince1900 = SoftRTC.GetTime();
	if (secsSince1900 == 0) return;

	Serial.print("Seconds since Jan 1 1900 = ");
	Serial.println(secsSince1900);

	// print the hour, minute and second:
	Serial.print("The UTC time is ");
	Serial.printf("%02i", SoftRTC.GetHours(secsSince1900));
	Serial.print(':');
	Serial.printf("%02i", SoftRTC.GetMinutes(secsSince1900));
	Serial.print(':');
	Serial.printf("%02i", SoftRTC.GetSeconds(secsSince1900));
	Serial.println("");

	time_t time = SoftRTC.GetUnixTime();
	tm t2;
	SoftRTC.BuildTime(time, t2);
	Serial.println(asctime(&t2));
}
