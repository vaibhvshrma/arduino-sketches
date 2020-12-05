//Text on LCD using I2C

#include <LiquidCrystal_I2C.h>  //Importing relevant libraries

LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);    //Creating object lcd

void setup()
{
  lcd.begin(16,2);    //Initialising object
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("    Welcome!");
  Serial.begin(9600);
}

void loop()
{
  if(Serial.available())
  {
    lcd.clear();
    lcd.print(Serial.readString());
  }
}


