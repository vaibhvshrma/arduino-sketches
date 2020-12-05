#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>    //Loading relevant libraries
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int pinCE = 9;    //Tells if SPI communication is command or message
const int pinCSN = 10;  //Sets module to active or standby mode
byte gotByte = 0;      //Used to store payload from transmitter module
bool done = false;     //Termination condition
RF24 nano(pinCE, pinCSN);   //Creates an nRF object
const uint64_t pAddress = 0xB00B1E5000LL;   //Defines pipe address (same as transmitter module)
                                            //LL shows long long type
LiquidCrystal_I2C lcd(63,2,1,0,4,5,6,7,3,POSITIVE); 
                                            
void setup()
{
  //Serial.begin(9600);   //Opens Serial Port
  lcd.begin(16,2);        //Initialising 16x2 display
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome!");
  nano.begin();       //Starts nRF module  
  nano.setAutoAck(1); //Ensure AutoAck is enabled
  nano.enableAckPayload();  //Allows optional payload on Ack packet
  nano.setRetries(5, 15);   //5*250us retry for maximum 15 times
  nano.openReadingPipe(1, pAddress);    //Open pipe for receiving messages
  nano.startListening();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);                     
}

void loop()
{
  if(gotByte >= 9 & !done)    //Once 10 packets are received, send ack packet with payload to signal end
  {
    char cArray[5] = "Done";    //Create array to store done, 5th character for null (terminating) character  
    nano.writeAckPayload(1, cArray, sizeof(cArray));    //Send ack payload
    //Arguments are: pipelinr number, pointer to variable, variable size
  }
  //Now loop until all of payload data is received, here, loop should run only once
  while(nano.available() & !done)
  {
    nano.read(&gotByte, 1);   //Read one byte of data and store in gotByte variable
    lcd.print("Rcvd pkt no: ");
    lcd.print(gotByte);
    delay(500); 
    lcd.clear();
    lcd.setCursor(0,0);

    if(gotByte > 9)
  {
    done = true;
    lcd.print("Done!");
  }
  }

  

  delay(200);
}


