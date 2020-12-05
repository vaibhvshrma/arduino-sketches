//Blinks led proportional to input
int bl;
const int led = LED_BUILTIN;
void setup()
{
    Serial.begin(9600);
    pinMode(led, OUTPUT);
}

void loop()
{
    if(Serial.available())
    {
        char ch = Serial.read();
        if(ch >= '0' && ch <= '9')
        {
          bl = ch - '0';
          bl *= 100;
          Serial.println("Received!");
        }
    }

    blink();
}

void blink()
{
    digitalWrite(led, HIGH);
    delay(bl);
    digitalWrite(led, LOW);
    delay(bl);
}

