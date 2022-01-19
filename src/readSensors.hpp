#include <Arduino.h>
#include "data_sensors.h"
#define PIN_TRIGGER 12 // 
#define PIN_ECHO   14 //
#define PIN_SENSOR_CAP D1 

sensor_ultrasonic sensor_distance[5];  

void initPorts(){
    pinMode(PIN_TRIGGER,OUTPUT) ;
    pinMode(PIN_SENSOR_CAP,OUTPUT) ;
    pinMode(PIN_ECHO,INPUT) ; 
}


// lectura cada 5 segundos, lee y calcula media movil y mediana 
// con un buffer de tamaño 5 
// además, obtiene el unix_time desde un servidor NTC (falta terminar)
void readUltrasonicSensor(){ 
    //getNTC unix time ! 
    int index_fifo_buffer = 0 ; 
    unsigned int distance_median[5] ; 
    //get unix_time 
    digitalWrite(PIN_TRIGGER,HIGH) ; 
    delayMicroseconds(10) ; 
    digitalWrite(PIN_TRIGGER,LOW) ;  
    //pulseIn return 0 if error response 
    unsigned long int miliseconds_response = pulseIn(PIN_ECHO,HIGH); 
    float distance_cm = 0.034 * (miliseconds_response/2) ;
    // fifo buffer rutina  
    for (index_fifo_buffer = 4;index_fifo_buffer>0;index_fifo_buffer--)
    {
        sensor_distance[index_fifo_buffer].distance = sensor_distance[index_fifo_buffer-1].distance; 
    }
    sensor_distance[0].distance = (int) distance_cm ; 
    //media movil
    distance_median[0] = sensor_distance[0].distance ; 
    distance_median[1] = sensor_distance[1].distance ; 
    distance_median[2] = sensor_distance[2].distance ; 
    distance_median[3] = sensor_distance[3].distance ;    
    distance_median[4] = sensor_distance[4].distance ; 
    int aux ; 
    //ordenamiento metodo de la burbuja  
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(distance_median[j] > distance_median[j + 1]){
                aux = distance_median[j];
                distance_median[j] = distance_median[j + 1];
                distance_median[j + 1] = aux;
            }
        }
    }
    //mediana: distance_median[2] ;  
    Serial.println("--------------------------------------------------------") ; 
    Serial.print("ordenados: : ") ; Serial.print( distance_median[0]) ; 
    Serial.print( distance_median[1]) ; Serial.print(" ")  ;
    Serial.print( distance_median[2]) ; Serial.print(" ")  ;
    Serial.print( distance_median[3]) ; Serial.print(" ")  ;
    Serial.println( distance_median[4]) ; 
    
    //calcular media movil y mediana! 



    Serial.print("distance: ") ; Serial.println( sensor_distance[0].distance) ; 
    Serial.print("distance: ") ; Serial.println( sensor_distance[1].distance) ; 
    Serial.print("distance: ") ; Serial.println( sensor_distance[2].distance) ; 
    Serial.print("distance: ") ; Serial.println( sensor_distance[3].distance) ; 
    Serial.print("distance: ") ; Serial.println( sensor_distance[4].distance) ; 
    Serial.println("--------------------------------------------------------") ; 
    
}


void sensorCapacitivo(){
    int value_sensor_cap ; 
    if (digitalRead(PIN_SENSOR_CAP) == LOW) {
        value_sensor_cap = 0 ; 
    }else if (digitalRead(PIN_SENSOR_CAP) == HIGH){ 
        value_sensor_cap = 0xFF ; 
    }  
    //PUBLISH SENSOR_CAP 


}