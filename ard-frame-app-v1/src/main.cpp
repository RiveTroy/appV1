#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

#define wifi_ssid "Merlin"
#define wifi_password "a19109510"

#define mqtt_server "192.168.2.137"
#define mqtt_user "esp8266"
#define mqtt_password "ebtro34"
#define mqtt_port 1883

#define humidity_topic "sensor/humidity"
#define temperature_celsius_topic "sensor/temperature_celsius"
#define temperature_fahrenheit_topic "sensor/temperature_fahrenheit"

WiFiClient espClient;
PubSubClient client(espClient);


String macToStr(const uint8_t* mac) {
    String result;
    for (int i=0; i<6; i++)
    {
        result += String(mac[i], 16);
        if (i<5)
            result += ':';
    }
    return result;
}

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
    Serial.println("WiFi connected.");
    Serial.println("IP Address: ");
}

void reconnect() {
    while(!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        
        String clientName;
        clientName += "esp8266-";
        uint8_t mac[6];
        WiFi.macAddress(mac);
        clientName += macToStr(mac);
        clientName += "-";
        clientName += String(micros() & 0xff, 16);
        Serial.print("Connecting to ");
        Serial.print(mqtt_server);
        Serial.print("as");
        Serial.println(clientName);

        if (client.connect((char*) clientName.c_str())) {
            Serial.println("connected");
        }
        else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      delay(10000);
      
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float f = dht.readTemperature(true);
      
      if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
      }
      
      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.print(" *C ");
      Serial.print(hif);
      Serial.println(" *F");


      Serial.print("Temperature in Celsius: ");
      Serial.println(String(t).c_str());
      client.publish(temperature_celsius_topic, String(t).c_str(), true);

      Serial.print("Temperature in Fahrenheit: ");
      Serial.println(String(f).c_str());
      client.publish(temperature_fahrenheit_topic, String(f).c_str(), true);


      Serial.print("Luminosity: ");
      Serial.println(String(h).c_str());
      client.publish(humidity_topic, String(h).c_str(), true);
}
