//Using the LCD to display temperature from LM35

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int tempPin = A0;   //Connection pin to temperature sensor
float tempC;               //Variable to store temperature in Celsius

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  lcd.begin(16,2);
  lcd.clear();        //Clears entire display sets cursor to 0,0
  lcd.print("Hello, Vaibhav!");
  pinMode(tempPin, INPUT);
  analogReference(INTERNAL);    //Sets aRef to 1.1V for higher resolution on LM35
}

void loop()
{
  tempC = analogRead(tempPin)/9.31;   //Converts voltage reading to temperature
  //clrLn(lcd, 1);    //Clear second line on lcd and set cursor to 0,1
  lcd.setCursor(0,1);
  lcd.print(" Temp: ");
  lcd.print(tempC);
  lcd.print("\337C");   //Prints °C
  delay(500);           //Pause for half a second
}

void clrLn(LiquidCrystal_I2C mylcd, int ln)   //Function which clears specified line on specified 16x2 lcd
{
  mylcd.setCursor(0, ln);
  mylcd.print("                ");    //Printing 16 spaces to clear line
  mylcd.setCursor(0, ln);
}

