// This sketch is the base readout sketch for a HiSPARC Weatherstation
// We measure Humidity, Pressure, Temperature and temperature of every detector.
// DHT22 for humidity, DTH22 (AM2302), sends temperature as well
// !!!!!!check humidity sensor-raw voor dauwpunt etc. 
// pressure: BMP085 of BMP180, temperature measurement
// ds18b20 temperaturesensor
// While we use different sensors and online resources some of the code is used from (open source websites) as well
// To support efforts from other programmers we include their comments as well.

#include <OneWire.h>  // to use data from ultiple sensors send through one wire
#include <DallasTemperature.h> //library for ds18Bb20
#include <DHT.h>  // library for humidity sensors DHTxx (11, 21, 22)
#include <Wire.h>
#include <Adafruit_BMP085.h> // library for pressure sensor

// #include "DHT.h"  is this code ok ->check next time????

#define DHTTYPE DHT22   // DHT 22  (AM2302) using other sensor: change to DHT11 
#define DHTPIN 2   // the pin no. used
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// code for digital Temperature sensor (ds18b20)
#define ONE_WIRE_BUS 3
// Data wire for temperature of detectors is plugged into pin 3 on the Arduino
// Data on temperature of all detectors is send using one datawire.

// Setup DTH library with right sensor
DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices 
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tsensors(&oneWire);

// Setup BMP085 library
Adafruit_BMP085 bmp;


void setup(void) {
  // start serial port
  Serial.begin(9600);
  Serial.println("HiSPARC Weather station measuring:");

  // if you want to add safety or check for sensor!
  // inlcude this line: Change !bmp.begin() to !sensors.begin() or !dht.begin() to check the other sensors
  //if (!bmp.begin()) {
  //  Serial.println("Could not find a valid BMP085 sensor, check wiring!");}
  
  // WithoutStart up the libraries
  tsensors.begin();
  dht.begin();
  bmp.begin();
}


// 12 mei change to send using wireless
void loop(void) {
  // DTH22 Reading temperature or humidity takes about 250 milliseconds!
  // DTH22 Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");
  }
  
  //BMP085 pressure sensor
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");
  
  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(101500));
  Serial.println(" meters");

  // ds18b20 tempsensor code for digital temperature
  // ds18b20 tempsensor request to all devices on the bus
  Serial.print(" Requesting temperatures..."); //remove this when sending data to HiSPARC database.
  tsensors.requestTemperatures(); 
  Serial.println("DONE");
  // ds18b20 tempsensor: You can have more than one IC on the same bus. Two for two detectorstation and four for four detectorstation
  // ds18b20 tempsensor: 0 refers to the first IC on the wire
  Serial.print("TempDevice 1:");
  Serial.println(sensors.getTempCByIndex(0)); 
  Serial.print("TempDevice 2 :");
  Serial.println(sensors.getTempCByIndex(1));
  Serial.print("TempDevice 3 :");
  Serial.println(sensors.getTempCByIndex(2));
  Serial.print("TempDevice 4 :");
  Serial.println(sensors.getTempCByIndex(3));
   
  delay(1000);
}


//*************************************************** 
//  This is an example for the BMP085 Barometric Pressure & Temp Sensor
//
//  Designed specifically to work with the Adafruit BMP085 Breakout 
//  ----> https://www.adafruit.com/products/391
//
//  These displays use I2C to communicate, 2 pins are required to  
//  interface
//  Adafruit invests time and resources providing this open source code, 
//  please support Adafruit and open-source hardware by purchasing 
//  products from Adafruit!
//
//  Written by Limor Fried/Ladyada for Adafruit Industries.  
//  BSD license, all text above must be included in any redistribution
// ****************************************************/
