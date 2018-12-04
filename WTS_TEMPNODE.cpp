#include <stdlib.h>
#include <ESP8266WiFi.h>

#include "WTS_TEMPNODE.h"
#include "WTS_V1_HWDEFS.h"

String sendGETRequest(WiFiClient *client, IPAddress target, String messageID, String value){
  String response;
//    Serial.printf("\n[Connecting to %s ... ", "192.168.4.1");
    if(client->connect(target, 80))
    {
//      Serial.println("connected]");
  
//      Serial.println("[Sending a request]");
      client->print(String("GET /") + messageID + String("/") + value + " HTTP/1.1\r\n");
  
//      Serial.println("[Response:]");
      while (client->connected())
      {
        if (client->available())
        {
          response = client->readStringUntil('\n');
          //Serial.println(line);
        }
      }
      client->stop();
//      Serial.println("\n[Disconnected]");
    }
    else
    {
      //Serial.println("connection failed!]");
      response = "Connection failed!";
      client->stop();
    }
    return response;
}
