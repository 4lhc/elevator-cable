/*
 * Code to debug level sensors
 * 
 */
//HCSR04 pins
const int trigPin1 = A0; //top sensor pins
const int echoPin1 = A1;
const int trigPin2 = A2; //bottom sensor pins
const int echoPin2 = A3;
long duration; //var to store pulseIn() from echo pin 
int distance; //var to store calculated distance value


void getDistance(char activeSensor)
{
  //returns the level value
  int tp, ep, offset = 0;
  int curr_dist;
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
  
  Serial.print(activeSensor);
  Serial.print(" :");
  Serial.println(distance);
  
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  getDistance('b');
  getDistance('t');
  delay(1000);

  

}
