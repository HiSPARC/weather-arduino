/* This program is intended for use as a test program for the CanSat course at NAROM, Andøya Rocket Range 2013.

** Beerware, this code is free to use, but if you use it and happen to meet me, you can buy me a beer
**   by Thomas Gansmoe, Narom AS (www.narom.no / www.rocketrange.no)  */

#include <DHT.h>  // library for humidity sensors DHTxx (11, 21, 22)
#include <BMP085.h>
#include <Wire.h>

#define BITRATE 9600         //Bitrate for the serial interface (Radio and logger)
#define Vcc 5.0              //Supply voltage

// Setup DTH library with right sensor
DHT dht;

BMP085 dps = BMP085();      // Digital Pressure Sensor -> dps is accessing library 

//needed for library of BMP085
long Temperature = 0, Pressure = 0, Altitude = 1000; //set alti
// **** [ Sensor calibration ] *************************************************************


// Pressure sensor
// Not implemented in the code yet

/*******************************************************************************/

int LOOPTIME = 500;         //Interval time in millisecounds
//Configure sensor output format
int OutFormatTmp = 0;       //Sets the output format. (0 = Volt, 1 = Deg.C)
int OutFormatNTC = 0;       //Sets the output format. (0 = Volt, 1 = Deg.C)
int OutFormatPressure = 0;  //Sets the output format. (0 = Volt, 1 = hPa)
int OutFormatAcc = 0;       //Sets the output format. (0 = Volt, 1 = G)

unsigned long time;
long int counter=0, number=0;
float G = 9.81; 

void setup() {
  Serial.begin(BITRATE);
  
}

float SecFromStart(){
  time = millis();
  int sec = time/1000;
  int des = (time%1000)/10;
  float result = (float)sec+((float)des)/100;
  return result;
}

void printdata(){
  Serial.print("Counter: ");
 
  Serial.print(" | Time[s]: ");
  Serial.print(SecFromStart());
    
  PrintTmp(OutFormatTmp);
  PrintNTC(OutFormatNTC);
  PrintPressure(OutFormatPressure);
  PrintAcc(OutFormatAcc);
  
  Serial.println();
}

float BitToVolt(int n){    //Function to convert raw ADC-data (0-255) to volt
  int raw = analogRead(n);
  float volt = (float)raw*5.000/1023;
  return volt;
}

// The LM335 temp sensor 
void PrintTmp(int Format){
  float Tmp, volt = BitToVolt(5);  //Reads Analog5
  Serial.print(" | Temp: ");
  switch (Format){
    case 0:
      Serial.print(volt,3);
      Serial.print(" V");
      break;
    case 1:
      Serial.print((volt - TmpOffset)*TmpSens);
      Serial.print(" Deg.C");
      break;
    }    
  }




void DHTREAD(){
  // DTH22 Reading temperature or humidity takes about 250 milliseconds!
  // DTH22 Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT");
  } 
  
  else {
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.println(dht.toFahrenheit(temperature), 1);
  }
}


void ReceiveSerial(){
  while (Serial.available()){
    delay(10);
    char RxChar = (char)Serial.read(); 
    switch(RxChar){
      case '1':
        LOOPTIME=1000;
        break;
      case '2':
        LOOPTIME=500;
        break;
      case '5':
        LOOPTIME=200;
        break;
      case 'R':
          counter = 0;
          break;
      case 'V':
          OutFormatNTC = 0;
          OutFormatAcc = 0;
          OutFormatPressure = 0;
          OutFormatTmp = 0;
          break;
      case 'S':
          OutFormatNTC = 1;
          OutFormatAcc = 1;
          OutFormatPressure = 1;
          OutFormatTmp = 1;
          break;
      }
  }
}

  
void loop() {
  long int loop_start = millis(), loop_end;
  printdata();
  ReceiveSerial();  
  counter++;
  loop_end = millis();
  if (LOOPTIME>(loop_end-loop_start)){      //Sets the delay to aquire right loop time
    delay(LOOPTIME-(millis()-loop_start));  
  }
}

