/******************************************
 *
 * This example works for both Industrial and STEM users.
 *
 * Developed by Jose Garcia, https://github.com/jotathebest/
 *
 * ****************************************/

/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <UbiConstants.h>
#include <UbiTypes.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <string.h>
#include "Humidityicon.h"
#include "temperature1icon.h"
#include "temperature2icon.h"
#include "temperature4icon.h"
#include "DHT.h"

/****************************************
 * Define Constants
 ****************************************/

#define DHTTYPE DHT22 
#define DHTPIN 27
DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI(135, 240);

const char *UBIDOTS_TOKEN = "BBFF-uuhitNbM3hGGSaCDaHgVKtEugR2O1J";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "CAROLAY";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "9811040359";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "ESP32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL_1 = "TEMPERATURA"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "HUMEDAD";
const char *VARIABLE_LABEL_3 = "SW1";
const char *VARIABLE_LABEL_4 = "SW2";

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
Ubidots ubidots(UBIDOTS_TOKEN);

const uint8_t LED_1 = 26;
const uint8_t LED_2 = 25;

char str1[]="/v2.0/devices/esp32/sw1/lv";
char str2[]="/v2.0/devices/esp32/sw2/lv";

/****************************************
 * Auxiliar Functions
 ****************************************/


void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    if(strcmp(str1,topic)==0){
      
    if ((char)payload[0] == '1')
    {
     Serial.print((char)payload[i]);
     digitalWrite(LED_1, HIGH); 
     tft.fillCircle( 34,  200,  30,  TFT_GREEN);
    }
    else{
      Serial.print((char)payload[i]);
      digitalWrite(LED_1, LOW);
      tft.fillCircle( 34,  200,  30,  TFT_DARKGREY);
    }
    }
    if(strcmp(str2,topic)==0){
          if ((char)payload[0] == '1')
    {
      Serial.print((char)payload[i]);
      digitalWrite(LED_2, HIGH);
      tft.fillCircle( 102,  200,  30,  TFT_RED);
    }else{
      Serial.print((char)payload[i]);
      digitalWrite(LED_2, LOW);
      tft.fillCircle( 102,  200,  30,  TFT_DARKGREY);
    }
    }
    
     
//    if ((char)payload[0] == '1')
//    {
//      digitalWrite(LED_1, HIGH); 
//      digitalWrite(LED_2, LOW);  
//      tft.fillCircle( 34,  200,  30,  TFT_GREEN);
//      tft.fillCircle( 102,  200,  30,  TFT_DARKGREY);
//    }
//       else if ((char)payload[0] == '2')
//    {
//      digitalWrite(LED_2, HIGH);   
//      digitalWrite(LED_1, LOW);
//      tft.fillCircle( 102,  200,  30,  TFT_RED);
//      tft.fillCircle( 34,  200,  30,  TFT_DARKGREY);
//    }
//    else if ((char)payload[0] == '3')
//    {
//      digitalWrite(LED_2, HIGH);   
//      digitalWrite(LED_1, HIGH);
//      tft.fillCircle( 34,  200,  30,  TFT_GREEN);
//      tft.fillCircle( 102,  200,  30,  TFT_RED);
//      
//    }
//    else
//    {
//      digitalWrite(LED_1, LOW);
//      digitalWrite(LED_2, LOW);
//      tft.fillCircle( 34,  200,  30,  TFT_DARKGREY);
//      tft.fillCircle( 102,  200,  30,  TFT_DARKGREY);
//    }
 
    
  }
  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
   pinMode(LED_1, OUTPUT);
   pinMode(LED_2, OUTPUT);
   
  // put your setup code here, to run once:
  Serial.begin(115200);
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_3); // Insert the dataSource and Variable's Labels
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_4); // Insert the dataSource and Variable's Labels
  
  timer = millis();

  dht.begin();

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0,10,16,16,temperature1icon);
  tft.pushImage(0,60,16,16,Humidityicon);
}

void loop()
{

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // put your main code here, to run repeatedly:
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_3); // Insert the dataSource and Variable's Labels
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_4); // Insert the dataSource and Variable's Labels
  }
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {

   Serial.print(F("Humidity: "));
   Serial.print(h);
   Serial.print(F("%  Temperature: "));
   Serial.print(t);
   Serial.println(F("°C "));

  
   tft.drawString(String(t),17,10,4);
   tft.drawString("ºC",87,10,4);
   tft.drawString(String(h),17,60,4);
   tft.drawString("%",87,60,4);

    ubidots.add(VARIABLE_LABEL_1, t); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);

    ubidots.add(VARIABLE_LABEL_2, h); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
}
