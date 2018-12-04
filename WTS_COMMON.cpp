#include <stdlib.h>
#include <ESP8266WiFi.h>

#include "WTS_COMMON.h"

float readBattVolts()
{
  unsigned int algV;
  float battV;
  algV = analogRead(A0);
  battV = (algV * 5.1) / 1024;
  return battV;
}

char *get_MAC_addr()
{
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  String mac_Str = String(mac[WL_MAC_ADDR_LENGTH - 6], HEX) + "." +
                   String(mac[WL_MAC_ADDR_LENGTH - 5], HEX) + "." +
                   String(mac[WL_MAC_ADDR_LENGTH - 4], HEX) + "." +
                   String(mac[WL_MAC_ADDR_LENGTH - 3], HEX) + "." +
                   String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + "." +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  char mac_char[mac_Str.length() + 1];
//  memset(mac_char, 0, mac_Str.length() + 1);
//  for (int i=0; i<mac_Str.length(); i++)
//    mac_char[i] = mac_Str.charAt(i);
  mac_Str.toCharArray(mac_char, mac_Str.length());
  return mac_char;
//  return mac_Str;
}
