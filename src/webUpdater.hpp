#include <WiFiClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
bool web_update_on = false ; 


void initWebUpdate(const char *user, const char *pass ){
    
    
    httpUpdater.setup(&httpServer,user,pass);
    httpServer.begin();
    web_update_on = true ; 
}

void updateSoftware(){
    httpServer.handleClient();
}