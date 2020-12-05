#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <avr/sleep.h>

const int pinCE = 4, pinCSN = 5, gnd = 7, gnd2 = 3; 
const int moistSensor = 9, moistPin = A0;  
int moist;
byte gotByte = 0;
RF24 radio(pinCE, pinCSN);
const uint64_t pAddress = 0xB00B1E5012LL;

void wakeUp();
void sleepNow();
void readMoisture();

void setup()
{
  Serial.begin(9600);
  //analogReference(INTERNAL);    //Setting aRef to 1.1V for more consistent readings
  pinMode(gnd, OUTPUT);
  pinMode(gnd2, OUTPUT);
  pinMode(moistSensor, OUTPUT);
  digitalWrite(gnd,LOW);
  digitalWrite(gnd2,LOW);
  pinMode(2, INPUT_PULLUP);  //Pin on which interrupt is attached
  pinMode(8, OUTPUT);  //Attaching led to indicate state
  digitalWrite(8, HIGH);
  
  //Setting up the nRF
  radio.begin();
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.openReadingPipe(1, pAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();
  radio.maskIRQ(1, 1, 0);  
  //Turns on IRQ when data is received 
  //i.e. interrupt is generated on rx_ready
  
}

void loop()
{
  //delay(2000);
  sleepNow();
  /*
  readMoisture();
  Serial.println(moist);
  */
}

void sleepNow()
{
  sleep_enable();  //Enables sleeping on the Arduino
  attachInterrupt(0, wakeUp, LOW);  //Interrupt on pin D2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  //Choose preferred sleep mode
  digitalWrite(8, LOW);          //Status LED switched off
  Serial.println("Going to sleep now!");
  delay(50);
   
  sleep_cpu();    //This is how the processor is put to sleep
  
  //Code continues from here after waking from interrupt
  
   readMoisture();  //Take moisture reading
   radio.writeAckPayload(1, &moist, sizeof(moist));
    //Transmit the moisture date to pipe 1 in form of AckPayload
    //ackpayload is delivered as soon as rx is received i.e. just when
    //node wakes up. Therefore this payload is sent with next rx
    //Therefore the real time moist reading is sent with the second rx
      
  Serial.println("Just woke up!");
  digitalWrite(8, HIGH);
  //Receive data being transmitted after waking up
  if(radio.available())
  {
    radio.read(&gotByte, sizeof(byte));
   delay(50);    //So that second transmission is successful
   //before the interrupt is attached again hence 
   //it prevents another interrupt from being generated
   if(radio.available())
    {
      radio.read(&gotByte, sizeof(byte));
      Serial.print("Received packet number: ");
      Serial.println(gotByte);
    }
  }
}

void wakeUp()    //This is the ISR
{
  sleep_disable();    //Disables sleeping  
  detachInterrupt(0);    //Detach the interrupt at pin D2
}
 
void readMoisture()
{
  digitalWrite(moistSensor, HIGH);  //Turns on moisture sensor
  delay(5);
  moist = analogRead(moistPin);  //Read moisture from analog pin 0
  //Reading is actually the level of dryness in the soil xD
  digitalWrite(moistSensor, LOW);
}
