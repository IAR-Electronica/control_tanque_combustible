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
#include "readSensors.hpp"

//#include <webUpdater.hpp>
#define PUBLISH_TIME_SENSOR 5 //TIME IN SECONDS 

#define SSID ""
#define PASSWORD_WIFI ""



int wifi_con = -1 ; 

// scheduler variables  
int connect_wifi_verification = 0 ; 
int time_publish_sensor_distance  = 0 ; 
int time_publish_sensor_capacitivo = 0 ; 
int time_verify_connect = 0 ;
unsigned int  time_connect_iar = 0 ; 
//esta funcion se conecta al wifi del iar, 1 true, -1 false  
int connectIAR() ; 
int readUltasonic() ; 
 //configura base de tiempo en un segundo 
void initTimer() ;
//rutina de interrupción cada un segundo 
void IRAM_ATTR isr_time() ;


void setup() {
  Serial.begin(115200);
  initTimer() ;   
  Serial.println() ;  
  wifi_con =-1;  
  if (wifi_con == 1){
    getHourNTC() ; 
    initMQTT() ; 
  }else if (wifi_con == -1){
    Serial.print("WIFI_CON -1") ;
  }else ESP.restart() ;  // seguridad !  
  initPorts() ; 
  time_publish_sensor_distance = 0 ; 
  connect_wifi_verification = 0 ; 
//  time_connect_iar = 0 ; 
}


void loop() {
  // 10 minutos 
  if (time_connect_iar==600000ul){
    Serial.print("10 minutos") ; 
    time_connect_iar = 0 ; 
  
  }
  
  if (web_update_on == true){
    updateSoftware() ; 
  }
  //publish data every five seconds using 
  if (time_publish_sensor_distance == 5000){
    readUltrasonicSensor() ; 
    time_publish_sensor_distance = 0 ; 
  }
  client.loop() ; 

}




int connectIAR() { 

  /*
  * return 1 --> conectado 
  * return -1 --> no se conecto 
  */
  Serial.println("connect_wifi") ; 
  int error_connect = -1 ; 
  const char *ssid = SSID ; 
  const char *psk = PASSWORD_WIFI ; 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, psk);
  
  connect_wifi_verification  = 0 ;  
   
  
  while (WiFi.status() != WL_CONNECTED && connect_wifi_verification <=10000)   
  {
    while(connect_wifi_verification%200 != 0){
      yield() ; //hace la magia de terminar la ejecución del stack tcp/ip 
    }  
    yield() ;  //hace la magia de terminar la ejecución del stack tcp/ip  
  } 
  connect_wifi_verification = 0 ; 
  if (WiFi.status() != WL_CONNECTED){
    error_connect = -1 ; 
  }else error_connect = 1 ; 
  return error_connect; 
}


void initTimer(){
  timer1_enable(TIM_DIV16,TIM_EDGE,TIM_LOOP) ; 
  //cantidad de ticks .. 23 bits 
  // TIMER TICK 1 ms (1000ms = 1 segundo)
  timer1_write(5000) ; 
  timer1_attachInterrupt(isr_time) ; //cantidad de ticks 

}


void IRAM_ATTR isr_time(){
  time_publish_sensor_distance++ ; 
  connect_wifi_verification++ ; 
  time_connect_iar++ ; 
}

