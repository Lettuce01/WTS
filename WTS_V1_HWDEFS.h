#ifndef HWDEFS
#define HWDEFS

//////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiAPPSK[] = "letmein";   // Access Point Password
//const char WiFiSSID[] = "ESP_91E6BC";  // SSID of ESP8266 acting as server
const char WiFiSSID[] = "ESP_91E53C";  // SSID of ESP8266 acting as server


// Hardware definition
#define PIN_IO0       0
#define PIN_I2C_SDA   2
#define PIN_CS_TEMP   4
#define PIN_SPI_SCK   5
#define PIN_SPI_SIO   12
#define PIN_I2C_SCL   14
#define PIN_BUTTON    15

//#ifdef TEMP_NODE
#define PIN_LED       13
//#else
//#define PIN_LED       16
//#endif




#endif
