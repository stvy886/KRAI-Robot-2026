#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

// PIN
#define rpw1 5
#define lpw1 3
#define rpw2 11
#define lpw2 6
#define pwm1 9
#define pwm2 10
#define en 4 

bool lastPos1 = false;
bool lastPos2 = false;
bool lastPos3 = false;

bool servoState1 = false; 
bool servoState2 = false; 
bool servoState3 = false;

// Data Modbus
int16_t au16data[2] = {0, 0};

// Inisiasi modbus slave 1
Modbus slave(1, Serial, en); 

// Stick griper
void updategriper(){
  int pos = (int16_t)au16data[2];

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

// Motor depan
void updateMotor(){
  // Get values from Modbus registers and cast to signed 16-bit
  int m = (int16_t)au16data[0];
  int k = (int16_t)au16data[1];

  if (m > 0){
    analogWrite(rpw1, 0);
    analogWrite(lpw1, m);
  }
  else {
    analogWrite(rpw1, -m);
    analogWrite(lpw1, 0);
  }

  if (k > 0){
    analogWrite(rpw2, 0);
    analogWrite(lpw2, k);
  }
  else {
    analogWrite(rpw2, -k);
    analogWrite(lpw2, 0);
  }
}

void setup() {
  Serial.begin(115200);
  
  srv1.attach(pwm1);
<<<<<<< HEAD
=======

>>>>>>> ed5497da11388fd8fe0c3e907c860984dae35c6d
  srv2.attach(pwm2);

  srv1.write(0);
  srv2.write(90);

  pinMode(rpw1, OUTPUT);
  pinMode(rpw2, OUTPUT);
  pinMode(lpw1, OUTPUT);
  pinMode(lpw2, OUTPUT);
  pinMode(en, OUTPUT);
  
  slave.start();
}

void loop() {
  // Check for incoming Modbus telegrams from ESP32
  slave.poll(au16data, 3);
  
  updateMotor();
  updategriper();
  
}