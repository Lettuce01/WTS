#ifndef RTCMEM
#define RTCMEM

enum radio_status {
  RADIO_OFF = 0,
  RADIO_ON = 1
};

#define READ_RECORDS 10   // Number of temperature readings to record

typedef struct {
  uint32_t crc32;
  radio_status rs;
  uint32_t count;
  float readings[READ_RECORDS];
} rtcData;

typedef struct {
  uint32_t crc32;
  union {
    rtcData data;
    uint8_t rawdata[sizeof(rtcData)];
  };
} rtcBlob;

bool readRTCMem(rtcData *data);
bool writeRTCMem(rtcData *data);


#endif
