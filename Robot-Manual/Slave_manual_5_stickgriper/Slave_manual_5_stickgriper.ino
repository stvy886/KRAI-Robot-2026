#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

// Pins
#define pwm1 9
#define pwm2 10
#define en 4 

int pos = 0;

bool lastPos1 = false;
bool lastPos2 = false;

bool servoState1 = false; 
bool servoState2 = false; 

// Data Modbus
int16_t au16data[1] = {0};

// Inisiasi modbus slave
Modbus slave(5, Serial, en);

void updategriper(){
  pos = (int16_t)au16data[0];

  if (pos == 1){
    if (!lastPos1){
      servoState1 = !servoState1;

      if (servoState1){
        srv1.write(120);
      } else {
        srv1.write(0);
      }
    }
    lastPos1 = true;
  } else {
    lastPos1 = false;
  }

  if (pos == 2){
    if (!lastPos2){
      servoState2 = !servoState2;

      if (servoState2){
        srv2.write(0);
      } else {
        srv2.write(90);
      }
    }
    lastPos2 = true;
  } else {
    lastPos2 = false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  srv1.attach(pwm1);
  srv2.attach(pwm2);

  srv1.write(0);
  srv2.write(90);

  pinMode(en, OUTPUT);
  slave.start();

}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 1);

  updateGriper();
}