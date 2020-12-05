const int ledPin = 9; //Pin where led is connected
const int ldrPin = A0;
int readVal, setVal;  //Read from ldr; set to led 
float printVal;

void setup()
{
  pinMode(ledPin, OUTPUT);  //Output mode on
  Serial.begin(9600);     //Serial comm open
}

void loop()
{
  readVal = analogRead(ldrPin);   //Read voltage across ldr
  //Lower light, higher resistance, more voltage drop
  setVal = map(readVal, 0, 1023, 255, 0);
  // Analog read returns values from 0-1023; must be scaled to 0-255
  //We reverse the output value range so that led is brightest when light is dimmest
  printVal = (5.0/255) * setVal;
  Serial.println(printVal);
  analogWrite(ledPin, setVal);
  delay(250);
}

