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
unsigned int timer_1  = 0 ; // ultrasonic sensor time 
unsigned int timer_2  = 0 ; // mqtt publish time sensor ultrasonic   
unsigned int timer_3  = 0 ; // connect_wifi  
unsigned int timer_4  = 0 ; // mqtt_ publish cap_sensor and off web server  

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
  // init timer variables 
  timer_1 = 0 ; 
  timer_2 = 0 ; 
  timer_3 = 0 ; 
  timer_4 = 0 ; 
}


void loop() {
  //server web on 
  if (web_update_on == true){
    updateSoftware() ; 
  }
  
  if (timer_4 == 600000ul){
    timer_4 = 0 ; 
    Serial.println("10 minutos") ; 
  }

  if (timer_2 == 60000ul ){
    Serial.println("one minute") ; 
    timer_2 = 0 ; 
  }
  
  if (timer_3 == 40000){
    Serial.println("cuarenta segundos") ; 
    timer_3 = 0 ; 
  }
  
  // read the ultrasonic sensor 
  if (timer_1 == 5000){
    readUltrasonicSensor() ; 
    timer_1 = 0 ; 
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
  while (WiFi.status() != WL_CONNECTED && timer_3 <=10000)   
  {
    while(timer_3%200 != 0){
      yield() ; //hace la magia de terminar la ejecución del stack tcp/ip 
    }  
    yield() ;  //hace la magia de terminar la ejecución del stack tcp/ip  
  } 

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
  timer_1 ++ ; 
  timer_2 ++ ; 
  timer_3 ++ ; 
  timer_4 ++ ; 
  


}

