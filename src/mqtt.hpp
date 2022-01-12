#include <PubSubClient.h>
#include <WiFiClient.h> 
#include <webUpdater.hpp>
#define PIN_TRIGGER 12 // 
#define PIN_ECHO   14 //

#define BROKER_MQTT "163.10.43.85"
#define TOPIC_1_MQTT "/iar/salaMaquinas/sensorUltrasonido"
#define TOPIC_2_MQTT "/iar/salaMaquinas/upgrade"

#define PORT_MQTT 1883  // PORT INSECURE 

int readUltrasonicSensor() ; 
void uploadONMQTT(char *topic, byte *payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);



void initMQTT(){ 
    pinMode(PIN_TRIGGER,OUTPUT) ; 
    pinMode(PIN_ECHO,INPUT) ; 
    
    randomSeed(micros()) ;
    client.setServer(BROKER_MQTT, 1883); 
    client.setCallback(uploadONMQTT) ; 
}

void publishmqtt() { 
    int distance = readUltrasonicSensor() ; 
    Serial.print("distancia: ") ; Serial.println(distance) ; 
    char payload[3] ; 
    sprintf(payload,"%d",distance);  //destino, fuente, tam 
    client.publish(TOPIC_1_MQTT,payload) ; 
}


int readUltrasonicSensor(){ 
    digitalWrite(PIN_TRIGGER,HIGH) ; 
    delayMicroseconds(10) ; 
    digitalWrite(PIN_TRIGGER,LOW) ; 
    unsigned long int miliseconds_response = pulseIn(PIN_ECHO,HIGH);
    float distance_cm = 0.034 * (miliseconds_response/2) ; 
    int distance = (int) distance_cm ; 
    return distance ; 
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266SALAMAQUINA";
    clientId += String(random(0xffff), HEX) ; 
    randomSeed(millis()) ;
    clientId += String(random(0xffff), HEX) ; 
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(TOPIC_2_MQTT) ; 
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(2000);
    }
  }
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
  Serial.print("user: ") ; Serial.println(user) ; 
  Serial.print("pass: ") ; Serial.println(pass) ; 

  initWebUpdate(user.c_str(), pass.c_str() ) ; 


}