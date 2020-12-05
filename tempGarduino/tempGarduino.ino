//System will periodically check every twelve hours at 0600 -- 1800 hours

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#define MAX_RETRIES 20      //Max retries if communication fails
#define SLEEP_DUR_LONG 12*60    //Time to sleep for in Mins when no pipes require water
#define SLEEP_DUR_WATERING 20  //Time after which periodic moisture check happens while watering 
#define NUM_PIPES 2      //Number of pipes
#define LED 3             //Led connected to D3
#define CORRECTION_FACTOR 1.05  //To achieve accurate timings

#define ALL_PIPES 100
#define CUR_WATERING 101
#define DISCON_PIPES 102

const int pinCE = 6, pinCSN = 7;
RF24 radio(pinCE, pinCSN);            //Declaring the nRF object
byte toSend = 1;                    //Dummy variable to be sent
//Address of the pipes
const uint64_t pAddress[NUM_PIPES] = {0xB00B1E5000LL, 0xB00B1E5012LL };      

int moist[NUM_PIPES] = {0};
int moistThreshold = 400;       //Minimum moisture for soil on a scale of 0-1023
int moistCutOff = 700;          //Moisture above which watering should be switched off

//Digital Pins the valves corresponding to pipeNum are connected to
byte valvePins[NUM_PIPES] = {8, 9};

//1 for pipes not connected; stores 1 for those which have error in connection
bool disconnectedPipes[NUM_PIPES] = {0};       

/*1 for pipes which have to be watered
@Note Handles storing state of watering as well as used to know which valves to open and close
@Note Pipes in need of watering will have 1 stored and then valve open/close function can be called to correspond action
@Note Before open/close of valve, this must be updated
*/
bool toBeWatered[NUM_PIPES] = {0};          

//Stores state of valves where 1 corresponds to on and 0 to off
bool valves[NUM_PIPES] = {0}; 

int counter;    //Variables used in sleepForMins() to achieve required sleeping time
unsigned long loopCount;
unsigned long loopCountSum = 0;   //Used to add up how much time has MCU slept for
int waterStart = 0;     //Indicates how many minutes it took to water; reset by minsToNextWake()

bool watering = false;  //If currently watering
bool wateredDiscon = false;   //Set in loop; reset every new day by isNewDay()

//Prototyping the functions

//Determines if all pipes are connected by checking disconnectedPipes[]
bool allConnected();
//To check if all valves are closed i.e. all zones have sufficient water
bool allWatered();
//Called on error generation
void errorGen(byte whichPipes = DISCON_PIPES);    //Defaults to DISCON_PIPES
//Blinks the LED at D3
void flash();
//Gets moisture from corresponding pipe numbers
void getMoisture(byte pipeNum);
//Call to get moisture on pipes
void getMoistureFrom(byte whichPipes);
//Check if its evening time i.e. 6 p.m.
bool isEvening();
//Check if its a new day or not by adding the loopCount variable
bool isNewDay();
//Calculate time for which MCU should sleep after watering i.e. to wake at next scheduled time
int minsToNextWake();
//For checking if watering is required in any zone
void needsWatering(byte whichPipes);
//Sets WDT to trigger interrupt after 8 secs      
void setupWDT();
//Puts the MCU to sleep from specified minutes
void sleepForMins(unsigned long mins);
//To switch on/off corresponding valves
void valveControl(byte whichPipes = ALL_PIPES);     //Defaults to ALL_PIPES 
//Determines if watering required in any zone by checking toBeWatered[]
bool wateringRequired();
//Check if we are watering disconnectedPipes
bool wateringDiscon();

//Prototyping finished

void setup()
{ 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    //Set sleep mode here 
  Serial.begin(9600);

  //Sets up nRF Radio
  radio.begin();      
  radio.setAutoAck(1);
  radio.enableAckPayload();
  //radio.openWritingPipe(PIPEADDR1);
  radio.stopListening();

   //Set all in valvePins to be output
  for(int j = 0; j < NUM_PIPES; j++)
  {
    pinMode(valvePins[j], OUTPUT);
  }
  
  //Take a moisture reading from all pipes on initialising; updates moist[] and disconnectedPipes[] 
  getMoistureFrom(ALL_PIPES);

}

void loop()
{
  flash();    //Blink led
  bool goodMorning = isNewDay();    //Checks and stores if its a new day
  bool goodEvening = false;                 //Check for evening is done only if some pipe is disconnected
  //bool disconWater = false;
  Restart:
  if(!allConnected())   //There are disconnectedPipes
  {
    getMoistureFrom(ALL_PIPES);     //Updates moist[] for all pipes marked connected; adds disconnectedPipes if any
    getMoistureFrom(DISCON_PIPES);    //Attempt to re-establish communication with them; update moist[] if success
  
  //Debugging
  delay(50);
  Serial.println("All Not Connected MR All");
  delay(50);
  
    //With above statements in that order first list of disconnected pipes is updated
    //Then a check to re-establish communication with all of them happens
    //Hence moist[] is updated for all possible pipes

    //Generate an error once everyday for those disconnectedPipes[]
    if(goodMorning)
    {
      errorGen(DISCON_PIPES);
    }
    else
      goodEvening = isEvening();

  }
  else if(!allWatered())  //Watering is going on
  {
  //Debugging
  Serial.println("MR Watering");
  delay(50);
    getMoistureFrom(CUR_WATERING);
    if(!allConnected())
      goto Restart;

  }
  else                  //All pipes are connected and none are being watered
  {
    getMoistureFrom(ALL_PIPES);
  Serial.println("MR All");
  delay(50);
    if(!allConnected())
      goto Restart;  
  }
  
  //Check if watering is required; updates toBeWatered[]
  needsWatering(ALL_PIPES);

  //Switch on/off corresponding valves; also switches off for disconnectedPipes
  valveControl();

  if(!allConnected() && goodEvening && !wateredDiscon)  //If evening and disconnected pipes and not watered them this day
  {
    valveControl(DISCON_PIPES);     //Switch on water for discon pipes
    wateredDiscon = true;
  }

  if(allWatered())    //There is no 1 in toBeWatered[]
  {
    if(watering)      //If we were watering
    {
      watering = false;     //Indicate that watering complete
      Serial.println("Remaining time");
      delay(50);
      sleepForMins(minsToNextWake());   //Sleep for required time to wake at next schedule
    }
    else      //There was no watering going on
    {
      Serial.println("Long time");
      delay(50);
      sleepForMins((SLEEP_DUR_LONG)/2);
      delay(1000);
      flash();
      Serial.println("Awake in interim");
      delay(1000);
      sleepForMins((SLEEP_DUR_LONG)/2);         //Breaks loop in two parts
      
    }     
  }
  else    //There is watering going on
  {
    waterStart++;   //Increment so that we know how many times intervals it took to complete watering
    Serial.println("Watering time");
    delay(50);
    sleepForMins(SLEEP_DUR_WATERING);
  }

}


// watchdog interrupt
ISR (WDT_vect) 
{
  wdt_disable();  // disable watchdog
}  // end of WDT_vect


/*
Sets WDT to trigger interrupt after 8 secs
@Note Called by sleepForMins(int)
*/
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
Checks toBeWatered to see if all pipes have been watered or not OR all valves are closed or not
@Note should be called after updating moist[] and toBeWatered[]
*/
bool allWatered()
{
  for(int j = 0; j < NUM_PIPES; j++)
  {
    if(toBeWatered[j])    //Watering required/going on
      return false;
  }
  return true;
}


/*
Generates an error for the disconnectedPipes[]
@Note Defaults to DISCON_PIPES as declared in the prototyping
@Note Should be called once everyday for disconnected pipes
*/
void errorGen(byte whichPipes)
{
  switch(whichPipes)
  {
    case DISCON_PIPES:
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(disconnectedPipes[j])      //Pipe is disconnected
        {
          //Put error generation here
          Serial.print("Error on pipe number: ");
          Serial.println(j);
        }
      }
    }
    break;

    default:
    {
      if(whichPipes < NUM_PIPES && whichPipes >= 0)     //Check if pipe exists
      {
        if(disconnectedPipes[whichPipes])      //Pipe is disconnected
        {
          //Put error generation here
          Serial.print("Error on pipe number: ");
          Serial.println(whichPipes);
        }
      }
      else
        Serial.println("No such pipe!");
    }
  }
}


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
    disconnectedPipes[pipeNum] = true;
    goto End;
  }
  radio.openWritingPipe(pAddress[pipeNum]);
  if(radio.write(&toSend, sizeof(byte)))  //Accurate reading is received on second turn
  {
    delay(50);  //Allows sufficient time for wake,ackPayload
    if(radio.available())
      radio.read(&tempMoist, sizeof(tempMoist));  //This is old reading
    
    if(radio.write(&toSend, sizeof(byte)))
    //Payload received here is of actual importance
    {
      Serial.println("Transmit Successful!");
      toSend++;
    }
    else
    {
      i++;
      goto Begin;
    }
  }
  else
  {
    i++;
    goto Begin;
  }
    
  //Take moisture reading arriving as ack payload
  if(radio.available())
  {
    radio.read(&tempMoist, sizeof(tempMoist));  //read ackPayload

    //Inverse the values of reading so that it corresponds to moisture & assign moisture reading to corresponding pipe
    moist[pipeNum] = map(tempMoist, 0, 1023, 1023, 0);    
    Serial.print("Moisture reading received on pipe ");
    Serial.print(pipeNum);
    Serial.print(" is: ");
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


/*
Gets moisture reading from one/multiple pipe
@Note Uses getMoisture() which writes moisture date to moist[]
@Note Checks pipes which have not yet been marked as disconnected in 
      disconnectedPipes[] when called with anything except DISCON_PIPES 
@Note getMoisture() marks pipes as disconnected/connected
@Note sets disconnectedPipes as false in toBeWatered[]
*/
void getMoistureFrom(byte whichPipes)
{
  switch(whichPipes)
  {
    case ALL_PIPES:   //Check all connected pipes
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j])      //Check only pipes which are connected
        {
          getMoisture(j);         //Get moisture reading from corresponding pipe
          //delay(50);
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
          //delay(50);
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
          //delay(50);
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
          getMoisture(whichPipes);
          //delay(50);               
        }
      }
      else
        Serial.println("No such pipe!");
    }
  }
}


/*
Checks loopCountSum for evening time
@Note Works only if isNewDay() resets the loopCountSum to 0
      Therefore must be called after isNewDay()
@Logic For 12 hours, the loop has to run for 5400# times and it runs for 7 times minimum collectively since 1 min is minimum argument
  #check updated times in sleepForMins
*/
bool isEvening()
{
  if(loopCountSum >= 5131)
    return true;
  else
    return false;
}


/*
Checks loopCountSum to see if its a new day
@Note Puts loopCountSum = 0 if it is a new day
@Note Can be used to determine if disconnectedPipes should be watered or not
@Note 1440 mins in a day hence if loop runs for 10800# times, one day is completed
      At one go, loop runs for a minimum of 1 minute i.e. 7 times
    #Check updated times in sleepForMins
@Note Sets wateredDiscon as false
*/
bool isNewDay()
{
  if(loopCountSum >= 10270)
  {
    wateredDiscon = false;
    loopCountSum = 0;   //Reset the counter
    return true;
  }
  else 
    return false;
}

/*
To be called to calculate time for which MCU should sleep after watering so that it wakes on scheduled 0600/1800 again
@Note Resets the waterStart counter after using it to calculate how much time it should sleep for now 
*/
int minsToNextWake()
{
  int wateringTook = waterStart * SLEEP_DUR_WATERING;   //Time in minutes watering took
  waterStart = 0;         //Reset the counter
  return ((SLEEP_DUR_LONG) - wateringTook);     //Returns remaining time
}


/*Accesses global variable moistThreshold
Checks if watering is required in any zone
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
        if(!disconnectedPipes[j])    //Given pipe is connected
        {
          if(toBeWatered[j])           //It is being currently watered
          {
            if(moist[j] > moistCutOff)   //Has sufficient water
              toBeWatered[j] = false;    
          }
          else                                //If it is not being currently watered
          {
          if(moist[j] < moistThreshold)    //Needs water
            toBeWatered[j] = true;
          else
            toBeWatered[j] = false;
          }
        }
        else        //Pipe is disconnected
        {
          toBeWatered[j] = false;
        }
      }
    }
      break;

    case CUR_WATERING:    //Check only pipes being watered currently 
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j])   //Check only connected pipes which are being watered
        {
          if(toBeWatered[j])
          {
            if(moist[j] > moistCutOff)      //Water exceeds moistCutOff
            {
              toBeWatered[j] = false;         //We already know that it is being watered so don't have to set 1 in toBeWatered
            }
          }
        }
        else        //Pipe is disconnected
        {
          toBeWatered[j] = false;
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
          if(toBeWatered[whichPipes])           //It is being currently watered
          {
            if(moist[whichPipes] > moistCutOff)   //Has sufficient water
              toBeWatered[whichPipes] = false;    
          }
          else                                //If it is not being currently watered
          {
          if(moist[whichPipes] < moistThreshold)    //Needs water
            toBeWatered[whichPipes] = true;
          else
            toBeWatered[whichPipes] = false;
          }
        }
        else        //Pipe is disconnected
        {
          toBeWatered[whichPipes] = false;
        }
      }
      else
        Serial.println("No such pipe!");
    }
  }
}


/*
Puts the MCU to sleep for specified minutes
@Param Sets sleeping duration in minutes
Uses the setupWDT() 
@Note WDT is not accurate and delays by avg of 0.05mins per min of sleep
      Hence we can set a more precise time by solving x + 0.05x = required minsForSleep
    Here x gives such value of mins which should get required sleep
*/
void sleepForMins(unsigned long mins)
{   
  mins /= (CORRECTION_FACTOR);
  loopCount = mins * 60 / 8;  //Calculates how many times loop must iterate
  delay(1000);
  Serial.println(loopCount);
  delay(1000);  
  
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
  
  //Debugging
  Serial.println(counter);
  delay(20);
    
  }
  loopCountSum += loopCount;    //Update loopCountSum
//When MCU has slept for required time and ISR has been called hence wdt_disable() has been called
}

/*
Switches on/off corresponding valves by checking toBeWatered[]
On for 1 in toBeWatered and off for disconnectedPipes or 0 in toBeWatered
@Note Should be called after needsWatering()
@Note sets watering true when switching on a valve
@Note switches on valves for disconnectedPipes when called with param DISCON_PIPES
      Should be switched off by calling valveControl with ALL_PIPES
@Note Defaults to ALL_PIPES
*/
void valveControl(byte whichPipes)
{
  switch(whichPipes)
  {
    case ALL_PIPES:
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(!disconnectedPipes[j])   //Pipe is connected
        {
          if(toBeWatered[j])        //Pipe is set as true in toBeWatered i.e. needs water
          {
             digitalWrite(valvePins[j], HIGH);   //Turn on corresponding valve
             watering = true;
          } 
          else
            digitalWrite(valvePins[j], LOW);    //Turn off corresponding valve
        }
        else
        {
          digitalWrite(valvePins[j], LOW);    //If pipe disconnected, turn off its water
        }
      }
    }
    break;

    case DISCON_PIPES:    //Switch on water for disconnectedPipes[]
    {
      for(int j = 0; j < NUM_PIPES; j++)
      {
        if(disconnectedPipes[j])
        {
          digitalWrite(valvePins[j], HIGH);
          toBeWatered[j] = true;
          watering = true;
        }
      }
    }
    break;
  }
}

/*
Checks if disconnectedPipes are being watered
*/
bool wateringDiscon()
{
  for(int j = 0; j < NUM_PIPES; j++)
  {
    if(disconnectedPipes[j])
    {
      if(toBeWatered[j])
        return true;
    }
    return false;
  }
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

