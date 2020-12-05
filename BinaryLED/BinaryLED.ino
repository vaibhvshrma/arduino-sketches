int led = 13;
int led2  = 12;
int led3 = 11;
int ground = 9;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(ground, OUTPUT);
  digitalWrite(ground, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  int flow = 13;
  for(int i = 0; i < 8; i++, delay(1000))
  {
    if(i%2==1)
      digitalWrite(led3, HIGH);
     else
      digitalWrite(led3, LOW);

    if(i==2 || i == 3 || i == 6 || i ==7)
      digitalWrite(led2, HIGH);
     else
      digitalWrite(led2, LOW);

    if(i >= 4)
      digitalWrite(led, HIGH);
     else
      digitalWrite(led, LOW);
  }
}
