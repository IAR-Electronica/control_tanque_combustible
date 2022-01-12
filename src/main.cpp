/*
 * Autor: Gaston Valdez   
 * Proyecto: medición remota del tanque de combustible del generador del iar 
 * Director: Elias S fliger 
 * Version 1.0 
 * Descripción: lee el sensor ultrasónico cada 10 segundos y envia la información al broker MQTT
 *              al topico esta subscrito node_red y puede almacenar y gráficar estos datos 
 *              Adicionalmente, se plantea su upgrade mediante OTA 
 *              
 * 
*/
#include <ESP8266WiFi.h> 
#include <Arduino.h>
#include <mqtt.hpp>
//#include <webUpdater.hpp>
#define PUBLISH_TIME_SENSOR 10000ul

#define SSID "local"
#define PASSWORD_WIFI "iarpublicas"
void connectWifiIAR() ; 
int readUltasonic() ; 




void setup() {
  Serial.begin(115200) ; 
  connectWifiIAR() ; 
  initMQTT() ; 
//  initWebUpdate() ; 

 
}

unsigned long int time_publish = 0 ; 

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (web_update_on == true){
    updateSoftware() ; 
  }

  if (millis()-time_publish>=PUBLISH_TIME_SENSOR){
    time_publish = millis() ; 
    publishmqtt() ; 
  }
  client.loop() ; 

}




void connectWifiIAR() { 

  /*
  * return 1 --> conectado 
  * return -1 --> no se conecto 
  */
  const char *ssid = SSID ; 
  const char *psk = PASSWORD_WIFI ; 
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, psk);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
  }
  Serial.print("IP: ") ; Serial.println(WiFi.localIP()) ; 



}

