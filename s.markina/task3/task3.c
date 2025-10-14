int swith =2;
int ld=12;  0`

void setup(){
  Serial.begin(9600);
  pinMode(swith, INPUT_PULLUP);
  pinMode(ld, OUTPUT);
}

void loop(){
  if (digitalRead(swith)==0){
    digitalWrite(ld,1);
    delay(200);
  } else {
    digitalWrite(ld, 0);
    delay(200);
  }
}