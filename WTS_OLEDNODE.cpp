#include <stdlib.h>
#include <ESP8266WiFi.h>

#include "WTS_OLEDNODE.h"
#include "WTS_V1_HWDEFS.h"

void setupWiFiAP()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "My ESP8266 " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);

  Serial.println("WiFi AP Name = " + AP_NameString);
  Serial.println("Wifi Name Char = " + String(AP_NameChar));
  Serial.println("WiFi Password = " + String(WiFiAPPSK));
  Serial.print("WiFi IP Address = ");
  Serial.println(WiFi.softAPIP()); 
}

int getReqID(String req) {
  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/led/0") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/led/1") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/read") != -1)
    val = -2; // Will print pin reads
  else if (req.indexOf("/temp") != -1)
    val = -3; // Will print temperature reading
  else if (req.indexOf("/volts") != -1)
    val = -4; // Will print the battery voltage
  else if (req.indexOf("/sendTemp") != -1)
    val = -5; // Client is sending a temperature value, display on OLED
  else if (req.indexOf("/sendVolts") != -1)
    val = -6; // Client is sending a battery voltage value, display on OLED
  else if (req.indexOf("/sendTem2") != -1)
    val = -7; // Client is sending an alternative temperature value, display on OLED
  // Otherwise request will be invalid.
  return val;
}

String createResponseHeader() {
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  return s;
}
