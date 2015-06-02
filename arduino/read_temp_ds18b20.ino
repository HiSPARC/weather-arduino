#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


void setup(void) {
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();  
}


void loop() { 
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); 
  Serial.println("DONE");
//  Serial.print("TempDevice 1:");
//  Serial.println(sensors.getTempCByIndex(0)); 
//  Serial.print("TempDevice 2 :");
//  Serial.println(sensors.getTempCByIndex(1));
//  Serial.print("TempDevice 3 :");
//  Serial.println(sensors.getTempCByIndex(2));
//  Serial.print("TempDevice 4 :");
//  Serial.println(sensors.getTempCByIndex(3));  
  for (int deviceA = 0; deviceA < 4; deviceA++) {
    printTemp(deviceA);
  }
  delay(500);
}


void printTemp(int adress) {  
  float TempC = sensors.getTempCByIndex(adress);
  String stringone = "TempDevice ";
  stringone += adress;
  Serial.print(stringone);
  Serial.print("  ");
  // Serial.print(adress);
  Serial.println(TempC);  
}
