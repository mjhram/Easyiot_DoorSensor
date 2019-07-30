#ifndef GLOBAL_H
#define GLOBAL_H

#include <ESP8266WiFi.h>
#define FS_NO_GLOBALS
//#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <SPIFFSEditor.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <SimpleTimer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <cppQueue.h>

#include <Arduino.h>
#include "strDateTime.h"

#include "wSerial.h"

#include <Wire.h>
#include "Adafruit_MCP23017.h"

extern RemoteDebug Debug;
extern wSerial wserial;

extern const char* ssid;
extern const char* password;

extern AsyncWebServer server;									// The Webserver
extern boolean firstStart;										// On firststart = true, NTP will try to get a valid time
extern int AdminTimeOutCounter;									// Counter for Disabling the AdminMode
extern strDateTime2 DateTime;											// Global DateTime structure, will be refreshed every Second
extern WiFiUDP UDPNTPClient;											// NTP Client
extern unsigned long UnixTimestamp;
extern boolean Refresh; // For Main Loop, to refresh things like GPIO / WS2812
extern int cNTP_Update;											// Counter for Updating the time via NTP
extern Ticker tkSecond;												// Second - Timer for Updating Datetime Structure
extern boolean AdminEnabled;		// Enable Admin Mode for a given Time
//extern byte Minute_Old;				// Helpvariable for checking, when a new Minute comes up (for Auto Turn On / Off)
extern bool ntpSyncd;
extern bool disarm;


struct strConfig {
	char ssid[16];
	char password[16];
	byte  IP[4];
	byte  Netmask[4];
	byte  Gateway[4];
	boolean dhcp;
	char ntpServerName[32];
	uint8 Update_Time_Via_NTP_Every;
	int8 timezone;
	boolean daylight;
	char DeviceName[16];
	/*boolean AutoTurnOff;
	boolean AutoTurnOn;
	byte TurnOffHour;
	byte TurnOffMinute;
	byte TurnOnHour;
	byte TurnOnMinute;
	byte LED_R;
	byte LED_G;
	byte LED_B;

  uint moduleId;  // module id
  boolean state;     // state*/
};
extern strConfig config;
/////////////////////
enum EventType {Dummy, DoorEvent, PowerEvent, AnalogEvent};

class EventStruct {
  public:
  EventType type;//door, power
  byte state;//door open/closed, power down/up => not used for now
  int8 pinIdx;
  int trigger;//0 don't trigger action, 1 trigger
  long time;
  unsigned long timestamp;
	String toString() {
		String tmp;
		switch(type) {
			case Dummy:
				tmp+="Dummy";
				break;
			case DoorEvent:
				tmp+="Door";
				break;
			case PowerEvent:
				tmp+="Power";
				break;
		}
		tmp+=" -Pin:"+String(pinIdx);
		tmp+=" -trigger:"+String(trigger);
		return tmp;
	}
  EventStruct() {
    
  }
  EventStruct(EventType t, byte st, byte id, int tr, long tm) {
    type = t;
    state = st;
    pinIdx = id;
    trigger = tr;
    time = tm;
  }

  EventStruct(const EventStruct &ev) {
    type = ev.type;
    state = ev.state;
    pinIdx = ev.pinIdx;
    trigger = ev.trigger;
    time = ev.time;
    timestamp = ev.timestamp;
  }

  void operator=(const EventStruct &ev) {
    type = ev.type;
    state = ev.state;
    pinIdx = ev.pinIdx;
    trigger = ev.trigger;
    time = ev.time;
    timestamp = ev.timestamp;
  }
  /*
  //use time & pinIdx to compare 2 events directly without operator:
  // ev.time - lastEvent[ev.pinIdx] >= xxxx
  bool operator==(const EventStruct& ev) {
    //if it is not the same pin => different event
    if(pinIdx != ev.pinIdx) {
      return false;
    } else {
      return true;
    }    
  }
  bool operator!=(const EventStruct& ev) {
    //if it is not the same pin => different event
    if(pinIdx != ev.pinIdx) {
      return true;
    } else {
      return false;
    }    
  }*/
};

const EventStruct dummyEvent = {Dummy, -1, -1, false, -1};
extern Queue	fifoq;
extern Queue publishq;
extern Adafruit_MCP23017 mcp;
//volatile EventStruct doorEvent;
//volatile bool newEvent, isNotifying, sendIfttt; 
//bool sendEspOn = true;

#define WATCHDOG_PIN D0           // used for watchdog (sending pulse every 1sec)
extern bool watchdogup;            //use it to toggle watchdog pin

#define NPINS 4
//const int sensorPins[NPINS]={D5, D6, A0};//, D4, D2, D5};
const int sensorPins[NPINS]={0,1,2,A0};//{D1, D2, A0};//, D4, D2, D5};
const byte sensorPinsInvert[NPINS]={0, 1, 0, 0};//, 0};//, 0, 1};
const EventType eventType[NPINS]={DoorEvent, PowerEvent, DoorEvent, AnalogEvent};//, DoorEvent};//, DoorEvent, PowerEvent};
extern EventStruct lastEvent[];//, dummyEvent, dummyEvent};
const int sensorPinMode[NPINS] = {RISING, FALLING, RISING, 0};//, RISING};
class EventsArray {
	public:
	int size;
	EventStruct events[NPINS];
  
	EventsArray(int nEvents) {
		size = nEvents;
		//events = new EventStruct[nEvents];
	}
	EventsArray() {}
};

/////////////////////
/*
**
** CONFIGURATION HANDLING
**
*/
//SimpleTimer timer;
extern bool bReconnect;

const int NTP_PACKET_SIZE = 48; 
extern byte packetBuffer[]; 

#define VERSION_1 '1'

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 
#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

void reconnectCheck();
void ConfigureWifi();
void WriteConfig();
boolean ReadConfig();
void NTPRefresh();
void Second_Tick();
void printDirectory(File dir, int numTabs);

boolean summertime(int year, byte month, byte day, byte hour, byte tzHours);
boolean checkRange(String Value);
void WriteStringToEEPROM(int beginaddress, String string);
String  ReadStringFromEEPROM(int beginaddress);
void EEPROMWritelong(int address, long value);
long EEPROMReadlong(long address);
void ConvertUnixTimeStamp( unsigned long TimeStamp, struct strDateTime2* DateTime);
String GetMacAddress();
unsigned char h2int(char c);
String urldecode(String input);
String getConnectionState();
String config2String(strConfig& cfg);

#endif
