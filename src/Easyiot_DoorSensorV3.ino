#include <ESP8266WiFi.h>
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <SimpleTimer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include "helpers.h"
#include "global.h"
//Telent
#include "RemoteDebug.h"
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ThingSpeak.h>

RemoteDebug Debug;
#define HOST_NAME "remotedebug-sample"

#include "sensor.h"

#include "Page_Admin.h"
//#include "Page_Root.h"

#include "Page_Script.js.h"
#include "Page_Style.css.h"
#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"

#include "example.h"

WiFiClient client;
unsigned long myChannelNumber = 772321;
const char * myWriteAPIKey = "1L8YUJBDUEJ8OFMR";

void setup(){
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(500);
  initSetup();
  if (!ReadConfig())
  {
    // DEFAULT CONFIG
    config.ssid = "MJH_MIFI";
    config.password = "2013Dgroup2";
    config.dhcp = true;
    config.IP[0] = 192;config.IP[1] = 168;config.IP[2] = 1;config.IP[3] = 100;
    config.Netmask[0] = 255;config.Netmask[1] = 255;config.Netmask[2] = 255;config.Netmask[3] = 0;
    config.Gateway[0] = 192;config.Gateway[1] = 168;config.Gateway[2] = 1;config.Gateway[3] = 1;
    config.ntpServerName = "0.de.pool.ntp.org";
    config.Update_Time_Via_NTP_Every =  0;
    config.timezone = +3;
    config.daylight = false;
    config.DeviceName = "ESP Test";
    config.AutoTurnOff = false;
    config.AutoTurnOn = false;
    config.TurnOffHour = 0;
    config.TurnOffMinute = 0;
    config.TurnOnHour = 0;
    config.TurnOnMinute = 0;
    
    config.moduleId = EIOTCLOUD_MODULEID;
    config.state = 0; // off
    
    WriteConfig();
    Serial.println("General config applied");
  }
  /*
  //ConfigureWifi();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid.c_str(), config.password.c_str());
  
  Serial.println("Connecting to WiFi");
  long time1 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis()-time1 < 20000)) {
    delay(1000);
    Serial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME);
  }
  
  Serial.println(WiFi.localIP());
  */
  setup_wifi();
  ThingSpeak.begin(client);
  setupTelnet();
  SPIFFS.begin();

  server.addHandler(new SPIFFSEditor(http_username,http_password));
  server.on("/hello", [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
    });
  server.on ( "/", [](AsyncWebServerRequest *request){
    request->redirect("/admin.html");
    //processExample(request);
    });
    
  server.on ( "/favicon.ico",   [](AsyncWebServerRequest *server) { Serial.println("favicon.ico"); 
    server->send ( SPIFFS, "/favicon.ico" );  
     
  }  );

   server.on ( "/admin.html", [](AsyncWebServerRequest *server) { 
    Serial.println("admin.html"); 
    server->send ( 200, "text/html", PAGE_AdminMainPage );   
    }  );

  server.on ( "/config.html", [](AsyncWebServerRequest *request) { 
    Serial.println("config.html"); 
    send_network_configuration_html(request);
    }  );
  server.on ( "/admin/values", [](AsyncWebServerRequest *request) { 
    Serial.println("values.html"); 
    send_network_configuration_values_html(request);
    }  );
   server.on ( "/admin/connectionstate", [](AsyncWebServerRequest *request) { 
    Serial.println("connectionstate.html"); 
    send_connection_state_values_html(request);
    }  );
    server.on ( "/admin/disarm", [](AsyncWebServerRequest *request) { 
    Serial.println("connectionstate.html"); 
    send_disarm_values_html(request);
    }  );


  server.on ( "/info.html", [](AsyncWebServerRequest *request) { 
    Serial.println("info.html"); 
    request->send ( 200, "text/html", PAGE_Information );   
    }  );
  server.on ( "/admin/infovalues", [](AsyncWebServerRequest *request) { 
    send_information_values_html(request);
  });
  
  server.on ( "/ntp.html", [](AsyncWebServerRequest *request) { 
    Serial.println("ntp.html"); 
    send_NTP_configuration_html(request);   
    }  );
  server.on ( "/admin/ntpvalues", [](AsyncWebServerRequest *request) { 
    Serial.println("ntp.html"); 
    send_NTP_configuration_values_html(request);   
    }  );

  server.on ( "/general.html", [](AsyncWebServerRequest *request) { 
    Serial.println("general.html"); 
    send_general_html(request);   
    }  );
    server.on ( "/admin/generalvalues", [](AsyncWebServerRequest *request) { 
    Serial.println("general.html"); 
    send_general_configuration_values_html(request);   
    }  );
    server.on ( "/admin/devicename",     [](AsyncWebServerRequest *request) { 
    Serial.println("general.html"); 
    send_devicename_value_html(request);   
    }  );



  server.on ( "/admin/filldynamicdata",  [](AsyncWebServerRequest *request){
    filldynamicdata(request);
    });
  server.on ( "/style.css", [](AsyncWebServerRequest *request) { 
    Serial.println("style.css"); 
    request->send ( 200, "text/plain", PAGE_Style_css );  
    } );
  server.on ( "/microajax.js", [](AsyncWebServerRequest *request) { 
    Serial.println("microajax.js"); 
    request->send ( 200, "text/plain", PAGE_microajax_js );  
    } );
  
  server.begin();

  tkSecond.attach(1,Second_Tick);
  UDPNTPClient.begin(2390);  // Port for NTP receive

  setupOTA();
}


void runOnce() {
    boolean result;
    String topic("");
    String valueStr("");
    newEvent = false;
    isNotifying = false;
    {
        valueStr = String(doorEvent.state);
        /*topic  = "/"+String(config.moduleId)+ "/Sensor.Parameter1";
        result = mqttClient.publish(topic.c_str(), 0, true, valueStr.c_str(), true); 
        */
        int x = ThingSpeak.writeField(myChannelNumber, 1, doorEvent.state, myWriteAPIKey);
        if(x == 200){
          Serial.println("Channel update successful.");
          DEBUG_V("Channel update successful\n");
        }
        else{
          Serial.println("Problem updating channel. HTTP error code " + String(x));
          DEBUG_V("Problem updating channel\n");
        }
        if(doorEvent.state==1) {
            runAsyncClient();   
        }
        Serial.print("Publish ");
        Serial.print(topic);
        Serial.print(" ");
        Serial.println(valueStr);
    }
}

long mTimeSeconds =0;
void loop(){
  ArduinoOTA.handle();
  if (config.Update_Time_Via_NTP_Every  > 0 )
  {
    if (cNTP_Update > 5 && firstStart)
    {
      NTPRefresh();
      cNTP_Update =0;
      firstStart = false;
    }
    else if ( cNTP_Update > (config.Update_Time_Via_NTP_Every * 60) )
    {

      NTPRefresh();
      cNTP_Update =0;
    }
  }

  if(DateTime.minute != Minute_Old)
  {
     Minute_Old = DateTime.minute;
     if (config.AutoTurnOn)
     {
       if (DateTime.hour == config.TurnOnHour && DateTime.minute == config.TurnOnMinute)
       {
          Serial.println("SwitchON");
       }
     }


     Minute_Old = DateTime.minute;
     if (config.AutoTurnOff)
     {
       if (DateTime.hour == config.TurnOffHour && DateTime.minute == config.TurnOffMinute)
       {
          Serial.println("SwitchOff");
       }
     }
  }

  if (Refresh)  
  {
    mTimeSeconds++;
    Refresh = false;
    Serial.print(".");
    //Serial.println("Refreshing...");
    //Serial.printf("FreeMem:%d %d:%d:%d %d.%d.%d \n",ESP.getFreeHeap() , DateTime.hour,DateTime.minute, DateTime.second, DateTime.year, DateTime.month, DateTime.day);
    DEBUG_V("* Time: %u seconds (VERBOSE)\n", mTimeSeconds);

  }

  if(!WiFi.isConnected()) {
    wifiReconnectTimer.once(30, reconnectToWifi);
  }else if(WiFi.isConnected() && mqttClient.connected() && newEvent && !isNotifying && !disarm) {
    isNotifying = true;
    timer.setTimeout(2000, runOnce);
    Serial.println("New Event...");
  } 
  Debug.handle();
  timer.run();
}

static AsyncClient * aClient = NULL;

void runAsyncClient(){
  if(aClient)//client already exists
    return;

  aClient = new AsyncClient();
  if(!aClient)//could not allocate client
    return;

  aClient->onError([](void * arg, AsyncClient * client, int error){
    Serial.println("Connect Error");
    aClient = NULL;
    delete client;
  }, NULL);

  aClient->onConnect([](void * arg, AsyncClient * client){
    Serial.println("Connected");
    aClient->onError(NULL, NULL);

    client->onDisconnect([](void * arg, AsyncClient * c){
      Serial.println("Disconnected");
      aClient = NULL;
      delete c;
    }, NULL);

    client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
      Serial.print("\r\nData: ");
      Serial.println(len);
      uint8_t * d = (uint8_t*)data;
      for(size_t i=0; i<len;i++)
        Serial.write(d[i]);
    }, NULL);

    //send the request
    client->write("GET /trigger/door_closed/with/key/nATYSPRWzpXrZ341TzTSf HTTP/1.0\r\nHost: maker.ifttt.com\r\n\r\n");
  }, NULL);

  if(!aClient->connect("maker.ifttt.com", 80)){
    Serial.println("Connect Fail");
    AsyncClient * client = aClient;
    aClient = NULL;
    delete client;
  }
}


