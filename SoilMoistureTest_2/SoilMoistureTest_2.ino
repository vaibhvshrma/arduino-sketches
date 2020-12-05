//Soil Moisture Sensor Test 2

/*We use a digital pin to power the sensor as to turn it off when not required
*so that sensor does not corrode due to current and has a long life
 */
#include <Streaming.h>    //Importing library for streaming output

const int sensorPin = A1;   //Declaring sensor pin to take input from
const int sensorPowerPin = 8;   //Sensor power from digital pin 8
int readVal;        //Variable for storing output of sensor

void setup()
{
  pinMode(sensorPowerPin, OUTPUT);  //Declaring as output
  pinMode(sensorPin, INPUT);    //Setting as input
  digitalWrite(sensorPowerPin, LOW);    //Switching off the sensor
  Serial.begin(9600);   //Opening Serial Port 
}

void loop() 
{
  digitalWrite(sensorPowerPin, HIGH);    //Switching on the sensor
  delay(50);                             //Wait for sensor to read efficiently
  readVal = analogRead(sensorPin);    //Reading voltage output of sensor
  Serial.println(readVal);    //Output to Serial Monitor
  digitalWrite(sensorPowerPin, LOW);    //Switching off the sensor
  delay(5000);   //Reading 1 value in 5 seconds  
}
