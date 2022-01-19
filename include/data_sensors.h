struct sensor_ultrasonic{
    unsigned int id_dato  ; //changes only publish data
    unsigned int distance ;  
    time_t last_unix_time ;   
} ;


struct sensor_ultrasonic_compute{
    int median_data ; 
    float media_movil ; // calculo con el promedio 
}  ; 




struct sensor_capacitivo{
    unsigned int id_dato  ; 
    bool state_sensor_cap  ; 
    time_t last_unix_time ; 
}; 


enum TYPE_SENSOR {
    CAPACITIVO, 
    ULTRASONIDO
}; 

typedef sensor_ultrasonic sensor_ultrasonic ; 
typedef TYPE_SENSOR type_sensor ; 
