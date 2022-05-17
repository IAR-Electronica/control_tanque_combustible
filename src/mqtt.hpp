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
#define TOPIC_1_MQTT ""
#define TOPIC_CAP_MAX ""
#define TOPIC_CAP_MIN ""
#define TOPIC_2_MQTT ""
#define ID_SENSOR_1 "ULTR"
#define ID_MSG_SENSOR_1 "CAPA_MAX" //FIXME: CAMBIAR POR NOMBRE MAS DESCRIPTIVO 
#define ID_MSG_SENSOR_2 "CAPA_MIN" //FIXME: CAMBIAR POR NOMBRE MAS DESCRIPTIVO 

#define MAC_ADDRESS "A4:CF:12:EF:7E:0B"
#define PORT_MQTT 1883  // PORT_INSECURE
extern s_cap sensor_cap_min ;
extern s_cap sensor_cap_max ;
extern sensor_ultrasonic sensor_distance[5];

//sensor de distancia 
unsigned long int id_dato_sensor_distancia = 0 ;
 //sensor capacitivo
unsigned long int id_dato_sensor_capacitivo_min = 0 ;
unsigned long int id_dato_sensor_capacitivo_max = 0 ;

// mediciones offline 
int index_capacitor_buffer_not_wifi_max = 0 ;
int index_capacitor_buffer_not_wifi_min = 0 ;
int index_distance_buffer_not_wifi = 0 ; 
char clean_buffer = 'x' ; 
  // buffer para perdida de conexión 
sensor_ultrasonic distance_buffer[15]  ; 
  //capacitor_buffer[3] ; 
s_cap sensor_cap_buffer_min[2] ; 
s_cap sensor_cap_buffer_max[2] ; 




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
    StaticJsonDocument<192> doc;
    char payload[200] ; 
    char topic_mqtt[40] ; 
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
        
      }else if (sensor == CAPACITIVO_MAX){
        sensor_cap_buffer_max[index_capacitor_buffer_not_wifi_max] = sensor_cap_max ; 
        index_capacitor_buffer_not_wifi_max++  ;
        index_capacitor_buffer_not_wifi_max = index_capacitor_buffer_not_wifi_max%2  ; 
       
      }else if (sensor == CAPACITIVO_MIN){
        sensor_cap_buffer_min[index_capacitor_buffer_not_wifi_min] = sensor_cap_min ;  
        index_capacitor_buffer_not_wifi_min++  ;
        index_capacitor_buffer_not_wifi_min = index_capacitor_buffer_not_wifi_min %2  ; 
      } 
      clean_buffer = 'c' ;
      return ; 
    }

    //cleaning buffer   
    if (clean_buffer == 'c')
    {
      //CLEAN DATA BECAUSE NOT WIFI FUNCTION 
      Serial.println("clean_buffer ") ; 
      for (int i = 0;i<15 ; i++)
      {
        if (distance_buffer[i].distance == 0){
          continue ; 
        }
        doc["id"] = id_dato_sensor_distancia;
        doc["fecha"] =distance_buffer[i].unix_time_sample;
        doc["idSensor"] = ID_SENSOR_1;
        doc["dato"] = distance_buffer[i].distance  ;
        serializeJson(doc,payload) ; 
        distance_buffer[i].distance = 0 ; 
        distance_buffer[i].unix_time_sample = 0 ; 
        id_dato_sensor_distancia++ ;     
        sprintf(topic_mqtt,"%s",TOPIC_1_MQTT)  ; 
        client.publish(topic_mqtt,payload) ;
        delay(10) ; //10 ms de retraso ! 
      }      

      // sensor capacitivo nivel maximo 
      sprintf(topic_mqtt,"%s",TOPIC_CAP_MAX)  ; 
      doc["id"] = id_dato_sensor_capacitivo_max ;
      doc["fecha"] = sensor_cap_buffer_max[0].last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer_max[0].state_sensor_cap;

      serializeJson(doc,payload) ; 
      delay(10) ; 
      client.publish(topic_mqtt,payload) ;
      id_dato_sensor_capacitivo_max++ ;
      doc["id"] = id_dato_sensor_capacitivo_max;
      doc["fecha"] = sensor_cap_buffer_max[1].last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer_max[1].state_sensor_cap;
      serializeJson(doc,payload) ; 
      client.publish(topic_mqtt,payload) ;
      id_dato_sensor_capacitivo_max++ ;
      // fin capacitivo nivel maximi 
      // sensor capacitivo nivel minimo 
      sprintf(topic_mqtt,"%s",TOPIC_CAP_MAX)  ; 
      doc["id"] = id_dato_sensor_capacitivo_max ;
      doc["fecha"] = sensor_cap_buffer_max[0].last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer_max[0].state_sensor_cap;

      serializeJson(doc,payload) ; 
      delay(10) ; 
      client.publish(topic_mqtt,payload) ;
      id_dato_sensor_capacitivo_max++ ;
      doc["id"] = id_dato_sensor_capacitivo_max;
      doc["fecha"] = sensor_cap_buffer_max[1].last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =   sensor_cap_buffer_max[1].state_sensor_cap;
      serializeJson(doc,payload) ; 
      client.publish(topic_mqtt,payload) ;
      id_dato_sensor_capacitivo_max++ ;
      //fin sensor capacitivo nivel minimo 
      index_distance_buffer_not_wifi = 0 ; 
      Serial.println ("end clean buffer") ; 
      return ; 
    }

    if (sensor == ULTRASONIDO){
      sprintf(topic_mqtt,"%s",TOPIC_1_MQTT)  ;   
      doc["id"] = id_dato_sensor_distancia;
      doc["fecha"] = sensor_distance[0].unix_time_sample;
      doc["idSensor"] = ID_SENSOR_1;
      doc["dato"] =sensor_distance[0].distance ;
      id_dato_sensor_distancia++ ; 
    }else if(sensor == CAPACITIVO_MAX){
      sprintf(topic_mqtt,"%s",TOPIC_CAP_MAX)  ;   
      doc["id"] = id_dato_sensor_capacitivo_max;
      doc["fecha"] = sensor_cap_max.last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_1;
      doc["dato"] =sensor_cap_max.state_sensor_cap ; 
      id_dato_sensor_capacitivo_max++ ; 
    }else if (sensor== CAPACITIVO_MIN){
      sprintf(topic_mqtt,"%s",TOPIC_CAP_MIN)  ; 
      doc["id"] = id_dato_sensor_capacitivo_min;
      doc["fecha"] = sensor_cap_min.last_unix_time;
      doc["idSensor"] = ID_MSG_SENSOR_2;
      doc["dato"] =sensor_cap_min.state_sensor_cap ; 
      id_dato_sensor_capacitivo_min++ ; 
    }
    serializeJson(doc,payload ) ;
    Serial.print("topic: ") ; Serial.println(topic_mqtt) ;  
    Serial.print("json_payload: ") ; Serial.println(payload) ; 
    client.publish(topic_mqtt,payload) ; 
   
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
  delay(10) ; 
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