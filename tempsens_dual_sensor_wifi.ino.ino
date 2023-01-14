/**********************************************************************
 * 
 * Measure multiple (2) temperatures with (2x) DS18B20
 * Monitor D7 digital input 5V/0V (fan on/off)
 * Output as simple webserver
 * Upload to Thingspeak
 * Output to OLED screen
 * 
 * januari 2023
 * ARV
 * 
 * v1.0
 * 
 * Application: e.g. Monitor convector pit with forced convection (fans)
 *              e.g. Monitor floor heating floor/ambient temperature
 *              e.g. Monitor outside/inside temperature
 * 
 **********************************************************************/


#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ThingSpeak.h"
#include "secrets.h"

#define ONE_WIRE_BUS D5
#define dig_inputpin D7
#define TIME_THINGSPEAK 300000
#define TIME_DISPLAY 10000
#define VERSION "tempsens_dual_sensor_wifi_thingspeak_jan_2023_V1.0"

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);

float tempSensor1, tempSensor2;
uint8_t   fan;
unsigned long updateOled=0, updateThingspeak=0, timeToThingspeak=0;
String fanstring;
uint8_t sensor1[8] = { 0x28, 0xFF, 0x64, 0x1D, 0x89, 0xB3, 0xFC, 0x8F  }; // sensor specific
uint8_t sensor2[8] = { 0x28, 0xFF, 0x64, 0x1D, 0x89, 0x8C, 0x27, 0xCD  }; // sensor specific
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
int httpCode=0;

/*Put your SSID & Password*/
const char* ssid = SECRET_SSID;  // Enter SSID here
const char* password = SECRET_PASS;  //Enter Password here

ESP8266WebServer server(80);          
WiFiClient clientWiFi;  // for thingspeak   
 
void setup() {
  Serial.begin(115200);
  pinMode(dig_inputpin, INPUT);  // for fan activity
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }  
  Serial.println(VERSION); //good practice to keep track of the version utilised..
  delay(2000); // experiment with lower values...
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  sensors.begin();              

  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("Trying .."); 
  display.setCursor(0,20);
  display.setTextSize(2);display.print(ssid);display.display();
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  display.setCursor(0,40);display.setTextSize(1);
  display.print("IP "); display.print(WiFi.localIP());display.display();

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  updateOled=millis();
  updateThingspeak=millis();
}
void loop() {
  sensors.requestTemperatures();
  tempSensor1 = sensors.getTempC(sensor1); // Gets the values of the temperature
  tempSensor2 = sensors.getTempC(sensor2); // Gets the values of the temperature
  fan = digitalRead(dig_inputpin);
  fanstring = fan ? "On" : "OFF";
  server.handleClient();

  if ((millis() - updateOled) > TIME_DISPLAY) {  //update OLED display
    Serial.print("Temperature1 = "); Serial.print(tempSensor1); Serial.println(" *C");
    Serial.print("Temperature2 = "); Serial.print(tempSensor2); Serial.println(" *C");
    Serial.print("Fan = "); Serial.println(fanstring);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.clearDisplay();

    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("1:   ");
    display.setTextSize(2);
    display.print(tempSensor1);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
    // 2nd sensor
    display.setCursor(0,20);
    display.setTextSize(1);
    display.print("2:   ");
    display.setTextSize(2);
    display.print(tempSensor2);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
    // fan on/off
    display.setCursor(0,40);
    display.setTextSize(1);
    display.print("Fan: ");
    display.setTextSize(2);
    display.print(fanstring);

    timeToThingspeak = (TIME_THINGSPEAK - (millis()-updateThingspeak))/1000;
    display.print("  ");display.print(timeToThingspeak);

    Serial.print("Time to Thingspeak: "); Serial.print(timeToThingspeak);
    Serial.println();
    Serial.print("a high = "); Serial.println(HIGH);
    Serial.print("a low = "); Serial.println(LOW);
    Serial.print("FAN = ");Serial.print(fan);
    display.display();
    updateOled = millis();
  }
  if ((millis() - updateThingspeak) > TIME_THINGSPEAK) { // update Thingspeak
    Serial.println("Starting Thingspeak connection");
    ThingSpeak.begin(clientWiFi);
    ThingSpeak.setField(1, tempSensor1);
    ThingSpeak.setField(2, tempSensor2);
    ThingSpeak.setField(3, fan);
    httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Thingspeak..");
    display.setCursor(0,20);
    if (httpCode == 200) {
      Serial.println("Channel write successful.");
      display.print("OK");
    }
    else {
      Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
      display.print("Problems ....");
    }      
    display.display();
    updateThingspeak = millis();
  }
  delay(2000);
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(tempSensor1,tempSensor2,fan)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float tempSensor1,float tempSensor2, int fan){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"10\" >";
  ptr +="<title>ESP8266 Temperature Monitor</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Monitor</h1>\n";
  ptr +="<p>Sensor 1: ";
  ptr +=tempSensor1;
  ptr +="&deg;C</p>";
  ptr +="<p>Sensor 2: ";
  ptr +=tempSensor2;
  ptr +="&deg;C</p>"; 
  ptr +="<p>Fan: ";
  ptr +=fanstring;
  ptr +="</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
