/**

    NodeMCU
    
    #637 Iothook DHT21 Nem ve Sıcaklık Sensörü 
    3lü bilgi gönderme
    
    27.06.2022

    mfbynl-no masters no slaves
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"


#define SERVER_IP "iothook.com"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#ifndef STASSID
#define STASSID "*****"
#define STAPSK  "*****"
#endif

unsigned long onceki;
float nem;
float sicaklik;
float hissedilen;

DHT dht(DHTPIN, DHTTYPE);


void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();
  dht.begin();
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  

}

void loop() {

  unsigned long simdi = millis();
  if (simdi - onceki > 1000) {
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));

    nem = h;
    sicaklik = t;
    hissedilen = hic;
    
    if ((WiFi.status() == WL_CONNECTED)) {

      WiFiClient client;
      HTTPClient http;

      Serial.print("[HTTP] begin...\n");
      // configure traged server and url
      http.begin(client, "http://" SERVER_IP "/api/update/"); //HTTP
      http.addHeader("Content-Type", "application/json");

      

      

      Serial.print("[HTTP] POST...\n");
      // start connection and send HTTP header and body
      int httpCode = http.POST("{\"api_key\":\"f7d119bff3826428ff2644ab\",\"field_1\":\"" + String(nem) + "\", \"field_2\":\"" + String(sicaklik) + "\", \"field_3\":\"" + String(hissedilen) + "\"}");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          const String& payload = http.getString();
          Serial.println("received payload:\n<<");
          Serial.println(payload);
          Serial.println(">>");
        }
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
    onceki = simdi;

  }

}
