#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>    //Loading relevant libraries

float tempr;
const int pinCE = 9;    //Tells if SPI communication is command or message
const int pinCSN = 10;  //Sets module to active or standby mode
byte gotByte = 0;      //Counting variable
bool done = false;     //Termination condition
RF24 nano(pinCE, pinCSN);   //Creates an nRF object
const uint64_t pAddress = 0xB00B1E5000LL;   //Defines pipe address (same as transmitter module)
                                            //LL shows long long type

                                            
void setup()
{
  Serial.begin(9600);   //Opens Serial Port
  nano.begin();       //Starts nRF module  
  nano.setAutoAck(1); //Ensure AutoAck is enabled
  nano.enableAckPayload();  //Allows optional payload on Ack packet
  nano.setRetries(5, 15);   //5*250us retry for maximum 15 times
  nano.openReadingPipe(1, pAddress);    //Open pipe for receiving messages
  nano.startListening();
}

void loop()
{
  if(done)    //Once 10 packets are received, send ack packet with payload to signal end
  {
    char cArray[5] = "Done";    //Create array to store done, 5th character for null (terminating) character  
    nano.writeAckPayload(1, cArray, sizeof(cArray));    //Send ack payload
    //Arguments are: pipelinr number, pointer to variable, variable size
  }
  //Now loop until all of payload data is received, here, loop should run only once
  while(done == false && nano.available())
  {
    nano.read(&tempr, 4);   //Read four bytes of data and store in tempr variable
    Serial.print("Temperature received is: ");
    Serial.println(tempr);
    gotByte++;
    if(gotByte >= 10)
    {
      done = true;
      Serial.println("Done!");
    }
  }

  

  delay(200);
}


