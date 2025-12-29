#include <WiFi.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_SENHA ""

void setup(){
    Serial.begin(115200);
    
    WiFi.begin(WIFI_SSID,WIFI_SENHA);
    Serial.println("Conectando a rede ...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(5000);
        Serial.print(".");
    }
    Serial.println("Conectado!");
    
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
}