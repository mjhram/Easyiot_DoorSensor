#ifndef SENSOR_H
#define SENSOR_H

#define ACCESS_POINT_NAME  "ESP_Test"        
#define ACCESS_POINT_PASSWORD  "123456" 
#define AdminTimeOut 3600  // Defines the Time in Seconds, when the Admin-Mode will be diabled
const char* http_username = "admin";
const char* http_password = "admin";

#define EIOTCLOUD_MODULEID 4
#define EIOTCLOUD_USERNAME "mjhram"
#define EIOTCLOUD_PASSWORD "passinto"
#define EIOT_CLOUD_ADDRESS "cloud.iot-playground.com"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
SimpleTimer timer;

#define AP_CONNECT_TIME 20 //sec. Turn to AP if not connected to STA in xx sec
int state = 0;  //0: Normal working, LED=>OFF
                //1: WiFi not connected (work as AP), Blink 1.2/1.2
                //2: MQTT not connected. Blink .4/2
struct EventStruct {
  int state;
  long time;
};
volatile EventStruct doorEvent;
volatile bool newEvent, isNotifying; 

const int buttonPin = D1;//D3; //D3 is flash Button
const int outPin = D4;  //D4 == 2 is LED

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  if(!mqttClient.connected()) {
      mqttClient.connect();
      state = 1;
  } else {
    state = 0;  
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  /*uint16_t packetIdSub = mqttClient.subscribe("/4/Sensor.Parameter1", 0);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  
  uint16_t packetIdSub = mqttClient.subscribe("/NewModule", 0);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("/4/Sensor.Parameter1", 0, true, "0");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  //uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  //Serial.print("Publishing at QoS 2, packetId: ");
  //Serial.println(packetIdPub2);
  */
  state = 0;
}


void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
  state = 2;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void reconnectToWifi() {
  if(WiFi.isConnected() && WiFi.localIP().toString() != "0.0.0.0") {
    return;
  }
  Serial.println("Connecting to Wi-Fi...");
  //WiFi.begin(config.ssid.c_str(), config.password.c_str());
  WiFi.reconnect();
}


void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(10, reconnectToWifi);
  state = 1;
}

bool ledState=false;
void repeat1s(){
  if (state==1) {
    ledState =!ledState;
    digitalWrite(outPin, ledState?HIGH:LOW);
    Serial.print("-");
  }
}

void handleInterrupt() {
  Serial.println("inteterrupt");
  doorEvent.state = digitalRead(buttonPin);
  doorEvent.time  = millis();
  newEvent = true;
      Serial.print("button state:");
      Serial.println(doorEvent.state);
}

void initSetup() {
  newEvent = false;
  isNotifying = false;
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(outPin, OUTPUT); 
  pinMode(D0, OUTPUT); digitalWrite(D0, HIGH); //will use it for reseting
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleInterrupt, CHANGE);
  digitalWrite(outPin, HIGH);

  int switchState = digitalRead(buttonPin);
  handleInterrupt(); //initially force send event 

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(EIOT_CLOUD_ADDRESS, 1883);
  mqttClient.setCredentials(EIOTCLOUD_USERNAME, EIOTCLOUD_PASSWORD);
}

void setupTelnet() {
  if (MDNS.begin(HOST_NAME)) {
        Serial.print("* MDNS responder started. Hostname -> ");
        Serial.println(HOST_NAME);
    }
    MDNS.addService("telnet", "tcp", 23);
    Debug.begin(HOST_NAME); // Initiaze the telnet server
    Debug.setResetCmdEnabled(true);
}

void Repeate5m() {
  handleInterrupt();
}

void setup_wifi() {
  delay(10);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid.c_str(), config.password.c_str());
  
  Serial.println("Connecting to WiFi");
  long time1 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis()-time1 < AP_CONNECT_TIME*1000)) {
    delay(1000);
    Serial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME);
  } else {
    //WiFi is connected=>force onWifiConnect
    WiFiEventStationModeGotIP dummy;
    onWifiConnect(dummy);
  }
  
  Serial.println(WiFi.localIP());
  timer.setInterval(5*60*000, Repeate5m);
}

#endif
