// ESP Wireless Temperature Sensor
// Board: ESP8266 WTS V1
// Module: Espressif ESP-WROOM-02
// Date: July 2018

// ESP_WROOM_02 specs:
// Flash size = 2M
// Crystal Frequency = 26MHz

//#define TEMP_NODE   // Remote Station node, battery powered and periodically transmitting data.
#define OLED_NODE   // Central AP node, receiving data and displaying on an OLED display.

#define TEMP_THRESH 0.2 // Temperature variation threshold to trigger an update from TEMP_NODE to OLED_NODE

#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <String.h>

// Includes for the temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Includes for the OLED display
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "WTS_V1_HWDEFS.h"
#include "RTC_MEM.h"
#include "WTS_COMMON.h"
#include "WTS_TEMPNODE.h"
#include "WTS_OLEDNODE.h"

#define SLEEP_TIME 30e6

unsigned int ConnectRetries = 25;

 
#ifdef OLED_NODE
String localTemp;
String localVolts;
String remoteTemp;
String remoteVolts;
String remoteTem2;
unsigned int updateCount = 0;
unsigned int updateCycles = 500;
unsigned int waitCount = 0;     // Measure of time since remote node last sent an update
unsigned int waitCycles = 50;

WiFiServer server(80);

Adafruit_SSD1306 display(-1);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#endif


OneWire oneWireB(PIN_SPI_SIO);
DallasTemperature MAX31820(&oneWireB);

// This was for the remote DS1820 probe on the temp node
//#ifdef TEMP_NODE
//OneWire oneWire(2);
//DallasTemperature DS1820(&oneWire);
//#endif

float nextVoltThresh = 5.0;

WiFiClient client;

void setup() 
{
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(74880);

#ifdef OLED_NODE
  setupWiFiAP();
  server.begin();
#endif

#ifdef TEMP_NODE
  uint32_t sleepTime = SLEEP_TIME;
  Serial.println("Woken...");
  rtcData data;
  bool result;
  Serial.print("Reading RTC data... ");
  result = readRTCMem(&data);
  if (result == false) {
    Serial.println("Data not valid");
    Serial.println("Setting data to RADIO_OFF and 1");
    data.rs = RADIO_OFF;
    data.count = 1;
    result = writeRTCMem(&data);
    if (result == true) {
      Serial.println("Data saved to RTC");
    } else {
      Serial.println("Write to RTC failed");
    }
  } else {
    Serial.println("Successful");
    Serial.print("radio_status = ");
    Serial.println((data.rs == RADIO_OFF)?"RADIO_OFF": "RADIO_ON");
    Serial.print("count = ");
    Serial.println(data.count);

    if (data.rs == RADIO_ON) {
      WiFi.mode(WIFI_STA);
      Serial.printf("Connecting to %s ", WiFiSSID);
      WiFi.begin(WiFiSSID, WiFiAPPSK);
      while ((WiFi.status() != WL_CONNECTED) & (ConnectRetries > 0))
      {
        delay(500);
        Serial.print(".");
        ConnectRetries--;
      }
      if (ConnectRetries > 0)
      {
        Serial.println(" connected");
      } else {
        // Note: this does not actually cancel the attempt to connect...
        Serial.println(" could not connect");
      }
      if (ConnectRetries > 0) {
        int count;
        if (data.count >= 10) {
          count = 9;
        } else {
          count = data.count;
        }
        Serial.print("count = ");
        Serial.println(data.count);
        Serial.print("data = {");
        for (int k = 0; k <= count; k++) {
          Serial.print(data.readings[k]);
          Serial.print(", ");
        }
        Serial.println("}");
        tempNodeSend(data.readings[count]);
      }

      data.count = 0;
      
    } else {
      data.count++;
    }


    float temp;
    int i = 0;
  
    do {
      MAX31820.requestTemperatures();
      temp = MAX31820.getTempCByIndex(0);
      delay(100);
      i++;
    } while ((temp == 85.0 || temp == (-127.0)) && i < 500);
    
    
    if (data.count < READ_RECORDS) {
      data.readings[data.count] = temp;
    }
    data.rs = RADIO_OFF;
    if (((temp - data.readings[0]) > TEMP_THRESH) || ((data.readings[0] - temp) > TEMP_THRESH)) {
      Serial.print("new temp (");
      Serial.print(temp);
      Serial.print(") differs from first record (");
      Serial.print(data.readings[0]);
      Serial.print(") by more than ");
      Serial.print(TEMP_THRESH);
      Serial.println(" :- sending data.");
      data.rs = RADIO_ON;
      sleepTime = 2e6;
    } else if (data.count >= READ_RECORDS) {
      data.rs = RADIO_ON;
    }
    
    Serial.print("Updating RTC data... ");
    result = writeRTCMem(&data);
    if (result == true) {
      Serial.println("Data saved to RTC");
    } else {
      Serial.println("Write to RTC failed");
    }
  }


  
#endif

  Serial.print("Local IP = ");
  Serial.println(WiFi.localIP());

  delay(500); //Wait half a second

#ifdef OLED_NODE
//  localTemp = String(tempSensor.readTemp());
  float temp;
  int i = 0;  
  do {
      MAX31820.requestTemperatures();
      temp = MAX31820.getTempCByIndex(0);
      delay(100);
      i++;
  } while ((temp == 85.0 || temp == (-127.0)) && i < 500);
  localTemp = temp;
  localVolts = String(readBattVolts());
  remoteTemp = "---";
  remoteVolts = "---";
//  remoteTem2 = "---";

  Wire.pins(PIN_I2C_SDA, PIN_I2C_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
  display.setRotation(2);
  display.display();
  delay(2000); 

  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  
  display.println("Hello World!!");
  display.display();
#endif

#ifdef TEMP_NODE

  if (data.rs == RADIO_ON) {
    Serial.println("Sleeping (Radio ON)...");
    delay(500);
    //ESP.deepSleep(SLEEP_TIME);  
    ESP.deepSleep(sleepTime);
  } else {
    Serial.println("Sleeping (Radio OFF)...");
    delay(500);
    //ESP.deepSleep(SLEEP_TIME, WAKE_RF_DISABLED);
    ESP.deepSleep(sleepTime, WAKE_RF_DISABLED);
  }
#endif

}

uint32 nextTime = 0;
uint32 tempNodeAccessCount = 0;
uint32 hourCount = 0;

void loop() 
{


#ifdef OLED_NODE
  uint32 m = millis();
  if (m > nextTime) {
    nextTime = m + (60 * 60 * 1000);
    Serial.print("Hour #");
    Serial.println(hourCount);
    hourCount++;
    Serial.print("Temp node has sent an update ");
    Serial.print(tempNodeAccessCount);
    Serial.println(" times in the last hour");
    tempNodeAccessCount = 0;
  }

  if (updateCount > updateCycles)
  {
    updateCount = 0;
    //localTemp = String(tempSensor.readTemp());=

    float temp;
    int i = 0;
  
    do {
      MAX31820.requestTemperatures();
      temp = MAX31820.getTempCByIndex(0);
      delay(100);
      i++;
    } while ((temp == 85.0 || temp == (-127.0)) && i < 500);

    
    localTemp = String(temp);
    localVolts = String(readBattVolts(), 1);
  }
  updateCount++;

  if (updateCount % waitCycles == 0)
  {
    if (waitCount < 320) waitCount++;
  }

  Draw_Display();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  tempNodeAccessCount++;

  waitCount = 0;

  // Read the first line of the request
  String req = client.readStringUntil('\r');
//  Serial.println(req);
  client.flush();

  int val = getReqID(req);

  // Set GPIO5 according to the request
  if (val >= 0)
    digitalWrite(PIN_LED, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = createResponseHeader();

  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    s += "LED is now ";
    s += (val)?"on":"off";
  }
  else if (val == -2)
  { // If we're reading pins, print out those values:
    s += "Analog Pin = ";
    //s += String(analogRead(ANALOG_PIN));
    s += "3.141";
    s += "<br>"; // Go to the next line.
    s += "Digital Pin 12 = ";
    //s += String(digitalRead(DIGITAL_PIN));
    s += "Pi";
  }
  else if (val == -3)
  {
    float temp;
    int i = 0;
  
    do {
      MAX31820.requestTemperatures();
      temp = MAX31820.getTempCByIndex(0);
      delay(100);
      i++;
    } while ((temp == 85.0 || temp == (-127.0)) && i < 500);

    s += "Temperature = ";
    s += temp;
  }
  else if (val == -4)
  {
    s += "Battery Voltage = ";
    s += readBattVolts();
  }
  else if (val == -5)
  {
    String tempIn = req.substring(req.indexOf("/sendTemp") + 10, req.indexOf("HTTP")-1);
    s += "Thank you for temperature reading of ";
    s += tempIn;
    Serial.print("Received temperature of ");
    Serial.println(tempIn);

    remoteTemp = tempIn.substring(0, 4);
  }
  else if (val == -6)
  {
    String voltsIn = req.substring(req.indexOf("/sendVolts") + 11, req.indexOf("HTTP")-1);
    s += "Thank you for voltage reading of ";
    s += voltsIn;
    Serial.print("Received voltage of ");
    Serial.println(voltsIn);

    remoteVolts = voltsIn.substring(0,3); 
  }
  else if (val == -7)
  {
    String tempIn = req.substring(req.indexOf("/sendTem2") + 10, req.indexOf("HTTP")-1);
    s += "Thank you for temperature reading of ";
    s += tempIn;
    Serial.print("Received remote temperature of ");
    Serial.println(tempIn);

    remoteTem2 = tempIn.substring(0, 4);
  }
  else
  {
    s += "Invalid Request.<br> Try /led/1, /led/0, /temp or /volts.";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
//  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
#endif
}

void Draw_Display()
{
#ifdef OLED_NODE
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("local  ");
  display.setTextSize(2);
  display.print(localTemp);
  display.setTextSize(1);
  display.setCursor(100,0);
  display.print(localVolts);
  display.setCursor(0,16);
  display.print("remote ");
  display.setTextSize(2);
//  if (updateCount > (updateCycles/2))
//  {
//    display.print(remoteTem2);
//    display.fillRect(95, 28, 3, 3, WHITE);
//  } 
//  else
//  {
    display.print(remoteTemp);
//    display.fillRect(35, 28, 3, 3, WHITE);
//  }
  display.setTextSize(1);
  display.setCursor(100,16);
  display.print(remoteVolts);
  display.drawLine(0, 31, waitCount / 10, 31, WHITE);
  display.display();
#endif
}

void tempNodeSend(float tempVal)
{
#ifdef TEMP_NODE
IPAddress target(192,168,4,1);
  
  float bv = readBattVolts();
  nextVoltThresh = (bv - 0.02);

  String resp;
  Serial.print("Voltage = " + String(bv) + "V. Sending... ");
  resp = sendGETRequest(&client, target, "sendVolts", String(bv) + "V");
  Serial.println(resp);

  float temp;
  int i = 0;

  do {
    MAX31820.requestTemperatures();
    temp = MAX31820.getTempCByIndex(0);
    delay(100);
    i++;
  } while ((temp == 85.0 || temp == (-127.0)) && i < 500);
  Serial.print("MAX31820 temp = " + String(temp) + "degC (i=" + i + "). Sending... ");

  temp = tempVal;
  
  resp = sendGETRequest(&client, target, "sendTemp", String(temp) + "C");
  Serial.println(resp);
/*
  Serial.printf("\n[Connecting to %s ... ", "192.168.4.1");
    //if (client.connect({192,168,4,1}, 80))
    //if (client.connect("192.168.4.1", 80))
    if(client.connect(target, 80))
    {
      Serial.println("connected]");
  
      Serial.println("[Sending a request]");
      client.print(String("GET /sendTemp/") + temp + "C HTTP/1.1\r\n");
  
      Serial.println("[Response:]");
      while (client.connected())
      {
        if (client.available())
        {
          String line = client.readStringUntil('\n');
          Serial.println(line);
        }
      }
      client.stop();
      Serial.println("\n[Disconnected]");
    }
    else
    {
      Serial.println("connection failed!]");
      client.stop();
    }
*/
//  temp = 0;
//  i = 0;
  
//  do {
//    DS1820.requestTemperatures();
//    temp = DS1820.getTempCByIndex(0);
//    delay(100);
//    i++;
//  } while ((temp == 85.0 || temp == (-127.0)) && i < 500);
//  Serial.print("DS1820 temp = " + String(temp) + "degC (i=" + i + "). Sending... ");
//  resp = sendGETRequest(&client, target, "sendTem2", String(temp) + "C");
//  Serial.println(resp);

/*
  Serial.printf("\n[Connecting to %s ... ", "192.168.4.1");
    //if (client.connect({192,168,4,1}, 80))
    //if (client.connect("192.168.4.1", 80))
    if(client.connect(target, 80))
    {
      Serial.println("connected]");
  
      Serial.println("[Sending a request]");
      client.print(String("GET /sendTem2/") + temp + "C HTTP/1.1\r\n");
  
      Serial.println("[Response:]");
      while (client.connected())
      {
        if (client.available())
        {
          String line = client.readStringUntil('\n');
          Serial.println(line);
        }
      }
      client.stop();
      Serial.println("\n[Disconnected]");
    }
    else
    {
      Serial.println("connection failed!]");
      client.stop();
    }
*/  


#endif
}







