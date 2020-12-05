//Using the LCD to display temperature from LM35 && Water Levels

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Streaming.h>

const int tempPin = A2;   //Connection pin to temperature sensor
float tempC;               //Variable to store temperature in Celsius
const int sensorPin = A0;   //Declaring sensor pin to take input
const int sensorPin2 = A1;   //Declaring sensor pin to take input
const int sensorPowerPin = 8;   //Sensor power from digital pin 8
const int sensor2PowerPin = 3;   //Sensor power from digital pin 7
int readVal;        //Variable for storing output of sensor
int readVal2;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  lcd.begin(16,2);
  lcd.clear();        //Clears entire display sets cursor to 0,0
  lcd.print("Welcome!");    //Greeting message on lcd
  pinMode(tempPin, INPUT);
  pinMode(sensorPowerPin, OUTPUT);  //Declaring as output
  pinMode(sensorPin, INPUT);    //Setting as input
  pinMode(sensor2PowerPin, OUTPUT);  //Declaring as output
  pinMode(sensorPin2, INPUT);    //Setting as input
  digitalWrite(sensorPowerPin, LOW);    //Switching off sensor
  digitalWrite(sensor2PowerPin, LOW);    //Switching off sensor2
  Serial.begin(9600);   //Opening Serial Port
  delay(1000);    //Pausing for 1 second 
  lcd.clear();    //Clearing screen 
 // analogReference(INTERNAL);    //Sets aRef to 1.1V for higher resolution on LM35
}

void loop()
{
  tempC = analogRead(tempPin) * 500.0 / 1024.0;   //Converts voltage reading to temperature
  digitalWrite(sensorPowerPin, HIGH);    //Switching on the sensors
  digitalWrite(sensor2PowerPin, HIGH);
  delay(100);                             //Wait for sensor to read efficiently
  readVal = analogRead(sensorPin);    //Reading voltage output of sensor
  readVal2 = analogRead(sensorPin2);    //Reading voltage output of sensor2
  readVal = map(readVal, 0, 1023, 100, 0);    //Scaling value to 0-100
  readVal2 = map(readVal2, 0, 1023, 100, 0); 
  digitalWrite(sensorPowerPin, LOW);    //Switching off the sensor
  digitalWrite(sensor2PowerPin, LOW); 
  //clrLn(lcd, 1);    //Clear second line on lcd and set cursor to 0,1
  lcd.setCursor(0,0);
  lcd.print(" Temp: ");
  lcd.print(tempC);
  lcd.print("\337C");   //Prints °C
  lcd.setCursor(0,1);   //Setting cursor to beginning of second line
  lcd << "S1 : " << readVal << " S2: " << readVal2;
  delay(1000);   //Reading 1 value in 5 seconds
}

void clrLn(LiquidCrystal_I2C mylcd, int ln)   //Function which clears specified line on specified 16x2 lcd
{
  mylcd.setCursor(0, ln);
  mylcd.print("                ");    //Printing 16 spaces to clear line
  mylcd.setCursor(0, ln);
}

