const int motorPin = 9;   //PWM as we want variable speeds

void setup()
{
  pinMode(motorPin, OUTPUT);    //Set as output
}

void loop()
{
  for(int i = 0; i <255; i++, delay(10))
  {
    analogWrite(motorPin, i);     //Write increasing voltage to motor
  }

  delay(1000);    //Hold at top speed for a second
  
  for(int i = 255; i >= 0; i--, delay(10))
  {
    analogWrite(motorPin, i);     //Write decreasing voltage to motor
  }

  delay(1000);    //Hold at motor stopped
}

