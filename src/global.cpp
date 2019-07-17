#include "global.h"
#include "wSerial.h"

RemoteDebug Debug;
strDateTime2 DateTime;											// Global DateTime structure, will be refreshed every Second
wSerial wserial(Debug, true, DateTime);

const char* ssid = "MJH_MIFI";
const char* password =  "2013Dgroup2";


AsyncWebServer server(80);									// The Webserver
//ESP8266WebServer server(80);
boolean firstStart = true;										// On firststart = true, NTP will try to get a valid time
int AdminTimeOutCounter = 0;									// Counter for Disabling the AdminMode
WiFiUDP UDPNTPClient;											// NTP Client

unsigned long UnixTimestamp = 0;								// GLOBALTIME  ( Will be set by NTP)
boolean Refresh = false; // For Main Loop, to refresh things like GPIO / WS2812
int cNTP_Update = 0;											// Counter for Updating the time via NTP
Ticker tkSecond;												// Second - Timer for Updating Datetime Structure
boolean AdminEnabled = false;		// Enable Admin Mode for a given Time
//byte Minute_Old = 100;				// Helpvariable for checking, when a new Minute comes up (for Auto Turn On / Off)
bool ntpSyncd = false;
bool disarm = false;

strConfig config;

Queue	fifoq(sizeof(EventStruct), 50, FIFO, true);
Queue publishq(sizeof(EventsArray), 20, FIFO, true);

EventStruct lastEvent[NPINS]={dummyEvent, dummyEvent, dummyEvent};//, dummyEvent, dummyEvent};

bool bReconnect = false;
byte packetBuffer[ NTP_PACKET_SIZE]; 

String config2String(strConfig& cfg) {
	String tmp = "";
	tmp +="ssid:"+String(cfg.ssid);
	tmp +="\ntimezone:"+String(cfg.timezone)+"\n";
	return tmp;
}
//
// Summertime calculates the daylight saving for a given date.
//
boolean summertime(int year, byte month, byte day, byte hour, byte tzHours)
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
{
 if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
 if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
 if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7)))
   return true;
 else
   return false;
}

//
// Check the Values is between 0-255
//
boolean checkRange(String Value)
{
	 if (Value.toInt() < 0 || Value.toInt() > 255)
	 {
		 return false;
	 }
	 else
	 {
		 return true;
	 }
}


void WriteStringToEEPROM(int beginaddress, String string)
{
	char  charBuf[string.length()+1];
	string.toCharArray(charBuf, string.length()+1);
	for (int t=  0; t<sizeof(charBuf);t++)
	{
			EEPROM.write(beginaddress + t,charBuf[t]);
	}
}
String  ReadStringFromEEPROM(int beginaddress)
{
	byte counter=0;
	char rChar;
	String retString = "";
	while (1)
	{
		rChar = EEPROM.read(beginaddress + counter);
		if (rChar == 0) break;
		if (counter > 31) break;
		counter++;
		retString.concat(rChar);

	}
	return retString;
}
void EEPROMWritelong(int address, long value)
      {
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void ConvertUnixTimeStamp( unsigned long TimeStamp, struct strDateTime2* DateTime)
{
		uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;
	  time = (uint32_t)TimeStamp;
	  DateTime->second = time % 60;
	  time /= 60; // now it is minutes
	  DateTime->minute = time % 60;
	  time /= 60; // now it is hours
	  DateTime->hour = time % 24;
	  time /= 24; // now it is days
	  DateTime->wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
	  year = 0;  
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	DateTime->year = year; // year is offset from 1970 
  
	  days -= LEAP_YEAR(year) ? 366 : 365;
	  time  -= days; // now it is days in this year, starting at 0
  
	  days=0;
	  month=0;
	  monthLength=0;
	  for (month=0; month<12; month++) {
		if (month==1) { // february
		  if (LEAP_YEAR(year)) {
			monthLength=29;
		  } else {
			monthLength=28;
		  }
		} else {
		  monthLength = monthDays[month];
		}
    
		if (time >= monthLength) {
		  time -= monthLength;
		} else {
			break;
		}
	  }
	  DateTime->month = month + 1;  // jan is month 1  
	  DateTime->day = time + 1;     // day of month
	  DateTime->year += 1970;

	 
}


	
String GetMacAddress()
{
	uint8_t mac[6];
    char macStr[18] = {0};
	WiFi.macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
    return  String(macStr);
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
	 char c;
	 String ret = "";
	 
	 for(byte t=0;t<input.length();t++)
	 {
		 c = input[t];
		 if (c == '+') c = ' ';
         if (c == '%') {


         t++;
         c = input[t];
         t++;
         c = (h2int(c) << 4) | h2int(input[t]);
		 }
		
		 ret.concat(c);
	 }
	 return ret;
  
}

String getConnectionState() {
  String state = "N/A";
  
  if (WiFi.status() == 0) state = "Idle";
  else if (WiFi.status() == 1) state = "NO SSID AVAILBLE";
  else if (WiFi.status() == 2) state = "SCAN COMPLETED";
  else if (WiFi.status() == 3) state = "CONNECTED";
  else if (WiFi.status() == 4) state = "CONNECT FAILED";
  else if (WiFi.status() == 5) state = "CONNECTION LOST";
  else if (WiFi.status() == 6) state = "DISCONNECTED";
  return state;
}


void reconnectCheck() {
  wserial.print(".");
  if(WiFi.status() == WL_CONNECTED) {
    bReconnect = false;
  }
}
//has an issue when re-establishing a connection
void ConfigureWifi()
{
	wserial.println("Configuring Wifi");
   wserial.println(config.ssid);
   wserial.print("=>");
   wserial.print(config.password);
   //disconnect WiFi
   WiFi.disconnect(true);
   wserial.println("Dis-connnecting");
   while(WiFi.status() == WL_CONNECTED){
      delay(1000);
      wserial.print(".");
   }
   wserial.println("connnecting");
   WiFi.persistent(false);
	WiFi.mode(WIFI_OFF);
	WiFi.mode(WIFI_STA);
	WiFi.setOutputPower(0);
	WiFi.begin(ssid, password);
  
	/*WiFi.begin (config.ssid.c_str(), config.password.c_str());
	if (!config.dhcp)
	{
		WiFi.config(IPAddress(config.IP[0],config.IP[1],config.IP[2],config.IP[3] ),  IPAddress(config.Gateway[0],config.Gateway[1],config.Gateway[2],config.Gateway[3] ) , IPAddress(config.Netmask[0],config.Netmask[1],config.Netmask[2],config.Netmask[3] ));
	}*/
 //int i = 0;
 //int reconnectCheckId = timer.setInterval(1000, reconnectCheck);
 //bReconnect = true;
  while (/*bReconnect == true*/ WiFi.status() != WL_CONNECTED /*&& i++ < (AP_CONNECT_TIME*2)*/) {
    delay(1000);
    //#ifdef DEBUG
      wserial.print(".");
    //#endif
  }
  //timer.disable(reconnectCheckId);*/
 wserial.println(String(WiFi.localIP()));
}

void WriteConfig()
{

	wserial.println("Writing Config");
	EEPROM.write(0,VERSION_1);
	EEPROM.write(1,'F');
	EEPROM.write(2,'G');
	
	EEPROM.put(16, config);
	#if 0
	
	EEPROM.write(16,config.dhcp);
	EEPROM.write(17,config.daylight);
	
	EEPROM.write(18,config.Update_Time_Via_NTP_Every); //  Byte

	EEPROM.write(22,config.timezone);  //  Byte


	/*EEPROM.write(26,config.LED_R);
	EEPROM.write(27,config.LED_G);
	EEPROM.write(28,config.LED_B);*/

	EEPROM.write(32,config.IP[0]);
	EEPROM.write(33,config.IP[1]);
	EEPROM.write(34,config.IP[2]);
	EEPROM.write(35,config.IP[3]);

	EEPROM.write(36,config.Netmask[0]);
	EEPROM.write(37,config.Netmask[1]);
	EEPROM.write(38,config.Netmask[2]);
	EEPROM.write(39,config.Netmask[3]);

	EEPROM.write(40,config.Gateway[0]);
	EEPROM.write(41,config.Gateway[1]);
	EEPROM.write(42,config.Gateway[2]);
	EEPROM.write(43,config.Gateway[3]);


	EEPROM.put(64,config.ssid);
	EEPROM.put(96,config.password);
	EEPROM.put(128,config.ntpServerName);

	/*EEPROM.write(300,config.AutoTurnOn);
	EEPROM.write(301,config.AutoTurnOff);
	EEPROM.write(302,config.TurnOnHour);
	EEPROM.write(303,config.TurnOnMinute);
	EEPROM.write(304,config.TurnOffHour);
	EEPROM.write(305,config.TurnOffMinute);*/
	//WriteStringToEEPROM
	EEPROM.put(306,config.DeviceName);
	//EEPROM.write(338,config.moduleId);
  	//EEPROM.write(339,config.state);
	#endif
	EEPROM.commit();
}
boolean ReadConfig()
{

	wserial.println("Reading Configuration");
	if (EEPROM.read(0) == VERSION_1 && EEPROM.read(1) == 'F'  && EEPROM.read(2) == 'G' )
	{
		wserial.println("Configurarion Found!");
		EEPROM.get(16, config);
		#if 0
		config.dhcp = 	EEPROM.read(16);

		config.daylight = EEPROM.read(17);

		config.Update_Time_Via_NTP_Every = EEPROM.read(18); // 4 Byte

		config.timezone = EEPROMReadlong(19); // 4 Byte

		/*config.LED_R = EEPROM.read(26);
		config.LED_G = EEPROM.read(27);
		config.LED_B = EEPROM.read(28);*/

		config.IP[0] = EEPROM.read(20);
		config.IP[1] = EEPROM.read(21);
		config.IP[2] = EEPROM.read(22);
		config.IP[3] = EEPROM.read(23);
		config.Netmask[0] = EEPROM.read(24);
		config.Netmask[1] = EEPROM.read(25);
		config.Netmask[2] = EEPROM.read(26);
		config.Netmask[3] = EEPROM.read(27);
		config.Gateway[0] = EEPROM.read(28);
		config.Gateway[1] = EEPROM.read(29);
		config.Gateway[2] = EEPROM.read(30);
		config.Gateway[3] = EEPROM.read(31);
		EEPROM.get(32, config.ssid);
		EEPROM.get(64, config.password);
		EEPROM.get(96, config.ntpServerName);
		/*config.AutoTurnOn = EEPROM.read(300);
		config.AutoTurnOff = EEPROM.read(301);
		config.TurnOnHour = EEPROM.read(302);
		config.TurnOnMinute = EEPROM.read(303);
		config.TurnOffHour = EEPROM.read(304);
		config.TurnOffMinute = EEPROM.read(305);*/
		EEPROM.get(306, config.DeviceName);
		//config.moduleId = EEPROM.read(338);
		//config.state =  EEPROM.read(339);
		#endif
		return true;		
	}
	else
	{
		wserial.println("Configurarion NOT FOUND!!!!");
		return false;
	}
}

/*
**
**  NTP 
**
*/

void NTPRefresh() {
	String tmp = String(fifoq.getCount());
    wserial.println(tmp);

	if (WiFi.status() == WL_CONNECTED)
	{
		IPAddress timeServerIP; 
		WiFi.hostByName(config.ntpServerName, timeServerIP); 
		//sendNTPpacket(timeServerIP); // send an NTP packet to a time server


		wserial.println("sending NTP packet...");
		memset(packetBuffer, 0, NTP_PACKET_SIZE);
		packetBuffer[0] = 0b11100011;   // LI, Version, Mode
		packetBuffer[1] = 0;     // Stratum, or type of clock
		packetBuffer[2] = 6;     // Polling Interval
		packetBuffer[3] = 0xEC;  // Peer Clock Precision
		packetBuffer[12]  = 49;
		packetBuffer[13]  = 0x4E;
		packetBuffer[14]  = 49;
		packetBuffer[15]  = 52;
		UDPNTPClient.beginPacket(timeServerIP, 123); 
		UDPNTPClient.write(packetBuffer, NTP_PACKET_SIZE);
		UDPNTPClient.endPacket();


		delay(1000);
  
		int cb = UDPNTPClient.parsePacket();
		if (!cb) {
			wserial.println("NTP no packet yet");
		}
		else 
		{
			wserial.print("NTP packet received, length=");
			wserial.println(String(cb));
			UDPNTPClient.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			const unsigned long seventyYears = 2208988800UL;
			unsigned long epoch = secsSince1900 - seventyYears;
			UnixTimestamp = epoch;
      ntpSyncd = true;
		}
	}
}

void Second_Tick()
{
	strDateTime2 tempDateTime;
	AdminTimeOutCounter++;
	cNTP_Update++;
	UnixTimestamp++;
	ConvertUnixTimeStamp(UnixTimestamp +  ((int)config.timezone *  360) , &tempDateTime);
	if (config.daylight) // Sommerzeit beachten
		if (summertime(tempDateTime.year,tempDateTime.month,tempDateTime.day,tempDateTime.hour,0))
		{
			ConvertUnixTimeStamp(UnixTimestamp +  ((int)config.timezone *  360) + 3600, &DateTime);
		}
		else
		{
			DateTime = tempDateTime;
		}
	else
	{
			DateTime = tempDateTime;
	}
	Refresh = true;
}
 
void printDirectory(File dir, int numTabs) {
	while (true) {

		File entry =  dir.openNextFile();
		if (! entry) {
			// no more files
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++) {
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory()) {
			Serial.println("/");
			printDirectory(entry, numTabs + 1);
		} else {
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}
}
