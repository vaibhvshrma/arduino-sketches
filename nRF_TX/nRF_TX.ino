#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

const int pinCE = 9;      //Tells if SPI communication is command or message
const int pinCSN = 10;    //Setting nRF to standby or active mode
byte counter = 1;       //Counting packets of data sent
bool done = false;      //Used to know when to stop sending packages
RF24 uno(pinCE, pinCSN);    //Create nRF24 object
const uint64_t pAddress = 0xB00B1E5000LL;

void setup()
{
  Serial.begin(9600);   //Open Serial Port
  uno.begin();        //Start the module
  uno.setAutoAck(1);  //Receiver send auto acknowledgement
  uno.enableAckPayload();     //Allow optional ack payload
  uno.setRetries(5,15);       //Sets up retries and timing for packets that were not ack'd
                              //Parameters : smallest time between retries(250us * 5) , max no. of retries
  uno.openWritingPipe(pAddress);      //Unique pipe ID for nRFs communicating with one another; must be same for all
  uno.stopListening();                //Transmitter hence must not listen 
}

void loop()
{
  if(!done)     //If we are not done yet
  {
    Serial.print("Now send packet: ");
    Serial.println(counter);        //Print the packet number being sent
    unsigned long time1 = micros();   //Start timer to measure roumd trip

    //Now we send the data to the receiver module using the following command
    //Arguments are the payload/variable address and its size

    if(!uno.write(&counter, 1))       //If the send fails, let user know through serial monitor
    {
      Serial.println("Packet delivery failed");
    }
    else         //If the send was successful
    {
      unsigned long time2 = micros();   //get new time
      time2 -= time1;     //Calculate round trip time to send and get ack from receiver module
      Serial.print("The round trip took: ");    
      Serial.print(time2);
      Serial.println("ms");
      counter++;    //Increment packet count    
    }

    //If the receiver sends a payload with ack packet then following while loop will get that data

    while(uno.available())    //Payload is received with ack
    {
      char gotChars[5];       //Create an array to hold payload
      uno.read(gotChars, 5);    //Read payload from ack packet

      for(int i = 0; i < 4; i++)
      {
        Serial.print(gotChars[i]);        //Print each character in array
      }
      done = true;      //In this example, ack payload signals end of experiment
    }
  }
  delay(1000);
}
