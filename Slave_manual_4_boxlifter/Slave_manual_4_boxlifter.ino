#include <ModbusRtu.h>

#define TXEN 4
#define addr 4

Modbus slave(addr, Serial, TXEN);
uint16_t au16data[1] = {0};

#define EN1 5
#define EN2 6

inline void boxLifter(){
  if (data == 1){
    analogWrite(EN1, 50);
    analogWrite(EN2, 0);
  } else if (data == 2) {
    analogWrite(EN1, 0);
    analogWrite(EN2, 50);
  } else {
    analogWrite(EN1, 0);
    analogWrite(EN2, 0);
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  slave.start();

  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  
  analogWrite(EN1, 0);
  analogWrite(EN2, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 1);

  int16_t data = (int16_t)au16data[0];

  boxLifter();
}
