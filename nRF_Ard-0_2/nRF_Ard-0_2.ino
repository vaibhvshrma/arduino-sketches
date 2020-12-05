#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>
#include <avr/sleep.h>


#define MAX_RETRIES 20      //Max retries if communication fails
#define MINS_UNTIL_ERR 3    //Max minutes to retry to re-establish connection
#define SLEEP_DURATION 2    //Time to sleep for in Mins
#define NUM_PIPES 2      //Number of pipes
#define LED 8             //Led connected to D8

#define ALL_PIPES 100
#define CUR_WATERING 101
#define DISCON_PIPES 102

const int pinCE = 6, pinCSN = 7;
RF24 uno(pinCE, pinCSN);
byte toSend = 1;
const uint64_t pAddress[NUM_PIPES] = {0xB00B1E5000LL, 0xABCDEF12345LL };  

int moist[NUM_PIPES] = {0};
int moistThreshold = 400;       //Moisture for soil on a scale of 0-1023
bool disconnectedPipes[NUM_PIPES] = {0};       //1 for pipes not connected; stores 1 for those which have error in connection

/*1 for pipes which have to be watered
@Note Handles storing state of watering as well as used to know which valves to open and close
@Note Pipes in need of watering will have 1 stored and then valve open/close function can be called to correspond action
@Note Before open/close of valve, this must be updated
*/
bool toBeWatered[NUM_PIPES] = {0};          

//Stores state of valves where 1 corresponds to on and 0 to off
bool valves[NUM_PIPES] = {0}; 

int counter;
int loopCount;
bool txFail = false;        //Variables to determine if an error in operation is there
// bool error = false;
// bool retry = false;
// bool awakeForCheck = false;   //Indicates that MCU has awoken to reattempt to establish a connection
// byte txFailCount = 0;


//Prototyping the functions

//Gets moisture from corresponding pipe numbers
void getMoisture(byte pipeNum);
//Sets WDT to trigger interrupt after 8 secs      
void setupWDT();
//Puts the MCU to sleep from specified minutes
void sleepForMins(int mins);
//Blinks the LED at D8
void flash();
//Called on error generation
void errorGen(byte pipeNum);
//For checking if watering is required in any zone
void needsWatering(byte whichPipes);
//Call to get moisture on all pipes
void getMoistureFrom(byte whichPipes);
//to manipulate the valves
void valvesToggle(byte whichPipes);
//Determines if watering required in any zone by checking toBeWatered[]
bool wateringRequired();
//Determines if all pipes are connected by checking disconnectedPipes[]
bool allConnected();

//Prototyping finished

void setup()
{ 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    //Set sleep mode here 
  Serial.begin(9600);

  //Sets up nRF Radio
  uno.begin();      
  uno.setAutoAck(1);
  uno.enableAckPayload();
  //uno.openWritingPipe(PIPEADDR1);
  uno.stopListening();
  
  //Take a moisture reading from all pipes on initialising
  for(int j = 0; j < NUM_PIPES; j++)
  {  
    getMoisture(j);            //Writes moisture reading obtained to moist[]
    delay(75);
  }

}

void loop()
{
  if(!error)    //error occurs when connection cannot be established even after MINS_UNTIL_ERR
  {
    //digitalWrite(LED, LOW);
    flash();    //Led turned on due to error turns off in flash
    sleepForMins(SLEEP_DURATION);
  }
  else    //Error in transmission even after trying for MINS_UNTIL_ERR
  {
    pinMode(LED, OUTPUT);              //Turn on red light to indicate error
    digitalWrite(LED, HIGH);
    if(retry)
    { 
      Serial.println("Error! Going to sleep");
      delay(75);
    }
    sleepForMins(SLEEP_DURATION);
  }

  
}
/*
Puts the MCU to sleep for specified minutes
@Param Sets sleeping duration in minutes
Uses the setupWDT() 
*/
void sleepForMins(int mins)
{   
  // if(txFailCount < MINS_UNTIL_ERR)
  // {
  //   if(txFail  && !retry)   //Retry false allows it to go to sleep for SLEEP_DURATION
  //   {
  //     loopCount = 60/8;    //Check for connection every 1 min
  //     txFailCount++;
  //   }   
  //   else 
  //   {
      loopCount = mins * 60 / 8;  //Calculates how many times loop must iterate
    //   if(retry)
    //   {
    //     retry = false;    //Allows recheck every MINS_UNTIL_ERR on waking up from error  
    //   }
    // }
    
    for(counter = 0; counter < loopCount; counter++)
    {
      setupWDT();      //Setting up wdt
      wdt_reset();  // pat the dog
      
      //noInterrupts ();           // timed sequence follows required for BOD
      sleep_enable();   
      /*
      // turn off brown-out enable in software
      MCUCR = bit (BODS) | bit (BODSE);
      MCUCR = bit (BODS);
      */ 
      interrupts ();             // guarantees next instruction executed; read data sheet; nick gammon interrupts
      sleep_cpu ();  
      
      //Code continues from here after waking up in every 8 seconds
      // cancel sleep as a precaution
      sleep_disable();
      /*if(button)    //If awake due to button
        break;*/
    }
    //When MCU has slept for required time and ISR has been called hence wdt_disable() has been called

/*  }
  else    //No connection can be established after retrying for MINS_UNTIL_ERR
  {
    error = true;
    txFailCount = 0;
    retry = true;       //Have to retry
  }
}*/

// watchdog interrupt
ISR (WDT_vect) 
{
  wdt_disable();  // disable watchdog
}  // end of WDT_vect

inline void setupWDT()
{
    //Setting up wdt timer
    // allow changes, disable reset; WDTCSR is the register for WDT
    WDTCSR = bit (WDCE) | bit (WDE);
    // set interrupt mode and an interval 
    WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
}


/*
Checks disconnectedPipes[] and returns true if all values are 0 i.e. none are disconnected
else returns false
@Note Should be called after getMoistureFrom(ALL_PIPES) & getMoistureFrom(DISCON_PIPES) which updates disconnectedPipes[]
*/
bool allConnected()
{
  for(int j = 0; j < NUM_PIPES; j++)
  {
    if(disconnectedPipes[j])
      return false;
  }
  return true;
}


/*
Checks toBeWatered[] and return true whether watering required and false if not
Watering is required if any value in the array is set
@Note Should be called after updating moist[] by getMoistureFrom()
@Note checks only connected pipes  
*/
bool wateringRequired()
{
  for(int j = 0; j < NUM_PIPES; j++)
  {
    if(!disconnectedPipes[j])   //Pipe is connected
    {
      if(toBeWatered[j])
        return true;
    }
  }
  return false;
}


/*Accesses global variable moistThreshold
@Note Must be called after getMoistureFrom(byte)
@Param which pipes should be checked
@Note Sets 1 in toBeWatered for those requiring water
*/
void needsWatering(byte whichPipes)    
{
  switch(whichPipes)
  {
    case ALL_PIPES:   //Check all pipes
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j])      //Check only pipes which are connected
        {
          if(moist[j] <= moistThreshold)
          {
            toBeWatered[j] = true;      //Set that corresponding pipe requires water
          }
          else
          {
            toBeWatered[j] = false;
          }
        }
      }
    }
    case CUR_WATERING:    //Check only pipes being watered currently 
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j] && toBeWatered[j])   //Check only connected pipes which are being watered
        {
          if(moist[j] > moistThreshold)
          {
            toBeWatered[j] = false;         //We already know that it is being watered so don't have to set 1 in toBeWatered
          }
        }  
      }
    }
    default:    //Check the pipeNumber given in arguments
    {
      if(whichPipes < NUM_PIPES && whichPipes >= 0)
      {  
        if(!disconnectedPipes[whichPipes])    //Given pipe is connected
        {
          if(moist[whichPipes] < moistThreshold)
            toBeWatered[whichPipes] = true;
          else
            toBeWatered[whichPipes] = false;
        }
      }
      else
        Serial.println("No such pipe!");
    }
  }
}


/*
Gets moisture reading from one/multiple pipe
@Note Uses getMoisture() which writes moisture date to moist[]
@Note Checks pipes which have not yet been marked as disconnected in 
      disconnectedPipes[] when called with anything except DISCON_PIPES 
@Note getMoisture() marks pipes as disconnected/connected
*/
void getMoistureFrom(byte whichPipes)
{
  switch(whichPipes)
  {
    case ALL_PIPES:   //Check all pipes
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j])      //Check only pipes which are connected
        {
          getMoisture(j);         //Get moisture reading from corresponding pipe
        }
      }
    }
    break;

    case CUR_WATERING:    //Check only pipes being watered currently 
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j] && toBeWatered[j])   //Check only connected pipes which are being watered
        {
          getMoisture(j);       //Get moisture reading from corresponding pipes 
        }  
      }
    }
    break;

    case DISCON_PIPES:    //Checks and attempts reconnection with disconnected pipes
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(disconnectedPipes[j])
        {
          getMoisture(j);
        }
      }
    }
    break;

    default:    //Check the pipeNumber given in arguments
    {
      if(whichPipes < NUM_PIPES && whichPipes >= 0)
      {  
        if(!disconnectedPipes[whichPipes])    //Given pipe is connected
        {
          getMoisture(j);               
        }
      }
      else
        Serial.println("No such pipe!");
    }
  }
}


/*
Gets moisture reading from specified pipe number
@Note If communication fails then corresponding pipe index is set as 1 in disconnectedPipes[]
      txFail is set as true which can be set false if all pipes are connected again
@Note pipe is set as disconnected only if failure of connection reattempts exceeds MAX_RETRIES  
@Note if txFail is true disconnectedPipes[] are later checked to reattempt connection
@Note Does not check whether pipe is disconnected hence can check all pipes
@Note When called with getMoistureFrom(DISCON_PIPES) sets pipe as connected when connection is re-established
*/
void getMoisture(byte pipeNum)  
{
  int i = 0, tempMoist = 0;
  Begin:
  if(i > MAX_RETRIES)
  {
    Serial.print("Communication failed on pipe: ");
    Serial.println(pipeNum);
    txFail = true;
    disconnectedPipes[pipeNum] = true;
    goto End;
  }
  uno.openWritingPipe(pAddress[pipeNum]);
  if(uno.write(&toSend, sizeof(byte)))  //Accurate reading is received on second turn
  {
    delay(50);  //Allows sufficient time for wake,ackPayload
    if(uno.available())
      uno.read(&tempMoist, sizeof(tempMoist));  //This is old reading
    
    if(uno.write(&toSend, sizeof(byte)))
    //Payload received here is of actual importance
    {
      Serial.println("Transmit Successful!");
      toSend++;
      //txFail = false;    //Acknowledge that communication re-established
      //txFailCount = 0;
    }
    else
    {
      //Serial.println("Transmit failed\nRetrying...");
      i++;
      goto Begin;
    }
  }
  else
  {
    //Serial.println("Transmit failed\nRetrying...");
    i++;
    goto Begin;
  }
    
  //Take moisture reading arriving as ack payload
  if(uno.available())
  {
    uno.read(&tempMoist, sizeof(tempMoist));  //read ackPayload

    //Inverse the values of reading so that it corresponds to moisture & assign moisture reading to corresponding pipe
    moist[pipeNum] = map(tempMoist, 0, 1023, 1023, 0);    
    Serial.print("Moisture reading received is: ");
    Serial.println(moist[pipeNum]);

    if(disconnectedPipes[pipeNum])    //If the pipe was disconnected, set as connected
    {
      disconnectedPipes[pipeNum] = false;
      Serial.print("Connection re-established with pipe no :");
      Serial.println(pipeNum);
    }
  }
  else
  {
    Serial.println("No ack payload available");
  }
  End:  //This is the point which will reach if communication is not successful for more than max retries
  i = 0;
}    //getMoisture ends


void flash ()    //To blink led
{
  pinMode (LED, OUTPUT);
  for (byte i = 0; i < 1; i++)
    {
      digitalWrite (LED, HIGH);
      delay (50);
      digitalWrite (LED, LOW);
      delay (50);
    }
    
  pinMode (LED, INPUT);
  
}  // end of flash