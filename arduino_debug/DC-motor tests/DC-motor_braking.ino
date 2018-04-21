/*version 1
 Date: 28-03-2018s
 Lift project - DC motor tests
*/
const int IN1 = 6; //->B1
const int IN2 = 5; //->B2 
const int EN = 3;

const int buttonPin1 = 7;
const int buttonPin2 = 9;

const int trigPin = 10;
const int echoPin = 11;
long duration;
int distance;

const int level1 = 14;
const int level2 = 45;


int getDistance()
{
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;
  Serial.println(distance);
  delay(200);
  return distance;
  
}

void runMotor(int dir, int runtime)
{
  if (dir == 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(EN, 255);
  }

  if (dir == 1)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(EN, 255);
  }

  delay(runtime);
  
  
}

void brakeMotor()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(EN, 255);
    delay(200);
    analogWrite(EN, 125);
    delay(300);
    analogWrite(EN, 50);
    delay(500);
    analogWrite(EN, 25);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
 
  //Serial.println(getDistance());
  
  if(digitalRead(buttonPin1))
  {
    //down
    Serial.println("Button1 pressed!");
    while(getDistance() >= level1) 
    {
      runMotor(1, 20);   
    }
    brakeMotor();
   
  } 
  
  if(digitalRead(buttonPin2))
  {
    //down
    Serial.println("Button2 pressed!");
    while(getDistance() <= level2) 
    {
      runMotor(0, 20);   
    }
    brakeMotor();
    
  }
}
