#include <PubSubClient.h>
#include <WiFiClient.h> 
#include <webUpdater.hpp>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#ifndef DATA_SENSORS__H
  #include "data_sensors.h"
#endif 
#define PORT_NTC 2390 
#define NTP_PACKET_SIZE 48
#define SERVER_NTC "gps.iar.unlp.edu.ar" //servidor NTC 
#define BROKER_MQTT "163.10.43.85"
#define TOPIC_1_MQTT "/iar/salaMaquinas/sensorUltrasonido"
#define TOPIC_CAP_MAX "/iar/salaMaquinas/sensorCapacitivoMax"
#define TOPIC_CAP_MIN "/iar/salaMaquinas/sensorCapacitivoMin"
#define TOPIC_2_MQTT "/iar/salaMaquinas/upgrade"
#define ID_SENSOR_1 "ULTR"
#define ID_MSG_SENSOR_2 "CAPA" 
#define MAC_ADDRESS "A4:CF:12:EF:7E:0B"
#define PORT_MQTT 1883  // PORT_INSECURE
extern s_cap sensor_cap ; 
extern sensor_ultrasonic sensor_distance[5];
//sensor de distancia 
unsigned long int id_dato_sensor_distancia = 0 ;
 //sensor capacitivo
unsigned long int id_dato_sensor_capacitivo = 0 ; 
// buffer para perdida de conexión 
sensor_ultrasonic distance_buffer[15]  ; 
//capacitor_buffer[3] ; 
s_cap sensor_cap_buffer[2] ; 
int index_capacitor_buffer_not_wifi = 0 ; 
int index_distance_buffer_not_wifi = 0 ; 
char clean_buffer = 'x' ; 


// clases para el manejo de MQTT y NTC  
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP udp; 
// obtiene la hora usando NTC 
time_t getHourNTC() ; 
void sendPacketNTP(IPAddress& address) ; 
void uploadONMQTT(char *topic, byte *payload, unsigned int length);
int reconnect() ; 



//DEFINICIÓN DE CALLBACKS Y SETTEAR SERVIDOR MQTT   
void initMQTT()
{ 
  randomSeed(micros()) ;
  client.setServer(BROKER_MQTT, 1883); 
  client.setCallback(uploadONMQTT) ; 
  udp.begin(PORT_NTC);
}

void publishmqtt(type_sensor sensor) { 
    int mqtt_status ; 
    int wifi_status ;

    //verficación del estado de la red y del servidor mqtt   
    //mqtt_status = -1 :  not OK , 1:OK 
    //wifi_status = -1 :  not OK , 1:OK 
    Serial.println("publish_mqtt") ; 

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("mqtt_status y wifi_status -1 ") ; 
      mqtt_status = -1 ;
      wifi_status = -1 ; 
    }else 
    {
      wifi_status = 1 ; 
      if (!client.connected()) 
      {
        mqtt_status = reconnect() ;   
      }else mqtt_status = 1 ;  
      Serial.print("wifi_status= 1 y mqtt_status = ") ; 
      Serial.println(mqtt_status) ; 
    }
   
    if (mqtt_status == -1 || wifi_status ==-1){
      if (sensor == ULTRASONIDO){ 
        distance_buffer[index_distance_buffer_not_wifi] = sensor_distance[0] ; 
        index_distance_buffer_not_wifi++ ;
        index_distance_buffer_not_wifi =   index_distance_buffer_not_wifi%15;     
        
      }else if (sensor == CAPACITIVO){
        sensor_cap_buffer[index_capacitor_buffer_not_wifi] = sensor_cap ; 
        index_capacitor_buffer_not_wifi++  ;
        index_capacitor_buffer_not_wifi = index_capacitor_buffer_not_wifi %2  ; 
       
      } 
      clean_buffer = 'c' ;
      return ; 
    }
    char payload[200] ; 
    char data_date[20] ;
    StaticJsonDocument<192> doc;
     
  
    if (clean_buffer == 'c')
    {
      //CLEAN DATA BECAUSE NOT WIFI FUNCTION 
      Serial.println("clean_buffer ") ; 
      for (int i = 0;i<15 ; i++){
        if (distance_buffer[i].distance == 0){
          continue ; 
        }
        doc["id"] = id_dato_sensor_distancia;
        doc["fecha"] =distance_buffer[i].unix_time_sample;
        doc["mac"] = MAC_ADDRESS;
        doc["idSensor"] = ID_SENSOR_1;
        doc["dato"] = distance_buffer[i].distance  ;
        serializeJson(doc,payload) ; 
        distance_buffer[i].distance = 0 ; 
        distance_buffer[i].unix_time_sample = 0 ; 
        id_dato_sensor_distancia++ ;     
        client.publish(TOPIC_1_MQTT,payload) ;
        
      }      
     
      doc["id"] = id_dato_sensor_distancia;
      doc["fecha"] = sensor_cap_buffer[0].last_unix_time;
      doc["mac"] = MAC_ADDRESS;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer[0].state_sensor_cap;
      
      serializeJson(doc,payload) ; 
      client.publish(TOPIC_1_MQTT,payload) ;
      
      doc["id"] = id_dato_sensor_distancia;
      doc["fecha"] = sensor_cap_buffer[1].last_unix_time;
      doc["mac"] = MAC_ADDRESS;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer[1].state_sensor_cap;
      serializeJson(doc,payload) ; 
      client.publish(TOPIC_1_MQTT,payload) ;
      id_dato_sensor_capacitivo++ ; 
      index_distance_buffer_not_wifi = 0 ; 
      Serial.println ("end clean buffer") ; 
      return ; 
    }

    if (sensor == ULTRASONIDO){
          
      doc["id"] = id_dato_sensor_distancia;
      doc["fecha"] = sensor_distance[0].unix_time_sample;
      doc["mac"] = MAC_ADDRESS;
      doc["idSensor"] = ID_SENSOR_1;
      doc["dato"] =sensor_distance[0].distance ;
      id_dato_sensor_distancia++ ; 
    }else if(sensor == CAPACITIVO){
      doc["id"] = id_dato_sensor_capacitivo;
      doc["fecha"] = sensor_cap.last_unix_time;
      doc["mac"] = MAC_ADDRESS;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =sensor_cap.state_sensor_cap ; 
      id_dato_sensor_capacitivo++ ; 
    }
    serializeJson(doc,payload ) ; 
    Serial.print("json_payload: ") ; Serial.println(payload) ; 
    client.publish(TOPIC_1_MQTT,payload) ; 
   
}





// return 1 -> connect 
// return -1 -> not connect 
int reconnect() {
  int status ; 
  if (!client.connected()) 
  {
    // Create a random client ID
    String clientId = "ESP8266SALAMAQUINA";
    clientId += String(random(0xffff), HEX) ; 
    randomSeed(millis()) ;
    clientId += String(random(0xffff), HEX) ; 
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected") ; 
      client.subscribe(TOPIC_2_MQTT) ; 
      status = 1  ; 
    } 
  }else status = -1 ; 
  return status ; 
}


void uploadONMQTT(char *topic, byte *payload, unsigned int length)
{
  // compara topics -- seguridad ! 
  if (strcmp(topic,TOPIC_2_MQTT) != 0){
    return ; 
  }
  // transform payload to string 
  String payload_str = "" ; 
  String user ; 
  String pass ; 
  char c ; 
  for (unsigned int i = 0;i<length ; i++){
    c = (char )payload[i] ;
    if (isAlphaNumeric(c) || c == ':' || c ==','){
      payload_str = payload_str + String(c) ; 
    } 
  }
  
  
  
  // obtain user and password for website for upgrade firmware 
  int index_user = payload_str.indexOf("user") ; 
  int index_user_end = payload_str.indexOf(":") ;
  int pass_index = payload_str.indexOf("pass") ; 
  int pass_index_end = payload_str.indexOf(":",pass_index) ;
  if (index_user == -1 || index_user_end == -1 || pass_index == -1 || pass_index_end == -1){
    Serial.println("error en payload") ; 
    return ; 
  }
  
  user = payload_str.substring(index_user_end+1,pass_index) ; 
  user.trim() ; 
  pass = payload_str.substring(pass_index_end+1) ; 
  pass.trim() ; 
 
  initWebUpdate(user.c_str(), pass.c_str() ) ; 


}


/*
 * return 0  -> error  
 * else return unixtime 
*/
time_t getHourNTC(){
  time_t response_time ; 
  IPAddress server_ntc_ip ; 
  byte obtain_date[NTP_PACKET_SIZE] ; 
  WiFi.hostByName(SERVER_NTC, server_ntc_ip);
  sendPacketNTP(server_ntc_ip) ; 
  int response = udp.parsePacket();
  
  if (!response){
    response_time = 0 ;  
  }else{ 
    //obtain ntc parameters 
    udp.read(obtain_date, NTP_PACKET_SIZE);
    unsigned long highWord = word(obtain_date[40],obtain_date[41]);
    unsigned long lowWord  = word(obtain_date[42], obtain_date[43]);
    unsigned long long int secsSince1900 = highWord << 16 | lowWord;
    // unix time calculate  
    time_t raw_secods = (time_t ) (secsSince1900 - 2208988800UL);  
    response_time = raw_secods ; 
  }
  return response_time ; 

}


void sendPacketNTP(IPAddress& server_ntc){
  byte packetBuffer[ NTP_PACKET_SIZE] ;  
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(server_ntc, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();




}