# ESP8266-Dual-temperature-Thingspeak-OLED-Webserver
Measure 2 temperatures, output to Thingspeak, Tiny webserver, OLED screen


ESP8266 script for measuring 2 temperature sensors DS18B20
Setup tiny webserver
Upload measurements to Thingspeak
Show data on OLED screen
Sense a digital 5VDC input.


Connect two DS18B20 sensors to D5 and add a pullup resistor from D5 to +3V (3k3 - 4k7)
Connect a 33k resistor from D7 to GND and 22k from D7 to input (5VDC max). This is a digital input
Connect OLED: SCA=D2, SCL=D1, GND and 3V of course :-)

Edit 'secrets.h' to suit your data
Edit the HEX uint8_t sensor1[8] and uint8_t sensor2[8] string in the script to your sensor unique number.

The script measures 2 temperature sensors (DS18B20).
Updates the OLED screen every 10 secs
Refreshes a tiny webserver
Updates Thingspeak every 5 minutes.

You will need a thingspeak account with 3 inputs: Sensor1, sensor2 and a digital input.

