#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdlib.h>
 //librerias de ubidots para la comunicacion mqtt 
#include "UbidotsEsp32Mqtt.h"
#include <UbiConstants.h>
#include <UbiTypes.h>

//librerias para la pantalla oled integrada
#include <TFT_eSPI.h>
#include <SPI.h>


//libreria para poder comparar y usar 2 o mas sw
#include <string.h>

//archivos de imagen
#include "Humidityicon.h"
#include "temperature1icon.h"
#include "temperature2icon.h"
#include "temperature4icon.h"

//libreria del sensor de temperatura_humedad 
#include "DHT.h"



#include "data.h"
#include "Settings.h"

#define BUTTON_LEFT 0        // btn activo en bajo
#define LONG_PRESS_TIME 3000 // 3000 milis = 3s

#define DHTTYPE DHT22 //tipo de DHT usado 11 o 22 
#define DHTPIN 27 //pin donde esta conectado el sensor DHT
DHT dht(DHTPIN, DHTTYPE);

TFT_eSPI tft = TFT_eSPI(135, 240); //se define el tamaño de la pantalla

//informacion para el server ubidots como el token de mi perfil, la conexion wifi y las variables que se estan usando tanto pra enviar como para recibir
const char *UBIDOTS_TOKEN = "BBFF-uuhitNbM3hGGSaCDaHgVKtEugR2O1J";  // Put here your Ubidots TOKEN
const char *DEVICE_LABEL = "ESP32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL_1 = "TEMPERATURA"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "HUMEDAD";
const char *VARIABLE_LABEL_3 = "SW1";
const char *VARIABLE_LABEL_4 = "SW2";

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
Ubidots ubidots(UBIDOTS_TOKEN);

//pines donde esta conectado los leds
const uint8_t LED_1 = 26;
const uint8_t LED_2 = 25;

//strings o nombres de los sw que se estan susando para tambien compararlos y diferenciarlos
char str1[]="/v2.0/devices/esp32/sw1/lv";
char str2[]="/v2.0/devices/esp32/sw2/lv";



WebServer server(80);

Settings settings;
int lastState = LOW; // para el btn
int currentState;    // the current reading from the input pin
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

void load404();
void loadIndex();
void loadFunctionsJS();
void restartESP();
void saveSettings();
bool is_STA_mode();
void AP_mode_onRst();
void STA_mode_onRst();
void detect_long_press();

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    if(strcmp(str1,topic)==0){//comparacion de los topic
      
    if ((char)payload[0] == '1')//comparacion del calor que llega
    {
     Serial.print((char)payload[i]);
     digitalWrite(LED_1, HIGH); //encendido de led
     tft.fillCircle( 34,  200,  30,  TFT_GREEN);//(pos x, pos y, radio, color)cambio de color de un circulo relleno funcion dada por la libreria de la pantalla
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
    
    
  }
  Serial.println();
}

// Rutina para iniciar en modo AP (Access Point) "Servidor"
void startAP()
{
  WiFi.disconnect();
  delay(19);
  Serial.println("Starting WiFi Access Point (AP)");
  tft.drawString("Starting WiFi ",10,10,2);
  tft.drawString("Access Point (AP)",10,25,2);

  WiFi.softAP("ESP_AP", "ADMIN123");
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  
  Serial.println(IP);
  tft.drawString("1 Ingrese al IP:",10,40,2);
  tft.drawString("  192.168.1.4",10,55,2);
  tft.drawString("2 Ingrese a la Red:",10,70,2);
  tft.drawString("SSID: ESP_AP",30,85,2);
  tft.drawString("PASSWORD: ",30,100,2);
  tft.drawString("ADMIN123",30,115,2);
  tft.drawString("3 Ingrese los datos de su red ",10,130,2);
  tft.drawString("  de su red",10,145,2);

  
  
}

// Rutina para iniciar en modo STA (Station) "Cliente"
void start_STA_client()
{
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  delay(100);
  Serial.println("Starting WiFi Station Mode");
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Starting WiFi",10,10,2);
  tft.drawString("Station Mode",10,25,2);
  WiFi.begin((const char *)settings.ssid.c_str(), (const char *)settings.password.c_str());
  WiFi.mode(WIFI_STA);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
    if (cnt == 100) // Si después de 100 intentos no se conecta, vuelve a modo AP
      AP_mode_onRst();
    cnt++;
    Serial.println("attempt # " + (String)cnt);
  }

  

  WiFi.setAutoReconnect(true);
  Serial.println(F("WiFi connected"));
  tft.fillScreen(TFT_BLACK);
  tft.drawString("WiFi connected",10,10,2);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  pressedTime = millis();
  // Rutinas de Ubidots

  delay(2000);
  pinMode(LED_1, OUTPUT);// se definen los pines de los leds como salidas
  pinMode(LED_2, OUTPUT);
   
  // put your setup code here, to run once:
 
  // ubidots.setDebug(true);  // uncomment this to make debug messages available

  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_3); // Insert the dataSource and Variable's Labels suscripcion a las variables de los sw
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_4); // Insert the dataSource and Variable's Labels
  
  timer = millis();

  dht.begin();// inicializacion del sensor dht


  
}




void setup()
{

  Serial.begin(115200);
  //inicializacion de la pantalla y set del color de fondo
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  EEPROM.begin(4096);                 // Se inicializa la EEPROM con su tamaño max 4KB
  pinMode(BUTTON_LEFT, INPUT_PULLUP); // btn activo en bajo

  // settings.reset();
  settings.load(); // se carga SSID y PWD guardados en EEPROM
  settings.info(); // ... y se visualizan

  Serial.println("");
  Serial.println("starting...");

  if (is_STA_mode())
  {
    start_STA_client();
  }
  else // Modo Access Point & WebServer
  {
    startAP();

    /* ========== Modo Web Server ========== */

    /* HTML sites */
    server.onNotFound(load404);

    server.on("/", loadIndex);
    server.on("/index.html", loadIndex);
    server.on("/functions.js", loadFunctionsJS);

    /* JSON */
    server.on("/settingsSave.json", saveSettings);
    server.on("/restartESP.json", restartESP);

    server.begin();
    Serial.println("HTTP server started");
  }
  
}

void loop()
{
  if (is_STA_mode()) // Rutina para modo Station (cliente Ubidots)
  {
    delay(10);
  detect_long_press();
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // put your main code here, to run repeatedly:
  // put your main code here, to run repeatedly:

    //impresion en pantalla de los datos de humedad y temperatura
  //publicacion en pantalla de 2 imagenes
 
  tft.pushImage(0,10,16,16,temperature1icon);
  tft.pushImage(0,60,16,16,Humidityicon);

   tft.drawString(String(t),17,10,4);
   tft.drawString("ºC",87,10,4);
   tft.drawString(String(h),17,60,4);
   tft.drawString("%",87,60,4);
   
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_3); // Insert the dataSource and Variable's Labels
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_4); // Insert the dataSource and Variable's Labels
  }
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {

  //impresion en serial de los datos de humedad y temperatura
   Serial.print(F("Humidity: "));
   Serial.print(h);
   Serial.print(F("%  Temperature: "));
   Serial.print(t);
   Serial.println(F("°C "));



    ubidots.add(VARIABLE_LABEL_1, t); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);

    ubidots.add(VARIABLE_LABEL_2, h); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
  }
  else // rutina para AP + WebServer
    server.handleClient();

  
}

// funciones para responder al cliente desde el webserver:
// load404(), loadIndex(), loadFunctionsJS(), restartESP(), saveSettings()

void load404()
{
  server.send(200, "text/html", data_get404());
}

void loadIndex()
{
  server.send(200, "text/html", data_getIndexHTML());
}

void loadFunctionsJS()
{
  server.send(200, "text/javascript", data_getFunctionsJS());
}

void restartESP()
{
  server.send(200, "text/json", "true");
  ESP.restart();
}

void saveSettings()
{
  if (server.hasArg("ssid"))
    settings.ssid = server.arg("ssid");
  if (server.hasArg("password"))
    settings.password = server.arg("password");

  settings.save();
  server.send(200, "text/json", "true");
  STA_mode_onRst();
}

// Rutina para verificar si ya se guardó SSID y PWD del cliente
// is_STA_mode retorna true si ya se guardaron
bool is_STA_mode()
{
  if (EEPROM.read(flagAdr))
    return true;
  else
    return false;
}

void AP_mode_onRst()
{
  EEPROM.write(flagAdr, 0);
  EEPROM.commit();
  delay(100);
  ESP.restart();
}

void STA_mode_onRst()
{
  EEPROM.write(flagAdr, 1);
  EEPROM.commit();
  delay(100);
  ESP.restart();
}

void detect_long_press()
{
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_LEFT);

  if (lastState == HIGH && currentState == LOW) // button is pressed
    pressedTime = millis();
  else if (lastState == LOW && currentState == HIGH)
  { // button is released
    releasedTime = millis();

    // Serial.println("releasedtime" + (String)releasedTime);
    // Serial.println("pressedtime" + (String)pressedTime);

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration > LONG_PRESS_TIME)
    {
      Serial.println("(Hard reset) returning to AP mode");
      delay(500);
      AP_mode_onRst();
    }
  }

  // save the the last state
  lastState = currentState;
}