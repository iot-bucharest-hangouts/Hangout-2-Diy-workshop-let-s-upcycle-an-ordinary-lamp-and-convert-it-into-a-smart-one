#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>         //https://github.com/knolleary/pubsubclient
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

//const char* ssid = "........";
//const char* password = "........";
const char* mqtt_server = "broker.hivemq.com"; //https://www.hivemq.com/public-mqtt-broker/
const char* UUID = "6bcdb172-b427-11e8-96f8-529269fb1459";
const char* sub_topic = "/control";

WiFiClient espClient;
PubSubClient client(espClient);

const char* wifi_device_name = "Lampix";
const char* wifi_device_pass = "iot_hangout_2";
long lastMsg = 0;
char msg[50];
int value = 0;
int gpio_pin = 2;

WiFiManager wifiManager;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";
  for (int i = 0; i < length; i++) {
    message = message + String((char)payload[i]);
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (message == "ON") {
    digitalWrite(gpio_pin, LOW);
  }
  else if (message == "OFF") {
    digitalWrite(gpio_pin, HIGH);
  }
  else if (message == "reset_settings") {
    digitalWrite(gpio_pin, HIGH);

    for (int i = 0; i < 5; i++){
      digitalWrite(gpio_pin, LOW);
      delay(250);
      digitalWrite(gpio_pin, HIGH);
      delay(250);
      }

    wifiManager.resetSettings();
    ESP.restart();
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");

      // ... and resubscribe
      client.subscribe(UUID);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(gpio_pin, OUTPUT);
  digitalWrite(gpio_pin, HIGH);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(wifi_device_name, wifi_device_pass)) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  digitalWrite(gpio_pin, LOW);
  delay(250);
  digitalWrite(gpio_pin, HIGH);
  delay(250);
  digitalWrite(gpio_pin, LOW);
  delay(250);
  digitalWrite(gpio_pin, HIGH);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}

void loop() {
  // put your main code here, to run repeatedly:

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //  long now = millis();
  //  if (now - lastMsg > 2000) {
  //    lastMsg = now;
  //    ++value;
  //    snprintf (msg, 75, "hello world #%ld", value);
  //    Serial.print("Publish message: ");
  //    Serial.println(msg);
  //    client.publish("outTopic", msg);
  //  }
}

