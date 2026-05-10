#include <ModbusRtu.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

const int pinSensor = 2;
volatile long pulses = 0;

// Pins
#define rpwm 5
#define lpwm 6
#define pwm1 9
#define pwm2 10
#define en 4 

int pos = 0;

int start = 0;
int target = 60;
float kp = 1.5;
float ki = 0.0;
float kd = 0.2;

int error = 0;
int last_error = 0;
int derivative = 0;
int integral = 0;

bool motorState = false;

bool lastPos1 = false;
bool lastPos2 = false;

bool servoState1 = false; 
bool servoState2 = false; 

// Data Modbus
uint16_t au16data[1] = {0};

// Inisiasi modbus slave
Modbus slave(5, Serial, en);

void pulseCounter(){
  if (motorState == true){
    pulses++;
  } 
  else {
    pulses--;
  }

}

void slider(int pulse, int target){
  error = target - pulse;
  integral += error;
  integral = constrain(integral, -255, 255);
  derivative = error - last_error;

  float output = kp*error + ki*integral + kd*derivative;
  last_error = error;

  output = constrain(output, -255, 255); 

  if (abs(error) < 2){
    integral = 0;
    analogWrite(rpwm, 0);
    analogWrite(lpwm, 0);
  } else if (output > 0){
    analogWrite(rpwm, output);
    analogWrite(lpwm, 0);
  } else {
    analogWrite(rpwm, 0);
    analogWrite(lpwm, abs(output));
  }
}

void updateGriper(){
  pos = (int16_t)au16data[0];

  if (pos == 1){
    if (!lastPos1){
      servoState1 = !servoState1;

      if (servoState1){
        srv1.write(110);
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
        motorState = true;
        interrupts();
        slider(pulses, target);
        srv2.write(0);
      } else {
        motorState = false;
        interrupts();
        srv2.write(90);
        slider(pulses, start);
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

  updateGriper();
  Serial.print("pulse: ");
  Serial.println(pulses);

}
