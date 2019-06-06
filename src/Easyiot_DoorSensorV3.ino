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
#include "wSerial.h"

#define RefreshTimerIsOn  false
RemoteDebug Debug;
wSerial wserial(Debug);
#include "global.h"
//Telent
//#include "RemoteDebug.h"
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ThingSpeak.h>

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
#include "PAGE_ForceEvent.h"

#include "example.h"

WiFiClient client;
unsigned long myChannelNumber = 772321;
const char * myWriteAPIKey = "1L8YUJBDUEJ8OFMR";

void setup(){
  EEPROM.begin(512);
  wserial.begin(115200);
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
    wserial.println("General config applied");
  }
  /*
  //ConfigureWifi();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid.c_str(), config.password.c_str());
  
  wserial.println("Connecting to WiFi");
  long time1 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis()-time1 < 20000)) {
    delay(1000);
    wserial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME);
  }
  
  wserial.println(WiFi.localIP());
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
    
  server.on ( "/favicon.ico",   [](AsyncWebServerRequest *server) { wserial.println("favicon.ico"); 
    server->send ( SPIFFS, "/favicon.ico" );  
     
  }  );

   server.on ( "/admin.html", [](AsyncWebServerRequest *server) { 
    wserial.println("admin.html"); 
    server->send ( 200, "text/html", PAGE_AdminMainPage );   
    }  );
    server.on ( "/forceEvent.html", [](AsyncWebServerRequest *server) { 
      wserial.println("forceEvent.html"); 
      send_force_event_html(server);  
    }  );
    server.on ( "/force/opendoor", [](AsyncWebServerRequest *request) { 
    wserial.println("/force/opendoor"); 
    send_force_open_html(request);
    }  );
    server.on ( "/force/mqttconnect", [](AsyncWebServerRequest *request) { 
    wserial.println("/force/mqttconnect"); 
    send_force_mqttconnect_html(request);
    }  );
    
  server.on ( "/config.html", [](AsyncWebServerRequest *request) { 
    wserial.println("config.html"); 
    send_network_configuration_html(request);
    }  );
  server.on ( "/admin/values", [](AsyncWebServerRequest *request) { 
    wserial.println("values.html"); 
    send_network_configuration_values_html(request);
    }  );
   server.on ( "/admin/connectionstate", [](AsyncWebServerRequest *request) { 
    wserial.println("connectionstate.html"); 
    send_connection_state_values_html(request);
    }  );
    server.on ( "/admin/disarm", [](AsyncWebServerRequest *request) { 
    wserial.println("connectionstate.html"); 
    send_disarm_values_html(request);
    }  );


  server.on ( "/info.html", [](AsyncWebServerRequest *request) { 
    wserial.println("info.html"); 
    request->send ( 200, "text/html", PAGE_Information );   
    }  );
  server.on ( "/admin/infovalues", [](AsyncWebServerRequest *request) { 
    send_information_values_html(request);
  });
  
  server.on ( "/ntp.html", [](AsyncWebServerRequest *request) { 
    wserial.println("ntp.html"); 
    send_NTP_configuration_html(request);   
    }  );
  server.on ( "/admin/ntpvalues", [](AsyncWebServerRequest *request) { 
    wserial.println("ntp.html"); 
    send_NTP_configuration_values_html(request);   
    }  );

  server.on ( "/general.html", [](AsyncWebServerRequest *request) { 
    wserial.println("general.html"); 
    send_general_html(request);   
    }  );
    server.on ( "/admin/generalvalues", [](AsyncWebServerRequest *request) { 
    wserial.println("general.html"); 
    send_general_configuration_values_html(request);   
    }  );
    server.on ( "/admin/devicename",     [](AsyncWebServerRequest *request) { 
    wserial.println("general.html"); 
    send_devicename_value_html(request);   
    }  );



  server.on ( "/admin/filldynamicdata",  [](AsyncWebServerRequest *request){
    filldynamicdata(request);
    });
  server.on ( "/style.css", [](AsyncWebServerRequest *request) { 
    wserial.println("style.css"); 
    request->send ( 200, "text/plain", PAGE_Style_css );  
    } );
  server.on ( "/microajax.js", [](AsyncWebServerRequest *request) { 
    wserial.println("microajax.js"); 
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
    newEvent = false;
    isNotifying = false;

    DEBUG_V("DoorValue:%d\n", doorEvent.state);
    if(sendIfttt) {
        runAsyncClient();   
    }
    String valueStr = String(doorEvent.state);
    /*//publish to easyiot
    topic  = "/"+String(config.moduleId)+ "/Sensor.Parameter1";
    result = mqttClient.publish(topic.c_str(), 0, true, valueStr.c_str(), true); 
    */
    //data string
    String data="";
    if(doorEvent.pinIdx==-1) {
        for(int k =0; k<NPINS; k++) {
          if(k!=0) data = data+"&";
          data = data+ String("field"+String(k+1)+"=" + valueStr);
        }
    } else {
        data = String("field"+String(doorEvent.pinIdx+1)+"=" + valueStr);
    }
    /*int length = data.length();
    char msgBuffer[length];
    data.toCharArray(msgBuffer,length+1);
    wserial.println(data);*/
    //topic string
    String topicString ="channels/" + String( myChannelNumber ) + "/publish/"+String(myWriteAPIKey);
    /*length=topicString.length();
    char topicBuffer[length];
    topicString.toCharArray(topicBuffer,length+1);*/
    result = mqttClient.publish(topicString.c_str(), 0, false, data.c_str()); 
    DEBUG_V("publishing...\n");
    wserial.println(data);
    /*int x = ThingSpeak.writeField(myChannelNumber, 1, doorEvent.state, myWriteAPIKey);
    if(x == 200){
      wserial.println("Channel update successful.");
      DEBUG_V("Channel update successful\n");
    }
    else{
      wserial.println("Problem updating channel. HTTP error code " + String(x));
      DEBUG_V("Problem updating channel\n");
    }*/        
    wserial.print("Publish ");
    wserial.print(topic);
    wserial.print(" ");
    wserial.println(valueStr);
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
          wserial.println("SwitchON");
       }
     }


     Minute_Old = DateTime.minute;
     if (config.AutoTurnOff)
     {
       if (DateTime.hour == config.TurnOffHour && DateTime.minute == config.TurnOffMinute)
       {
          wserial.println("SwitchOff");
       }
     }
  }

  if (Refresh)  
  {
    mTimeSeconds++;
    Refresh = false;
    if(RefreshTimerIsOn) {
      wserial.print(".");
      //wserial.println("Refreshing...");
      //wserial.printf("FreeMem:%d %d:%d:%d %d.%d.%d \n",ESP.getFreeHeap() , DateTime.hour,DateTime.minute, DateTime.second, DateTime.year, DateTime.month, DateTime.day);
      DEBUG_V("* Time: %u seconds (VERBOSE)\n", mTimeSeconds);
    }

  }

  if(!WiFi.isConnected()) {
    wifiReconnectTimer.once(30, reconnectToWifi);
  }else if(WiFi.isConnected() && mqttClient.connected() && newEvent && !isNotifying && !disarm) {
    isNotifying = true;
    timer.setTimeout(2000, runOnce);
    wserial.println("New Event...");
  } 
  Debug.handle();
  timer.run();
}

static AsyncClient * aClient = NULL;

void runAsyncClient(){
  //if(aClient)//client already exists
    //return;
  wserial.println("runAsyncClient()");
  aClient = new AsyncClient();
  if(!aClient){
    //could not allocate client
    DEBUG_V("could not allocate client");
    return;
  }
  aClient->onError([](void * arg, AsyncClient * client, err_t error){
    wserial.println("Connect Error");
    DEBUG_V("Connect Error\n");
    aClient = NULL;
    delete client;
  }, NULL);

  aClient->onConnect([](void * arg, AsyncClient * client){
    wserial.println("Connected");
    DEBUG_V("connected");
    aClient->onError(NULL, NULL);

    client->onDisconnect([](void * arg, AsyncClient * c){
      wserial.println("Disconnected");
      DEBUG_V("disconnected");
      aClient = NULL;
      delete c;
    }, NULL);

    client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
      wserial.print("\r\nData: ");
      wserial.println(String(len));
      uint8_t * d = (uint8_t*)data;
      for(size_t i=0; i<len;i++)
        wserial.write(d[i]);
    }, NULL);

    //send the request
    client->write("GET /trigger/door_closed/with/key/c3znCYclNej6D1b7JWlFBS HTTP/1.0\r\nHost: maker.ifttt.com\r\n\r\n");
    DEBUG_V("IFTTT triggered\n");
    sendIfttt = false;//it is false once IFTTT is triggered.
  }, NULL);
  
  if(!aClient->connect("maker.ifttt.com", 80)){
    wserial.println("Connect Fail");
    DEBUG_V("Connect Fail");
    AsyncClient * client = aClient;
    aClient = NULL;
    delete client;
  }
}

