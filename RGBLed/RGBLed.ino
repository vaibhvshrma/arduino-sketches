const int redPin = 11, greenPin = 10, bluePin = 9;    //Setting ledPins to corresponding colors
int r = 255,g = 215,b = 0;    //Value for brightness of led in analogWrite
const int off = 255;  //Full voltage to turn off led
//int rate = 1000;

void setup()
{
  r = abs(255-r);
  b = abs(255-b);     //Working with a common anode rgb led
  g = abs(255-g);
  /*pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);    //Setting as outputs although redundant
  pinMode(bluePin, OUTPUT);   
  */
}

void loop()
{
  analogWrite(redPin, r);
  analogWrite(greenPin, g);           
  analogWrite(bluePin, b);
  /*delay(rate);
  analogWrite(redPin, off);
  analogWrite(greenPin, off);           
  analogWrite(bluePin, off);
  delay(rate);
  */
}
