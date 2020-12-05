const int buzzerPin = 9;  //Buzzer connected to pin 9
unsigned int pitch;  //Define pitch for buzzer
int rate = 1000;  //Define delay rate
const int potPin = A0;  //Pot connected to A0
void setup()
{
  //pinMode(buzzerPin, OUTPUT);   //Initialise as output
  Serial.begin(9600);
}

void loop()
{
  int input = analogRead(A0);   //Read pot value
  pitch = map(input, 0, 1023, 2500, 3500);
  Serial.println(pitch);
  tone(buzzerPin, pitch, 3000);   //Emit tone
  delay(50);
  /*
  noTone(buzzerPin);  //Stop emmitting tone
  delay(rate);
  */
}
