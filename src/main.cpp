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
#ifndef DATA_SENSORS__H
  #include "data_sensors.h"
#endif 
#include "readSensors.hpp"
//#include <webUpdater.hpp>
#define PUBLISH_TIME_SENSOR 5000 //TIME IN SECONDS 
#define TIMER_CONTROL_WIFI_CONNECT  40000ul           // check the wifi connection every 40 seconds 
#define PUBLISH_TIME_SENSOR_ULTRASONIC 6000ul       // publish distance sensor ultrasonic  
#define PUBLISH_SENSOR_CAPACITIVE_DISTANCE 6500ul //600000ul //
#define SSID "local"
#define PASSWORD_WIFI "iarpublicas"



// scheduler variables  
unsigned int timer_1  = 0 ; // ultrasonic sensor time 
unsigned int timer_2  = 0 ; // mqtt publish time sensor ultrasonic   
unsigned int timer_3  = 0 ; // connect_wifi  
unsigned int timer_4  = 0 ; // mqtt_ publish cap_sensor and off web server  

error_connect isConnecctWifi_mqtt() ; 

//esta funcion se conecta al wifi del iar, 1 true, -1 false  
int connectIAR() ; 
 //configura base de tiempo en un milisegundo 
void initTimer() ;
//rutina de interrupción cada un segundo 
void IRAM_ATTR isr_time() ;


void setup() {
  //system_set_os_print(0) ; 
  Serial.begin(115200);
  Serial.println("init") ; 

  initTimer() ;   
  int wifi_con = connectIAR(); 
  initMQTT() ; 

  if (wifi_con == 1){
    getHourNTC() ; 
  }else if (wifi_con == -1){
    Serial.print("WIFI_CON -1") ;
  }else ESP.restart() ;  // seguridad   
  initPorts() ; 
  // init timer variables 
  timer_1 = 0 ; 
  timer_2 = 0 ; 
  timer_3 = 0 ; 
  timer_4 = 0 ; 
  Serial.println("end_init") ; 
}


void loop() {
  //server web on 
  if (web_update_on == true){
    updateSoftware() ; 
  }
  
  if (timer_4 >= PUBLISH_SENSOR_CAPACITIVE_DISTANCE){
    sensorCapacitivo() ; 
    publishmqtt(CAPACITIVO_MIN) ;
    publishmqtt(CAPACITIVO_MAX) ;     
    server_web_off() ; 
    timer_4 = 0 ; 
  }

  // timer de un minuto 
  //60000ul 
  // publish distance sensor ultrasonic  
  if (timer_2 >= PUBLISH_TIME_SENSOR_ULTRASONIC ){
    publishmqtt(ULTRASONIDO) ; 
    timer_2 = 0 ; 
  }
  
  //verificación de wifi y mqtt cada 40 segundos 
  if (timer_3 >= TIMER_CONTROL_WIFI_CONNECT){
    isConnecctWifi_mqtt() ; //response using in the next designers
    timer_3 = 0 ; 
  }
  
  // read the ultrasonic sensor 
  if (timer_1 >= PUBLISH_TIME_SENSOR){
    readUltrasonicSensor() ; 
    timer_1 = 0 ; 
  }


  client.loop() ; 

}



//función para conectarse a la red del iar
//maximo tiempo de ejecucion 10 segundos ! 
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
  Serial.print("IP: ") ; Serial.println(WiFi.localIP()) ; 
  timer_3 = 0 ;
  if (WiFi.status() != WL_CONNECTED){
    error_connect = -1 ; 
  }else error_connect = 1 ; 
  return error_connect; 
}


/*
 * return enum error_connect 
 * CONNECT --> AMBOS CONECTADOS 
 * ERROR_WIFI --> MQTT DESACTIVADO Y DESCONECTADO DEL WIFI 
 * ERROR_MQTT --> WIFI CONECTADO Y MQTT NO CONECTADO 
 * En caso de estar desconectado, reintentará conectarse 
 * tiempo maximo de ejecución: 13 segundos ! 
*/


error_connect isConnecctWifi_mqtt()
{
  Serial.println("isconnect_wifi_mqtt ") ; 
  error_connect error_status_network = INIT; 
  int connect_iar ; 
  if (WiFi.status() != WL_CONNECTED )
  {
    connect_iar = connectIAR() ;  //intento de reconexión 
    error_status_network = ERROR_WIFI ; 
    return error_status_network ; 
  }else {
    connect_iar = 1 ; 
  }

  if (connect_iar == 1 && !client.connected()) 
  {
    error_status_network = reconnect() ==1 ? CONNECT: ERROR_MQTT  ;
  }

  return error_status_network ; 
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

