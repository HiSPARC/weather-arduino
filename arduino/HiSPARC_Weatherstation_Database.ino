// This sketch is for testing Weather station to Database HiSPARC

#include <OneWire.h>  // to use data from ultiple sensors send through one wire
#include <DallasTemperature.h> //library for ds18Bb20
#include <DHT.h>  // library for humidity sensors DHTxx (11, 21, 22)
#include <Wire.h>
#include <BMP085.h>

// code for digital Temperature sensor (ds18b20)
#define ONE_WIRE_BUS 3
// Data wire for temperature of detectors is plugged into pin 3 on the Arduino
// Data on temperature of all detectors is send using one datawire.
// To database means we only use the temperature of one detector. 
// Place the right temperature sensor in one of the detector.

// Setup DTH library with right sensor
DHT dht = DHT();

// Setup a oneWire instance to communicate with any OneWire devices in this case tempsensors
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tsensors(&oneWire);

BMP085 dps = BMP085();      // Digital Pressure Sensor -> dps is accessing library 

// Needed for library of BMP085
long Temperature = 0, Pressure = 0, Altitude = 1000; 


void setup(void) {
  // start serial port
  Serial.begin(9600);
  //Serial.println("HiSPARC Weather station measuring:");

  Wire.begin();
  //Serial.println();
  //Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");

  dht.setup(5); // data pin 5 humidity
    
  dps.init(MODE_ULTRA_HIGHRES, 1000, true);  // 250 meters, true = using meter units
           
  tsensors.begin();
}


void loop(void) {
  // DTH22 Reading temperature or humidity takes about 250 milliseconds!
  // DTH22 Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  delay(dht.getMinimumSamplingPeriod());
  
  tsensors.requestTemperatures(); // Get temperature of detectors (one is default)
  for (int deviceA = 0; deviceA < 1; deviceA++) {
    printTemp(deviceA);
  }
  
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  // check if returns are valid, if they are nan (not a number) then something went wrong!
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT");
  } 
  
  else {
    //Serial.print(dht.getStatusString());  // shows the status of the data read from the sensor
    //Serial.print("\t");
    Serial.print(temperature, 1); //Temperature outside
    //Serial.print(",");
    //Serial.print("-999"); // Fill in when using measuring humidity inside with sensor.
    Serial.print(",");
    Serial.print(humidity, 1);
    Serial.print(",");
  }
  
  dps.getTemperature(&Temperature);
  dps.getPressure(&Pressure);
  float pressure = Pressure/100;
  //Serial.print("  Pressure :");
  Serial.print(pressure);

  Serial.println();
  
  delay(2000);
}


void printTemp(int adress) { 
  float TempC = tsensors.getTempCByIndex(adress);
  String stringone = "Detector ";
  stringone += adress;
  
  //Serial.print(adress);
  Serial.print(" ");
  //Serial.print(adress);
  Serial.print(TempC,1);
  Serial.print(",");
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
