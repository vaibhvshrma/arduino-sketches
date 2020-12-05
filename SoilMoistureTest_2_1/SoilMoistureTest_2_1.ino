//Soil Moisture Sensor Test 2

/*We use a digital pin to power the sensor as to turn it off when not required
  so that sensor does not corrode due to current and has a long life
*/
#include <Streaming.h>    //Importing library for streaming output

const int sensorPin = A0;   //Declaring sensor pin to take input
const int sensorPin2 = A1;   //Declaring sensor pin to take input
const int sensorPowerPin = 8;   //Sensor power from digital pin 8
const int sensor2PowerPin = 12;   //Sensor power from digital pin 7
int readVal;        //Variable for storing output of sensor
int readVal2;

void setup()
{
  pinMode(sensorPowerPin, OUTPUT);  //Declaring as output
  pinMode(sensorPin, INPUT);    //Setting as input
  pinMode(sensor2PowerPin, OUTPUT);  //Declaring as output
  pinMode(sensorPin2, INPUT);    //Setting as input
  digitalWrite(sensorPowerPin, LOW);    //Switching off sensor
  digitalWrite(sensor2PowerPin, LOW);    //Switching off sensor2
  Serial.begin(9600);   //Opening Serial Port
  Serial << "Sensor 1     Sensor 2\n";
}

void loop()
{
  digitalWrite(sensorPowerPin, HIGH);    //Switching on the sensors
  digitalWrite(sensor2PowerPin, HIGH);
  delay(50);                             //Wait for sensor to read efficiently
  readVal = analogRead(sensorPin);    //Reading voltage output of sensor
  //delay(50);
  readVal2 = analogRead(sensorPin2);    //Reading voltage output of sensor2
  Serial << readVal << "            " << readVal2 <<  "\n";    //Output to Serial Monitor
  digitalWrite(sensorPowerPin, LOW);    //Switching off the sensor
  digitalWrite(sensor2PowerPin, LOW);
  delay(5000);   //Reading 1 value in 5 seconds
}
