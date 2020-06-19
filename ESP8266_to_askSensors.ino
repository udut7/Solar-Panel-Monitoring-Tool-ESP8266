#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

const char* wifi_ssid = "ssid";             
const char* wifi_password = "password";        
const char* apiKeyIn = "apiKeyIn";      
const unsigned int writeInterval = 25000; 

// ASKSENSORS config
const char* https_host = "api.asksensors.com";         
const int https_port = 443;                        
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     

// INA219 config
Adafruit_INA219 ina219;
float myvoltage = 0;
float mycurrent = 0;
unsigned long previousMillis = 0;
const long interval = 10000;  

// create ASKSENSORS client
WiFiClientSecure client;
void setup() {
  Serial.begin(115200);
  Wire.begin(2, 0);
  ina219.begin();
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();
}

void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float busvoltage = ina219.getBusVoltage_V();
    if (isnan(busvoltage)) {
      Serial.println("Failed to read from INA219 sensor!");
    }
    else {
      myvoltage = busvoltage;
      Serial.println(myvoltage);
    }
    float current = ina219.getCurrent_mA();
    if (isnan(current)) {
      Serial.println("Failed to read from INA219 sensor!");
    }
    else {
      mycurrent = current;
      Serial.println(mycurrent);
    }
  }
  
// Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }
// Create a URL for the request
  String url = "/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += myvoltage;
  url += "&module2=";
  url += mycurrent;
  
  Serial.print("********** requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + https_host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("> Request sent to ASKSENSORS");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
        String line = client.readStringUntil('\n');
        Serial.println("********** ASKSENSORS replay:");
        Serial.println(line);
        Serial.println("********** closing connection");
      
        break;
    }
  }
  delay(writeInterval );
}
