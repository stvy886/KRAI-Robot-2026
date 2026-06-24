#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

// Pin and Address
#define ID 6
#define TXEN 4

// Variable
bool servoState = false;
bool lastState = false;

int servo = 0;

Modbus slave(ID, Serial, TXEN);
uint16_t au16data[1] = {0};

void griper(){
  if (servo == 1){
    if (!servoState){
      lastState = !lastState;
      servoState = true;
      if (lastState){
        srv1.write(10);
        srv2.write(50);
      } else{
        srv1.write(70);
        srv2.write(120);
      }
    } 
  } else {
    servoState = false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  slave.start();

  srv1.attach(9);
  srv2.attach(10);

  srv1.write(70);
  srv2.write(120);
}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 1);
  servo = (int16_t)au16data[0];

  griper();
}
