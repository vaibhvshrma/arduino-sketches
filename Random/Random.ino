//Random number using different concepts
#include <Streaming.h>

int rNum;

void setup()
{
    Serial.begin(9600);

    for(int i = 0; i < 20; i++)
    {
        rNum = random(10);
        Serial << rNum << " ";
    }

    Serial.println();

    randomSeed(1111);       //Printing twenty numbers with 
    for(int i = 0; i < 20; i++)  // constant randomSeed
    {
        rNum = random(10);
        Serial.print(rNum);
        Serial.print(" ");
    }

    Serial.println();
    randomSeed(analogRead(0));    //Taking seed value from
    for(int i = 0; i < 20; i++)   //unconnected analog port
    {
        rNum = random(10);
        Serial.print(rNum);   //On resetting each time
        Serial.print(" ");
    }                               //a different pattern

    Serial.println();
    
}

void loop()
{
  
}

