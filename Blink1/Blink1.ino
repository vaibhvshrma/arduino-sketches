//Sketch for blinking

const int ledPin = 9;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);    //Enable output on the led pin  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH); //Turns led on
  delay(1000);                 //Waits for 200ms
  digitalWrite(ledPin, LOW);  //Turns led off
  delay(1000);
}
