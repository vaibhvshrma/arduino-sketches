const int buzzer = 8;
const int sensor = 0;
const int minD = 300, maxD = 10000;
void setup() {
  // put your setup code here, to run once:
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int rate = analogRead(0);   //Reading from pin 0
  rate = map(rate, 0, 255, minD, maxD);
  
  tone(buzzer, rate);
  delay(1000);
  noTone(buzzer);
  delay(1000);
}
