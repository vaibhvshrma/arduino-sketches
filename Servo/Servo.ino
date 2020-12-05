#include <Servo.h>
const int servoPin = 11;      //Servo attached to Arduino pin 11
Servo srv;                    //Creating Servo object
int angle;                    //Variable to store angle

void setup()
{
  srv.attach(servoPin);   //Setting up Servo
  Serial.begin(9600);     //Opening Serial port
  srv.write(0);
}

void loop()
{
  /*if(Serial.available())    //Check for data input
  {
    angle = Serial.parseInt();   //Reading and assigning value from input
    srv.write(angle);     //Writing angle to servo
    delay(25);            //Waiting for 25ms
  }
  */
  for(angle = 0; angle < 160; angle++)
  {
    srv.write(angle);       //Move in a sweep
    delay(25);
  }
  for(angle; angle > 0; angle--)
  {
    srv.write(angle);       //Move in a sweep
    delay(25);
  }
}
