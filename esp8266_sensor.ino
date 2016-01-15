#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

void sendTemperature(float temp);
const char* ssid     = "cez";
const char* password = "wifipass";
OneWire  ds(4);
DallasTemperature DS18B20(&ds);
float oldTemp;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  
  Serial.begin(115200);
  Serial.print("Discovering oneWire sensors...\n");


  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println("");

  }
  Serial.println("DHT Weather Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// the loop function runs over and over again forever
void loop() {
  Serial.print("Starting loop");
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));
  
  if (temp != oldTemp)
  {
    sendTemperature(temp);
    oldTemp = temp;
  }
  
  delay(30000);
}

void sendTemperature(float temp)
{
 String url;
 WiFiClient client;
 if (!client.connect("api.mymetrics.org", 80)) {
    Serial.println("connection failed");
    return;
  }
  
  url = "/metrics/submit?account=cez3&metric=temp2&value=";
  url += String(temp) ;
  //url += "}";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.mymetrics.org\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);
  Serial.println(url);
  if (client.find("200 OK")) {              // look for OK from server
    Serial.println("sent OK");
  } 
  Serial.print(" Response: ");
  while(client.available()){                    // read the remaining text from serial otherwise connection cannot be closed
    String line = client.readStringUntil('\r');
    
    Serial.print(line);
  }
  
  client.stop();
  
}
