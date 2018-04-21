/*version 1.2
 Date: 09-04-2018
 Lift project
*/

boolean debug = 0;  //if true - will enable debug output
boolean piFeedback = 1; //Do NOT change; true - send level info to raspberry pi

//Motor pin defs
const int IN1 = 9; //->B1 -> MB 1 -> (M)Black
const int IN2 = 7; //->B2 -> MB 2 -> (M)Red
const int EN = 10;

const int runtime = 100; //ms; how long the motor should run before measuring the distance

/*
//Buttons to control the lift movement
const int buttonPin1 = 7;  //Go one level Down
const int buttonPin2 = 9;  //Go one level Up
const int buttonPin3 = 10; //Get stuck
*/

//Level defs
const int levels[] = {5, 30, 64, 18}; //level0, level1, level2(T: 18+83), stuck; At level2: check rope break
int currLevel = 0; 
int prev_dist = 0; //will store previous level sensor value
 
//pins for "detect human presence if !power"
const int powerPin = 2; //check whether motor is powered on
const int pirPin = 4; //PIR sensor signal
int ledPin = 13;     // choose the pin for the LED

int pirState = LOW;     // we start, assuming no motion detected
int val = 0;            // variable for reading the pin status -> This will store digitalRead(pirPin)
unsigned long timer;    // keeptrack of time


//HCSR04 pins
const int trigPin1 = A0; //top sensor pins
const int echoPin1 = A1;
const int trigPin2 = A2; //bottom sensor pins
const int echoPin2 = A3;
long duration; //var to store pulseIn() from echo pin 
int distance; //var to store calculated distance value
char activeSensor = 'b'; //'b' or 't' -> bottom level sensor will be measuring values



void setup() {
  Serial.begin(9600);
 /* pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
*/
  pinMode(powerPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);      // declare LED as output
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  if (piFeedback)
  {
    Serial.print("Power :" );
    Serial.println(digitalRead(powerPin));
    
  }
  prev_dist = getDistance();
  
}


void loop() {
 
  //Serial.println(getDistance());
  String command = "";
  if(Serial.available())
  {
    command = Serial.readStringUntil(';'); //commands from rpi
  }

  if (command.indexOf("--") != -1) 
  {
    //run motor down
    runMotor(1, 1000);
    brakeMotor();
  }
  if (command.indexOf("++") != -1) 
  {
    //run motor up
    runMotor(0, 1000);
    brakeMotor();
  }
  
  if(command.indexOf("b1") != -1)
  {
    //Go down
    if(currLevel > 0) --currLevel;
    //setActiveSensor();

    if (debug)
    {
       Serial.println("\nButton1 pressed: Down");
       Serial.print("Current level: ");
       Serial.println(currLevel);
       Serial.print("Active Sensor: ");
       Serial.println(activeSensor);
       Serial.print("Target Distance: ");
       Serial.println(levels[currLevel]);
       Serial.print("Sensor Distances: ");
        
    }
    while(getDistance() >= levels[currLevel]) 
    {
      runMotor(1, runtime);   //run motor CW -> down
    }
    brakeMotor();
   
  } 
  
  if(command.indexOf("b2") != -1)
  {
    //Go up
    if(currLevel < 2) ++currLevel;
    //setActiveSensor();

    if (debug)
    {
       Serial.println("\nButton2 pressed: Up");
       Serial.print("Current level: ");
       Serial.println(currLevel);
       Serial.print("Active Sensor: ");
       Serial.println(activeSensor);
       Serial.print("Target Distance: ");
       Serial.println(levels[currLevel]);
       Serial.print("Sensor Distances: ");
        
    }
    while(getDistance() <= levels[currLevel]) 
    {
      runMotor(0, runtime);   //run motor CCW -> UP
    }
    brakeMotor();
    
  }

  if(command.indexOf("b3") != -1)
  {
    currLevel = 1; //as lift gets stuck between floors 0 and 1
    //setActiveSensor();
    if (debug)
    {
       Serial.println("\nButton3 pressed: Get Stuck");
       Serial.print("Current level: ");
       Serial.println(currLevel);
       Serial.print("Active Sensor: ");
       Serial.println(activeSensor);
       Serial.print("Target Distance: ");
       Serial.println(levels[3]);
       Serial.print("Sensor Distances: ");
        
    }
    
    while(getDistance() <= levels[3]) 
    {
      runMotor(0, runtime);    //run motor CCW -> UP
    }
    while(getDistance() >= levels[3]) 
    {
      runMotor(1, runtime);   //run motor CW -> down
    }
    brakeMotor();

    if(piFeedback)
    {
      delay(5000);
      Serial.println("WARNING: Lift is Stuck!");
    }
    
    
  }

  if(digitalRead(powerPin)==LOW)

  {
    if (piFeedback)
    {
      Serial.print("Power :" );
      Serial.println(digitalRead(powerPin));
    }
    //Detect human presence if power cut off from motor
    if (checkPresence()) Serial.println("Power failure!");
  }
 }



 
int getDistance()
{
  //returns the level value
  int tp, ep, offset = 0;
  int curr_dist;
  setActiveSensor();
  
  if(activeSensor == 't')
  {
    //top sensor value
    tp = trigPin1;
    ep = echoPin1;
    offset = 83;
  }
  if(activeSensor == 'b')
  {
    //bottom sensor value
    tp = trigPin2;
    ep = echoPin2;
  }
  digitalWrite(tp, LOW);
  delayMicroseconds(2);
  digitalWrite(tp, HIGH);
  delayMicroseconds(10);
  digitalWrite(tp, LOW);
  
  duration = pulseIn(ep, HIGH);
  distance = duration*0.034/2;
  curr_dist = abs(offset - distance);
 /* if (debug) 
  {
    Serial.print(prev_dist);
    Serial.print(" : ");
    Serial.print(curr_dist);
    Serial.print(" ");
  }*/
  if (piFeedback && abs(prev_dist - curr_dist) >= 2)
  {
    Serial.print("Level :");
    Serial.print(curr_dist);
    Serial.print('\n');
    prev_dist = curr_dist;
  }
  if (distance == 0)
  {
    Serial.print("Error :");
    Serial.print("Sensor error");
    Serial.print('\n');
  }
  
  return curr_dist;
  
}

void setActiveSensor()
{
  /*
   * Will set activeSensor depending on the level
   * level 0 to 1 -> activeSensor = 'b'
   * level 2 -> activeSensor = 't'
   */
   ///Serial.print("Prev Dist: ");
   //Serial.println(prev_dist);
   if(prev_dist > 50)
   {
      if(debug) Serial.println("Active Sensor: Top");
      activeSensor = 't';
   }
   else
   {
      activeSensor = 'b';
      if(debug) Serial.println("Active Sensor: Bottom");
   }
  
}



void runMotor(int dir, int dur)
{
  if (dir == 0)
  {
    //CCW - UP
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(EN, 255);
  }

  if (dir == 1)
  {
    //CW - DOWN
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(EN, 255);
  }

  delay(dur);
  
  
}

void brakeMotor()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(EN, 255);
    delay(runtime);
    analogWrite(EN, 125);
    delay(runtime);
    analogWrite(EN, 50);
    delay(runtime);
    analogWrite(EN, 25);
}




boolean checkPresence()
{
val = digitalRead(pirPin);  // read input value
  if (val == HIGH) 
  {            // check if the input is HIGH
    digitalWrite(ledPin, HIGH);  // turn LED ON
    if (pirState == LOW) 
    {
      // we have just turned on

      if(debug)
      {
         timer = millis(); //init ms to start time
         Serial.println("Motion detected!");
      }
      if (piFeedback)
      {
        Serial.print("DANGER: Passengers Present" );
      }
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
    return true;
  }
  else 
  {
    digitalWrite(ledPin, LOW); // turn LED OFF
    if (pirState == HIGH)
    {
      // we have just turned of
      if(debug)
      {
        Serial.print("Duration of detection: ");
        Serial.println(millis() - timer);
        Serial.println("Motion ended!");
      }
  
      // We only want to print on the output change, not state
      pirState = LOW;
    }
    return false;
  }
  
}
  
