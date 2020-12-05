//Soil Moisture Sensor Test
#include <Streaming.h>    //Importing library for streaming output

const int sensorPin = A0;   //Declaring sensor pin
int readVal;        //Variable for storing output of sensor

void setup()
{
  pinMode(sensorPin, INPUT);    //Setting as input
  Serial.begin(9600);   //Opening Serial Port 
}

void loop() 
{
  readVal = analogRead(sensorPin);    //Reading voltage output of sensor
  Serial << "Read voltage is " << readVal << "\n";    //Output to Serial Monitor
  delay(250);   //Reading 4 values per second  
}
