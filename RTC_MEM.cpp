#include <stdlib.h>
#include <ESP8266WiFi.h>

#include "RTC_MEM.h"

uint32_t calculateCRC32(const uint8_t *data, size_t length);

bool readRTCMem(rtcData *data) {
  rtcBlob b;
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &b, sizeof(rtcBlob))) {
    uint32_t crcOfData = calculateCRC32((uint8_t*) &b.rawdata[0], sizeof(rtcData));
    if (crcOfData != b.crc32) {
      return false;
    } else {
      *data = b.data;
    }
    return true;
  }
  return false;
}

bool writeRTCMem(rtcData *data) {
  rtcBlob b;
  b.data = *data;
  b.crc32 = calculateCRC32((uint8_t*) &data[0], sizeof(rtcData));
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &b, sizeof(rtcBlob))) {
    return true;
  } else {
    return false;
  }
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

