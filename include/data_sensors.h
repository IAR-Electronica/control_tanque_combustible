#define DATA_SENSORS__H 
struct sensor_ultrasonic{
    unsigned int distance ;  
    time_t unix_time_sample ;  
} ;


struct sensor_ultrasonic_compute{
    int median_data ; 
    float media_movil ; // calculo con el promedio 
}  ; 




struct sensor_capacitivo{
    int state_sensor_cap  ; 
    time_t last_unix_time ; 
}; 


enum TYPE_SENSOR {
    CAPACITIVO, 
    ULTRASONIDO,
    ERROR_ULTRASONIDO
}; 



enum ERROR_CONNECT_WIFI_MQTT {
    INIT, 
    CONNECT, 
    ERROR_WIFI,  
    ERROR_MQTT 
}; 


// DEFINICION DE ALIAS PARA ESTRUCTURAS DE DATOS ! 
typedef sensor_ultrasonic_compute sensor_distance_media_values  ;
typedef sensor_ultrasonic sensor_ultrasonic ; 
typedef TYPE_SENSOR type_sensor ; 
typedef ERROR_CONNECT_WIFI_MQTT error_connect ; 
typedef sensor_capacitivo s_cap ; 
