int ledPin = 8;
int switchPin = 5;
bool state = true;

void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);
}

void loop()
{
  if(state)
    digitalWrite(ledPin, HIGH);
  else
    digitalWrite(ledPin, LOW);
  
  if(digitalRead(switchPin))
  {
    while(digitalRead(switchPin));
    state = !state;
  }
  
  
}
