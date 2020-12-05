//Using the LCD

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  lcd.begin(16,2);  //Initialises lcd as 16x2
  lcd.clear();  //Clears entire display
  lcd.home();   //Sets cursor to top left corner
  lcd.print(" Hello, Vaibhav!");
}

void loop()
{
  
}

