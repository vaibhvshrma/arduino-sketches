const int conLed = 8;
const int sensor = 0;
const int minD = 100, maxD = 700;
void setup() {
  // put your setup code here, to run once:
  pinMode(conLed, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int rate = analogRead(0);   //Reading from pin 0
  rate = map(rate, 0, 255, minD, maxD);
  
  digitalWrite(conLed, HIGH);
  delay(rate);
  digitalWrite(conLed, LOW);
  delay(rate);
}
