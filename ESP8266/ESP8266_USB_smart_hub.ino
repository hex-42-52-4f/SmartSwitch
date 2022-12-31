// YOU HAVE TO SET PUBSUBCLINET MAX LENGTH MESSAGE TO SOMETHING BIGGER
// https://roelofjanelsinga.com/articles/mqtt-discovery-with-an-arduino/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <credentials.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#define usb1_en 0
#define usb2_en 2
#define usb3_en 16
#define usb4_en 12

#define usb1_flg 4
#define usb2_flg 5
#define usb3_flg 14
#define usb4_flg 13

#define wifi_ssid mySSID
#define wifi_password myPASSWORD

#define mqtt_server "10.0.0.52"
#define mqtt_user myMqttName
#define mqtt_password myMqttPassword

String deviceName = "SmartSwitch2";
String visibleDeviceName = "Smart Switch myroom";

// ------------------ SETTING UP WIFI AND MQTT -----------------
WiFiClient espClinet;
PubSubClient client(espClinet);

void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ---------------- FUNCTIONS FOR MQTT DISCOVERY ---------------
void sendFirstDiscoveryMsg(const String deviceName, int usbNum) {
  //                        homeassistant/switch/SmartSwitch0_usb0/onfig
  String discoveryTopic = "homeassistant/switch/" + String(deviceName) + "_usb" + String(usbNum) + "/config";
  
  DynamicJsonDocument doc(4096);
  char buffer[1024];

  doc["~"] = "homeassistant/switch/" + String(deviceName) + "_usb" + String(usbNum);
  doc["name"] = String(deviceName) + " USB" + String(usbNum);
  doc["unique_id"] = String(deviceName) + "_usb" + String(usbNum);
  doc["state_topic"] = "~/state";
  doc["command_topic"] = "~/set";
  doc["schema"] = "json";
  doc["pl_on"] = "1";
  doc["pl_off"] = "0";
  doc["device"]["model"] = "ESP8266 based";
  doc["device"]["identifiers"] = String(deviceName);
  //doc["device"]["via_device"] = "ESP8266";
  doc["device"]["name"] = String(visibleDeviceName);
  doc["device"]["manufacturer"] = "Cincin";
  doc["device"]["hw"] = "0.1";
  doc["device"]["sw"] = "0.0.1";
  
  size_t n = serializeJson(doc, buffer);
  //Serial.println("");
  //Serial.print("size n:");
  //Serial.println(n);
  //client.publish(discoveryTopic.c_str(), buffer, n);


  if(!client.publish(discoveryTopic.c_str(), buffer, n)) {
    Serial.println("PUBLISHING FAILED!");
  } else {
    Serial.print("Sending discovery message to: ");
    Serial.println(discoveryTopic);  
  }

}

void sendDiscoveryMsg(const String deviceName, int usbNum) {
  String discoveryTopic = "homeassistant/switch/" + String(deviceName) + "_usb" + String(usbNum) + "/config";
  
  DynamicJsonDocument doc(512);
  char buffer[512];
  
  doc["~"] = "homeassistant/switch/" + String(deviceName) + "_usb" + String(usbNum);
  doc["name"] = String(deviceName) + " USB" + String(usbNum);
  doc["unique_id"] = String(deviceName) + "_usb" + String(usbNum);
  doc["state_topic"] = "~/state";
  doc["command_topic"] = "~/set";
  doc["schema"] = "json";
  doc["pl_on"] = "1";
  doc["pl_off"] = "0";
  doc["device"]["identifiers"] = String(deviceName);
  
  size_t n = serializeJson(doc, buffer);

  if(!client.publish(discoveryTopic.c_str(), buffer, n)) {
    Serial.println("PUBLISHING FAILED!");
  } else {
    Serial.print("Sending discovery message to: ");
    Serial.println(discoveryTopic);  
  }

}

// ------------ MQTT REACTION BACK TO HOMEASSISTANT ------------
bool usb0_prevstate = 0;
bool usb1_prevstate = 0;
bool usb2_prevstate = 0;
bool usb3_prevstate = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("-----------------------");
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  Serial.print("Message:");
  char buffer[length];
  for (int i = 0; i < length; i++) {
    buffer[i] = (char)payload[i];
    Serial.print(buffer[i]);
  }
  
  Serial.print("  length ");
  Serial.println(length);
  Serial.println("-----------------------");

  bool usb_ac = false;
  if(buffer[0] == '1') {
    usb_ac = true;
  } else if(buffer[0] == '0') {
    usb_ac = false;
  }

  if(!strcmp(topic, "homeassistant/switch/SmartSwitch2_usb0/set")) {
    Serial.println("USB0!");
    Serial.println("set state: " + String(usb_ac));
    if(usb_ac != usb0_prevstate) {
      Serial.println("Different state!");
      client.publish("homeassistant/switch/SmartSwitch2_usb0/state", String(usb_ac).c_str());
      Serial.println("Setting state to " + String(usb_ac));
      digitalWrite(usb1_en, usb_ac);
      usb0_prevstate = usb_ac;
    }
  } else if(!strcmp(topic, "homeassistant/switch/SmartSwitch2_usb1/set")) {
    Serial.println("USB1!");
    Serial.println("set state: " + String(usb_ac));
    if(usb_ac != usb1_prevstate) {
      Serial.println("Different state!");
      client.publish("homeassistant/switch/SmartSwitch2_usb1/state", String(usb_ac).c_str());
      Serial.println("Setting state to " + String(usb_ac));
      digitalWrite(usb2_en, usb_ac);
      usb1_prevstate = usb_ac;
    }
  } else if(!strcmp(topic, "homeassistant/switch/SmartSwitch2_usb2/set")) {
    Serial.println("USB2!");
    Serial.println("set state: " + String(usb_ac));
    if(usb_ac != usb2_prevstate) {
      Serial.println("Different state!");
      client.publish("homeassistant/switch/SmartSwitch2_usb2/state", String(usb_ac).c_str());
      Serial.println("Setting state to " + String(usb_ac));
      digitalWrite(usb3_en, usb_ac);
      usb2_prevstate = usb_ac;
    }
  } else if(!strcmp(topic, "homeassistant/switch/SmartSwitch2_usb3/set")) {
    Serial.println("USB3!");
    Serial.println("set state: " + String(usb_ac));
    if(usb_ac != usb3_prevstate) {
      Serial.println("Different state!");
      client.publish("homeassistant/switch/SmartSwitch2_usb3/state", String(usb_ac).c_str());
      Serial.println("Setting state to " + String(usb_ac));
      digitalWrite(usb4_en, usb_ac);
      usb3_prevstate = usb_ac;
    }
  }
}

// --------------- DEFINING INTERRUPT FUNCTUIONS ---------------
ICACHE_RAM_ATTR void overload1() {
  digitalWrite(usb1_en, LOW);
  client.publish("homeassistant/switch/SmartSwitch2_usb0/state", "0");
  usb0_prevstate = 0;
  Serial.println("OVERLOAD USB0!!");
}
ICACHE_RAM_ATTR void overload2() {
  digitalWrite(usb2_en, LOW);
  client.publish("homeassistant/switch/SmartSwitch2_usb1/state", "0");
  usb1_prevstate = 0;
  Serial.println("OVERLOAD USB1!!");
}
ICACHE_RAM_ATTR void overload3() {
  digitalWrite(usb3_en, LOW);
  digitalWrite(usb4_en, usb3_prevstate);      // chane to prev state
  client.publish("homeassistant/switch/SmartSwitch2_usb2/state", "0");
  usb2_prevstate = 0;
  Serial.println("OVERLOAD USB2!!");
}
ICACHE_RAM_ATTR void overload4() {
  digitalWrite(usb4_en, LOW);
  digitalWrite(usb3_en, usb2_prevstate);    // chane to prev state
  client.publish("homeassistant/switch/SmartSwitch2_usb3/state", "0");
  usb3_prevstate = 0;
  Serial.println("OVERLOAD USB3!!");
}

Ticker sendAvalable;

void sendAvalableMessage() {
  //String actualtopic = "homeassistant/binary_sensor/" + String(deviceName) + "_usb" + String(i) + "/state";
  //client.publish(actualtopic.c_str(), "online");
  Serial.println("Sendig that Im available");
}

// --------------------------- SETUP ---------------------------
void setup() {
  Serial.begin(115200);
  
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(usb1_en, OUTPUT);
  pinMode(usb2_en, OUTPUT);
  pinMode(usb3_en, OUTPUT);
  pinMode(usb4_en, OUTPUT);

  pinMode(usb1_flg, INPUT);
  pinMode(usb2_flg, INPUT);
  pinMode(usb3_flg, INPUT);
  pinMode(usb4_flg, INPUT);

  attachInterrupt(digitalPinToInterrupt(usb1_flg), overload1, FALLING);
  attachInterrupt(digitalPinToInterrupt(usb2_flg), overload2, FALLING);
  attachInterrupt(digitalPinToInterrupt(usb3_flg), overload3, FALLING);
  attachInterrupt(digitalPinToInterrupt(usb4_flg), overload4, FALLING);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  digitalWrite(usb1_en, LOW);
  digitalWrite(usb2_en, LOW);
  digitalWrite(usb3_en, LOW);
  digitalWrite(usb4_en, LOW);

  reconnect();
  client.publish("homeassistant/switch/SmartSwitch0_usb0/onfig", "TEST");
  sendFirstDiscoveryMsg(deviceName, 0);
  for(int i = 1; i < 4; i++) {
    sendDiscoveryMsg(deviceName, i);
  }
  for(int i = 0; i < 4; i++) {
    String actualtopic = "homeassistant/switch/" + String(deviceName) + "_usb" + String(i) + "/state";
    client.publish(actualtopic.c_str(), "0");
  }
  for(int i = 0; i < 4; i++) {
    String actualtopic = "homeassistant/switch/" + String(deviceName) + "_usb" + String(i) + "/set";
    client.subscribe(actualtopic.c_str());
  }

  sendAvalable.attach(5, sendAvalableMessage);    // send messege that esp is available every 5 seconds
}

// --------------------------- LOOP ----------------------------
void loop() { 
  client.loop();
}
