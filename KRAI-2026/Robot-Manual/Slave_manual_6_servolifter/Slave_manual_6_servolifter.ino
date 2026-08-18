#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

// Pin and Address
#define ID 6
#define TXEN 4

// Variable
bool servoState = false;
bool servoState2 = false;
bool lastState = false;
bool lastState2 = false;

int servo = 0;

Modbus slave(ID, Serial, TXEN);
uint16_t au16data[1] = {0};

void griper(){
  if (servo == 1){
    if (!servoState){
      lastState = !lastState;
      servoState = true;
      if (lastState){
        srv1.write(0);
        srv2.write(90);
      } else{
        srv1.write(90);
        srv2.write(0);
      }
    } 
  } 
  else {
    servoState = false;
  }

  // if (servo == 2){
  //   if (!servoState2){
  //     lastState2 = !lastState2;
  //     servoState2 = true;
  //     if (lastState2){
  //       srv2.write(0);
  //     } else{
  //       srv2.write(90);
  //     }
  //   } 
  // } 
  // else {
  //   servoState2 = false;
  // }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  slave.start();

  srv1.attach(9);
  srv2.attach(10);

  srv1.write(0);
  srv2.write(90);
}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 1);
  servo = (int16_t)au16data[0];

  griper();
}
