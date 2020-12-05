//Using the LCD
const int rsPin = 10;
const int ePin = 9;
const int d4 = 5;
const int d5 = 4;     //Declaring pin connection from lcd to arduino
const int d6 = 3;
const int d7 = 2;
const int tempPin = A0;   //Pin connected to LM35 temp sensor
float temp;           //Variable to hold temperature

#include <LiquidCrystal.h>    //Importing required library
LiquidCrystal lcd(rsPin, ePin, d4, d5, d6, d7);

//void clrLn(LiquidCrystal lcd, int ln);

void setup()
{
   lcd.begin(16,2);   //Defining lcd to be 16x2
   lcd.setCursor(0,0);  //Setting cursor to top-left corner
   lcd.print(" Hello, Vaibhav");
   lcd.setCursor(0,1);  //Set cursor to beginning of second line
   pinMode(tempPin, INPUT);   //Setting tempPin to input
   analogReference(INTERNAL);   //Setting aRef to 1.1V
}

void loop()
{
    temp = analogRead(tempPin)/ 9.31;
   // clrLn(lcd, 1);    //Clear line 2
    lcd.setCursor(0,1);   //Set Cursor to start of second line
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print('\370');
    lcd.print("C");
    delay(1000);
    
}

void clrLn(LiquidCrystal mylcd, int ln)   //Function which clears specified line on specified 16x2 lcd
{
  mylcd.setCursor(0, ln);
  mylcd.print("                ");    //Printing 16 spaces to clear line
  delay(5);
  mylcd.setCursor(0, ln);
}

