#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

const int pinSensor = 2;
volatile unsigned long pulses = 0;

// Pins
#define rpwm 5
#define lpwm 6
#define pwm1 9
#define pwm2 10
#define en 4 

int pos = 0;
int max_first = 60;
int min_first = 58;
int max_target = 120;
int min_target = 118;

bool lastPos1 = false;
bool lastPos2 = false;

bool servoState1 = false; 
bool servoState2 = false; 

// Data Modbus
uint16_t au16data[1] = {0};

// Inisiasi modbus slave
Modbus slave(5, Serial, en);

void pulseCounter(){
  pulses++;
}

void slider(int pulse, int max, int min){
  if (pulse > max){
    analogWrite(rpwm, 50);
    analogWrite(lpwm, 0);
  } else if (pulse < min){
    analogWrite(rpwm, 0);
    analogWrite(lpwm, 50);
  } else {
    analogWrite(rpwm, 0);
    analogWrite(lpwm, 0);
  }
}

void updateGriper(){
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
        slider(pulses, max_target, min_target);
        srv2.write(0);
      } else {
        srv2.write(90);
        slider(pulses, max_first, min_first);
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

  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);

  analogWrite(rpwm, 0);
  analogWrite(lpwm, 0);

  pinMode(pinSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSensor), pulseCounter, FALLING);

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

  interrupts();
  updateGriper();

}
