 bool secure = 0;
 int stat;
void setup() {
 pinMode(2, INPUT);  // Receiver Throttle Signal Input
 pinMode(4, OUTPUT); //  Mosfet Gate Output Pin 
 pinMode (3, OUTPUT); // Red led
 pinMode (1, OUTPUT); // Blue led

 analogWrite(4,0);
 digitalWrite(3,HIGH);
 digitalWrite(1,LOW);
 delay(300);
 digitalWrite(3,LOW);
 digitalWrite(1,HIGH);
 delay(300);
 digitalWrite(3,HIGH);
 digitalWrite(1,LOW);
 delay(300);
 digitalWrite(3,LOW);
 digitalWrite(1,HIGH);
 delay(300);
 digitalWrite(3,LOW);
 digitalWrite(1,LOW);
 delay(300);
  secure = 1;

}

void loop() {

while (secure == 1) {
  digitalWrite(3,HIGH);
 delay(200);
 digitalWrite(3,LOW);
 delay(200);
 stat = pulseIn(2, HIGH);
 if (stat >= 1050){
  secure = 1;
  }
 if (stat <= 1049){
  secure = 0;
    }
 }

 digitalWrite(1,HIGH);

  analogWrite(4, map(pulseIn(2, HIGH), 1000, 2000, 0, 255)); //

delay(15);
}
