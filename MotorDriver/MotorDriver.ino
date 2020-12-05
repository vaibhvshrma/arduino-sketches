//Motor Driver using H bridge

//We declare variables to change the direction of motor with H bridge
const int mPin = 9;   //First motor controller
const int mPin2 = 8;   //Second motor controller for H bridge 
char input;             //Declaring character to store input

void setup() 
{
  pinMode(mPin, OUTPUT);    //Declaring them as output
  pinMode(mPin2, OUTPUT);
  Serial.begin(9600);       //Opening Serial Monitor
}

void loop() 
{
  if(Serial.available())    //Checking for input
  {
    input = Serial.read();    //Assigning input from Serial Monitor to input
    if(input == '1')          //If input is 1
    {
      digitalWrite(mPin, HIGH);
      digitalWrite(mPin2, LOW);     //Setting up motor to run in one direction
    }
    else if(input == '2')         //If input is 2
    {
      digitalWrite(mPin, LOW);
      digitalWrite(mPin2, HIGH);    //Setting motor to run in reverse direction
    }
    else
    {
      digitalWrite(mPin, LOW);
      digitalWrite(mPin2, LOW);     //Switching off motor
    }
  }
}
