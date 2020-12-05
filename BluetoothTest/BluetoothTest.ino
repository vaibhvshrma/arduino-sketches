#include <SoftwareSerial.h>

SoftwareSerial bt(7, 10);    //RX, TX
char command;       //Variable for imput instructions
const int ledPin = LED_BUILTIN;

void setup()
{
  bt.begin(9600);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop()
{
  if(bt.available())
  {
    command = bt.read();
    if(command == 'n')
      digitalWrite(ledPin, HIGH);
    else
      digitalWrite(ledPin, LOW);
    Serial.println(command);
  }
}
