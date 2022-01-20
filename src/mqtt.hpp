#include <PubSubClient.h>
#include <WiFiClient.h> 
#include <webUpdater.hpp>
#include <WiFiUdp.h>
#define PORT_NTC 2390 
#define PIN_TRIGGER 12 // 
#define PIN_ECHO   14 //
#define NTP_PACKET_SIZE 48
#define SERVER_NTC "gps.iar.unlp.edu.ar" //servidor NTC 
#define BROKER_MQTT "163.10.43.85"
#define TOPIC_1_MQTT "/iar/salaMaquinas/sensorUltrasonido"
#define TOPIC_2_MQTT "/iar/salaMaquinas/upgrade"
#define ID_SENSOR_1 "ULTR"
#define ID_MSG_SENSOR_2 "CAPA" 
#define MAC_ADDRESS "A4:CF:12:EF:7E:0B"
#define PORT_MQTT 1883  // PORT_INSECURE
 //sensor de distancia 
unsigned long int id_dato_sensor_distancia = 0 ;
 //sensor capacitivo
unsigned long int id_dato_sensor_capacitivo = 0 ; 
//dia - mes - año  hh:mm:ss
int date [6]; //
// clases para el manejo de MQTT y NTC  
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP udp; 
// obtiene la hora usando NTC 
time_t getHourNTC() ; 
void sendPacketNTP(IPAddress& address) ; 
void uploadONMQTT(char *topic, byte *payload, unsigned int length);




//DEFINICIÓN DE CALLBACKS Y SETTEAR SERVIDOR MQTT   
void initMQTT()
{ 
  randomSeed(micros()) ;
  client.setServer(BROKER_MQTT, 1883); 
  client.setCallback(uploadONMQTT) ; 
  udp.begin(PORT_NTC);
}

void publishmqtt() { 
    
    //id_dato,timestamp, id_mcu,id_sensor, dato  
    /* --ber_data++ -->init in one 
      id_dato --> incre>id_dato: nummenta en 1,
      timestamp --> ntc
      id_mcu = "mac"
      dato 
    */
   
   // getHourNTC() ; 
    //id_dato,timestamp, id_mcu,id_sensor, dato  
    char payload[41] ; 
    /*                //  
    sprintf(payload,"%ld ,%d/%d/%d %02d:%02d:%02d,%s,%s,%d" ,id_dato_sensor_1,date[0],date[1],
            date[2],date[3],date[4],date[5],MAC_ADDRESS,
            ID_SENSOR_1, distance); */ 
    
   // client.publish(TOPIC_1_MQTT,payload) ; 
}






void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    // Create a random client ID
    String clientId = "ESP8266SALAMAQUINA";
    clientId += String(random(0xffff), HEX) ; 
    randomSeed(millis()) ;
    clientId += String(random(0xffff), HEX) ; 
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(TOPIC_2_MQTT) ; 
    
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
    struct tm  ts;    
    ts = *localtime(&raw_secods);
    date[0] =  ts.tm_mday ; 
    date[1] = ts.tm_mon+1 ; 
    date[2] = ts.tm_year +1900;
    date[3] =  ((ts.tm_hour+24) -3 )% 24 ; 
    date[4] = ts.tm_min ; 
    date[5] = ts.tm_sec ; 
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