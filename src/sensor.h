#ifndef SENSOR_H
#define SENSOR_H

#define ACCESS_POINT_NAME  "ESP_Test"        
#define ACCESS_POINT_PASSWORD  "123456" 
#define AdminTimeOut 3600  // Defines the Time in Seconds, when the Admin-Mode will be diabled
//const char* http_username = "admin";
//const char* http_password = "admin";


//#define EIOTCLOUD_MODULEID 4
/*
#define MQTT_USERNAME "mjhram"
#define MQTT_PASSWORD "passinto"
#define MQTT_ADDRESS "cloud.iot-playground.com"

#define MQTT_CLIENTID "MQTTTEST2"
#define MQTT_USERNAME "TSArduinoMQTTDemo"
#define MQTT_PASSWORD "Y3OP4BB8DOEOA33G"
#define MQTT_ADDRESS "mqtt.thingspeak.com"
*/
#define MQTT_CLIENTID "MQTTTEST2"
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_ADDRESS IPAddress(192, 168, 0, 105)

long subscribeTime = -1;
uint16_t packetIdSub=0;
unsigned long myChannelNumber = 772321;
const char * myWriteAPIKey = "1L8YUJBDUEJ8OFMR";
const char * myReadAPIKey = "9YBUC8XF4YQMPNA1";

AsyncMqttClient mqttClient;
//Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
SimpleTimer timer;

#define AP_CONNECT_TIME 20 //sec. Turn to AP if not connected to STA in xx sec
int state = 0;  //0: Normal working, LED=>OFF
                //1: WiFi not connected (work as AP), Blink 1.2/1.2
                //2: MQTT not connected. Blink .4/2

static AsyncClient *aClient2 = NULL;

/*void sendIfttt_espOn(){
  wserial.println("sendIfttt_espOn()");
  aClient2 = new AsyncClient();
  if(!aClient2){
    //could not allocate client
    wserial.println("could not allocate client2");
    return;
  }
  aClient2->onError([](void * arg, AsyncClient * client, err_t error){
    wserial.println("Connect Error-ifttt espon");
    aClient2 = NULL;
    delete client;
  }, NULL);

  aClient2->onConnect([](void * arg, AsyncClient * client){
    wserial.println("ifttt-espon-Connected");
    aClient2->onError(NULL, NULL);

    client->onDisconnect([](void * arg, AsyncClient * c){
      wserial.println("ifttt-esponDisconnected");
      aClient2 = NULL;
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
    client->write("GET /trigger/esp_on/with/key/02k7jTN4K_wnu4pv4j7tR HTTP/1.0\r\nHost: maker.ifttt.com\r\n\r\n");
    wserial.println("ifttt-espon triggered\n");
    //sendEspOn = false;//it is false once IFTTT is triggered.
  }, NULL);
  
  if(!aClient2->connect("maker.ifttt.com", 80)){
    wserial.println("sendIfttt_espOn-Connect Fail");
    AsyncClient * client = aClient2;
    aClient2 = NULL;
    delete client;
  }
}*/

void publish2thingspeak(){
  EventsArray events;
  //read analog A0:
  int sensorValue = analogRead(A0);
  String tmpstr = "Analog Val:"+String(sensorValue);
  wserial.println(tmpstr);

  if(publishq.peek(&events)==false) return;
  wserial.print("publish2thingspeak");
  if(!mqttClient.connected()) {
    wserial.println("-> MQTT not connected");
    return;
  }else{
    wserial.println("...");
  }
  if(notificationTime - millis() < 5000) {//make sure that there is a gap time from last sent notification 
    String tmp = String("notification in process:") + String(notificationTime);
    wserial.println(tmp);
    return;
  }
  publishTime = millis();
  String data="";    
  for(int k=0; k<events.size; k++) {
    wserial.println(events.events[k].toString());
    int a = events.events[k].trigger;
    if(sensorPinsInvert[events.events[k].pinIdx]==1) {
        //invert
        a = (a==1)?0:1;
    }
    String valueStr = String(a);//a?"1":"0";
    int tmp = events.events[k].pinIdx+1;
    //bool a = (sensorPinsInvert[event.pinIdx]==1)?~event.trigger:event.trigger;//mistake in use ~(bitwise negation) while ! should be used
    
    if(k != 0) {
      data+=String("&");
    }
    data += String("field"+String(tmp)+"=" + valueStr);  
  }
  if(DateTime.year > 2000) {
      String dt = (String)DateTime.year+ "-" +(String)DateTime.month+ "-" +(String)DateTime.day
      + " " + (String)DateTime.hour+":"+(String)+ DateTime.minute+":"+(String)DateTime.second;
      
      data += String("&created_at=")+dt;
      data += String("&timezone=Asia/Baghdad");
    }  
  String topicString ="channels/" + String( myChannelNumber ) + "/publish/"+String(myWriteAPIKey);
  int result = mqttClient.publish(topicString.c_str(), 0, false, data.c_str(), data.length()); 
  if(result == 1) {
    publishq.pop(&events);
  } else {
    wserial.println("event not published => kept in Queue.");
  }
  //wserial.print("publishing...");
  wserial.print("Topic:");
  wserial.print(topicString);
  wserial.print(" -Data:");
  wserial.print(data);  
  wserial.print(" -Returned:");
  wserial.println(String(result));
}

#define dupEvDuration  2000

EventStruct handleInterruptQ(int pinIdx, bool dontPublish=false) {
  EventStruct event;
  
  if(pinIdx == -1) {
    EventsArray events(NPINS);
    wserial.println("check all pins (handleinterrupt(-1)...");
    for(int kk=0; kk<NPINS;kk++){
        event = handleInterruptQ(kk, true);
        events.events[kk]=event;
    }
    publishq.push(&events);
    return event;
  }
  if(pinIdx <0 || pinIdx >= NPINS){
    return event;
  }
  wserial.println("\r\nqueue inteterrupt");
  event.pinIdx = pinIdx;
  event.type = eventType[pinIdx];
  event.time  = millis();
  event.timestamp = UnixTimestamp;
  String tmp = "time:"+String(event.time);  
  wserial.println(tmp);
  event.trigger = 0;
  wserial.print("Pins:");
  wserial.println(String(pinIdx));
  
  if(event.type == AnalogEvent) {
    wserial.println("...Analog intr");
    event.trigger = analogRead(sensorPins[pinIdx]);
    return event;
  } else {
    #if USEMCP
    int a = mcp.digitalRead(sensorPins[pinIdx]);
    #else
    int a = digitalRead(sensorPins[pinIdx]);
    #endif
    if(sensorPinsInvert[pinIdx]==0){
        event.trigger = a;
    }else{
        event.trigger = (a==1)?0:1;
    }
  }  
  //if(doPublish) 
  /*{
    EventsArray evarr(1);
    evarr.events[0] = event;
    //publish2thingspeak(event);
    publishq.push(&event);
  }*/
  if(lastEvent[event.pinIdx].time != -1 && event.time-lastEvent[event.pinIdx].time <= dupEvDuration) {
      //fifoq.pop(&ev);
      wserial.println("duplicate event->neglect");
      return event; //duplicated event within dupEvDuration
  } 
  if(event.trigger == 0) {
    wserial.println("false interrupt->neglect");
    return event; //false trigger!
  }
  lastEvent[event.pinIdx] = event;
  fifoq.push(&event);
  //to publish:
  if(!dontPublish) {
    EventsArray events(1);

    events.events[0]=event;
    publishq.push(&events);
    wserial.println("event to be published.");
  }
  wserial.print("button state:");
  wserial.println(String(event.trigger));
  return event;
}

#if USEMCP
volatile uint16_t dmy; 
ICACHE_RAM_ATTR void handleMcpInterrupt() {
  uint8_t p,v;
   Serial.println("<McpInterrupt>");
   // Debounce. Slow I2C: extra debounce between interrupts anyway.
   // Can not use delay() in interrupt code.
   noInterrupts();
   delayMicroseconds(1000); 
    
    // Stop interrupts from external pin.
   detachInterrupt(digitalPinToInterrupt(IntPin));
   interrupts(); // re-start interrupts for mcp
   p = mcp.getLastInterruptPin();
   // This one resets the interrupt state as it reads from reg INTCAPA(B).
   v = mcp.getLastInterruptPinValue();
   //dmy = mcp.readGPIOAB();
  String tmp = String("Intr on Pin:")+String(p)+String(".");
  wserial.println(tmp);
  attachMcpInterrupt;

  if((sensorPinMode[p]==RISING && v==1) || (sensorPinMode[p]==FALLING && v==0)) {
    handleInterruptQ(p);
  } else {
    wserial.println("Dont handled MCP Intr");
  }
}

#else
ICACHE_RAM_ATTR void handleInterrupt1() {
  handleInterruptQ(0);
}
ICACHE_RAM_ATTR void handleInterrupt2() {
  handleInterruptQ(1);
}
ICACHE_RAM_ATTR void handleInterrupt3() {
  handleInterruptQ(2);
}
ICACHE_RAM_ATTR void handleInterrupt4() {
  handleInterruptQ(3);
}
ICACHE_RAM_ATTR void handleInterruptEmpty() {
  
}
//const int buttonPin = D1;//D3; //D3 is flash Button
ICACHE_RAM_ATTR void (* handleInt[NPINS])() ={handleInterrupt1, handleInterrupt2, handleInterrupt3, handleInterruptEmpty};//, handleInterrupt3};
#endif

long mqttIsConnecting = 0;
void connectToMqtt() {
  mqttIsConnecting = millis();
  
  if(!mqttClient.connected()) {
      wserial.println("Connecting to MQTT...");
      mqttClient.setMaxTopicLength(200);
      mqttClient.connect();
      state = 1;
  } else {
    wserial.println("MQTT already connected.");
    state = 0;  
  }
}

void onMqttConnect(bool sessionPresent) {
  wserial.println("Connected to MQTT.");
  wserial.print("Session present: ");
  wserial.println(sessionPresent?"True":"False");
  
  String tmp = String("channels/772321/subscribe/fields/field5/")+String(myReadAPIKey);
  packetIdSub = mqttClient.subscribe(tmp.c_str(), 0);
  wserial.print("Subscribing packetId: ");
  wserial.println(String(packetIdSub));
  /*uint16_t packetIdSub = mqttClient.subscribe("/4/Sensor.Parameter1", 0);
  wserial.print("Subscribing at QoS 2, packetId: ");
  wserial.println(packetIdSub);
  
  uint16_t packetIdSub = mqttClient.subscribe("/NewModule", 0);
  wserial.print("Subscribing at QoS 2, packetId: ");
  wserial.println(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  wserial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("/4/Sensor.Parameter1", 0, true, "0");
  wserial.print("Publishing at QoS 1, packetId: ");
  wserial.println(packetIdPub1);
  //uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  //wserial.print("Publishing at QoS 2, packetId: ");
  //wserial.println(packetIdPub2);
  */

  state = 0;
}


void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  wserial.print("Disconnected from MQTT.");
  wserial.println(String("Reason:")+String(static_cast<uint8_t>(reason)));
  /*if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }*/
  state = 2;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  wserial.println("Subscribe acknowledged.");
  wserial.print("  packetId: ");
  wserial.println(String(packetId));
  wserial.print("  qos: ");
  wserial.println(String(qos));

  subscribeTime = millis();
}

void onMqttUnsubscribe(uint16_t packetId) {
  wserial.println("Unsubscribe acknowledged.");
  wserial.print("  packetId: ");
  wserial.println(String(packetId));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  wserial.println("Publish received.");
  wserial.print("  topic: ");
  wserial.println(topic);
  wserial.print("  qos: ");
  wserial.println(String(properties.qos));
  wserial.print("  dup: ");
  wserial.println(String(properties.dup));
  wserial.print("  retain: ");
  wserial.println(String(properties.retain));
  wserial.print("  len: ");
  wserial.println(String(len));
  wserial.print("  index: ");
  wserial.println(String(index));
  wserial.print("  total: ");
  wserial.println(String(total));
  wserial.print("  payload: ");
  char c[len+1];
  strncpy(c, payload,len);
  c[len]='\0';
  String cmd(c);
  wserial.println(cmd);
  long delta = 0;
  if(subscribeTime < 0) {
    delta = 0;
  } else {
    delta = millis()-subscribeTime;
  }
  if(delta < 5000) {
    
  } else {
    wserial.print("subTimeDelta:");
    wserial.println(String(delta));
    //process commands
    //111 == reset
    if(cmd == "111") {
      wserial.print("restarting in few sec...");
      //delay(5000);
      //ESP.restart();
      watchdogup = 2;//to restart
    }
  }
}

void onMqttPublish(uint16_t packetId) {
  wserial.println("Publish acknowledged.");
  wserial.print("  packetId: ");
  wserial.println(String(packetId));
}

bool wifiIsConnecting = false;
void reconnectToWifi() {
  wifiIsConnecting = false;
  if(WiFi.isConnected() && WiFi.localIP().toString() != "0.0.0.0") {
    return;
  }
  wserial.println("Connecting to Wi-Fi...");
  //WiFi.begin(config.ssid.c_str(), config.password.c_str());
  WiFi.reconnect();
}


void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  wserial.println("Connected to Wi-Fi.");
  WiFi.config(WiFi.localIP(),WiFi.gatewayIP(),WiFi.subnetMask(),WiFi.dnsIP(), IPAddress(8,8,8,8));
  
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  wserial.println("Disconnected from Wi-Fi.");
  //mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(10, reconnectToWifi);
  state = 1;
}

void repeat1s(){
  if (state==1) {
    wserial.print("-");
  }
}

void processCmdRemoteDebug() {

	String lastCmd = Debug.getLastCommand();

	if (lastCmd == "clrq") {

		// Clear Queue

		debugA("* Queue Cleared");
    fifoq.clean();
	} else if (lastCmd == "getq") {
		debugA("* Queue entries#: %d", fifoq.getCount());
	} else if (lastCmd == "getpq") {
		debugA("* Publish Queue entries#: %d", publishq.getCount());
	}else if (lastCmd == "close") {
    wserial.close();
	}else if (lastCmd == "open") {
    wserial.open();
	} else if (lastCmd == "cfg") {
    wserial.println(config2String(config));
	} else if (lastCmd == "getpins") {
    wserial.println("NA");
	} else if (lastCmd == "restart") {
    wserial.println("restarting...");
    watchdogup = 2;//to restart
	} 
}

void initSetup() {
  /*newEvent = false;
  isNotifying = false;
  sendIfttt = false;*/

  pinMode(WATCHDOG_PIN, OUTPUT);
  #if USEMCP
  pinMode(IntPin,INPUT);
  #endif
  for (int kk=0; kk<NPINS;kk++) {
    if(eventType[kk] == AnalogEvent) continue;
    if(sensorPinsInvert[kk]==0) {
      #if USEMCP
      mcp.pinMode(sensorPins[kk], INPUT);
      mcp.pullUp(sensorPins[kk],HIGH);     // Puled high to ~100k
      #else
      pinMode(sensorPins[kk], INPUT_PULLUP);
      #endif
    }else{
      #if USEMCP
      mcp.pinMode(sensorPins[kk], INPUT);
      #else
      pinMode(sensorPins[kk], INPUT);
      #endif
    }
    #if USEMCP
    mcp.setupInterruptPin(sensorPins[kk],CHANGE);
    #else
    attachInterrupt(digitalPinToInterrupt(sensorPins[kk]), handleInt[kk], sensorPinMode[kk]);
    #endif
  }
  #if USEMCP
  mcp.setupInterrupts(MCP_INT_MIRROR, MCP_INT_ODR, LOW);
  mcp.readGPIOAB(); // Initialise for interrupts.
  attachMcpInterrupt 
  #endif
  //attachInterrupt(digitalPinToInterrupt(buttonPin), handleInterrupt, CHANGE);
  //digitalWrite(outPin, HIGH);

  //int switchState = digitalRead(buttonPin);
  //handleInterrupt(-1); //initially force send event 
  handleInterruptQ(2); //force send power event

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setClientId(MQTT_CLIENTID);
  mqttClient.setServer(MQTT_ADDRESS, 1883);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
}

void setupTelnet() {
  if (MDNS.begin(HOST_NAME)) {
        wserial.print("* MDNS responder started. Hostname -> ");
        wserial.println(HOST_NAME);
    }
    MDNS.addService("telnet", "tcp", 23);
    Debug.begin(HOST_NAME); // Initiaze the telnet server
    Debug.setResetCmdEnabled(true);
    Debug.showColors(true);
    // Project commands

	String helpCmd = "clrq - Clear Queue\r\n";
	helpCmd.concat("getq - Get # Queue entries count\n");
  helpCmd.concat("getpq - Get # publish queue entries\n");
  helpCmd.concat("close - Close logfile\n");
  helpCmd.concat("open - Open logfile\n");
  helpCmd.concat("cfg - Get Settings\n");
  helpCmd.concat("getpins - Get Pins\n");
  helpCmd.concat("restart - Restarting\n");

	Debug.setHelpProjectsCmds(helpCmd);
	Debug.setCallBackProjectCmds(&processCmdRemoteDebug);
}

void Repeate5m() {
  handleInterruptQ(-1);
  #if USEMCP
  mcp.readGPIOAB();  //reset MCP interrupts:
  #endif
  
  String tmp="Status \nWIFI:";
  tmp += WiFi.isConnected()?"Connected":"Disconnected";
  tmp+=" \nMQTT:";
  tmp+=mqttClient.connected()?"Connected":"Disconnected";
  tmp+="\n deltaTime:";
  tmp+=String(millis()-mqttIsConnecting);
  wserial.println(tmp);
  tmp=String("Free Heap:")+ String(ESP.getFreeHeap());
  wserial.println(tmp);
  wserial.reopen();
  /*if(sendEspOn == true) {
    sendIfttt_espOn();
  }*/
}

void setup_wifi() {
  delay(10);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.password);
  
  wserial.println("Connecting to WiFi");
  long time1 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis()-time1 < AP_CONNECT_TIME*1000)) {
    delay(1000);
    wserial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME);
  } else {
    //WiFi is connected=>force onWifiConnect
    WiFiEventStationModeGotIP dummy;
    onWifiConnect(dummy);
  }
  
  wserial.println(WiFi.localIP().toString());
  timer.setInterval(300000, Repeate5m);
  timer.setInterval(31000, publish2thingspeak);
}

void setupOTA() {
  ArduinoOTA.setPort(8266);
  //ArduinoOTA.setHostname("myesp8266");
  ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    wserial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    wserial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    wserial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    wserial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) wserial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) wserial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) wserial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) wserial.println("Receive Failed");
    else if (error == OTA_END_ERROR) wserial.println("End Failed");
  });
  ArduinoOTA.begin();
}

#endif
