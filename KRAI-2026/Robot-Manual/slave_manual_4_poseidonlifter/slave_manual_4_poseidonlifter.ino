#include <ModbusRtu.h>
#include <Servo.h>

// Pins
#define srvPwm 9
#define srvPwm2 10
#define rpwm 6
#define lpwm 5
#define TXEN 4
#define ENA 3
#define ENB 2

Servo srv;
Servo srv2;

bool lastPos = false;
bool servoState = false;

int com = 0;
int pos = 0;

float kp = 2.0;
float ki = 0.0;
float kd = 0.2;

float error = 0;
float last_error = 0;
float integral = 0;
float derivative = 0;
float last_pulse = 0;

volatile long pulse = 0;
int count = 0;
int fixState = 0;
int fixVal = -320;

Modbus slave(4, Serial, TXEN);
uint16_t au16data[2] = {0, 0};

void enc(){
  if(digitalRead(ENB)==HIGH){
    pulse++;
  } else{
    pulse--;
  }
}

void PID(float target){

  error = target - pulse;
  integral += error;
  derivative = (error - last_error);

  float output = kp*error + ki*integral + kd*derivative;

  last_error = error;

  output = constrain(output, -150, 50);
  // Serial.print("output: ");
  // Serial.println(output);
  // Serial.print("error: ");
  // Serial.println(error);


  if (!(abs(error) < 2)){
    if (output > 0){
      analogWrite(rpwm, 0);
      analogWrite(lpwm, output);
        
    } 
    else {
      analogWrite(rpwm, abs(output));
      analogWrite(lpwm, 0);
      // Serial.println("TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");
    }
      
  } 
  else {
    integral = 0;
    analogWrite(rpwm, 0);
    analogWrite(lpwm, 0);
  }
}

void lift(){
  if (pulse > -10){
    if (com == 1){
      count = 0;
      fixState = 0;
      analogWrite(rpwm, 100);
      analogWrite(lpwm, 0);
    }
    else if (com == 3){
      fixState = 1;
    } 
    else {
      count++;
      if (count == 1){
        last_pulse = pulse;
      } 
      if(fixState == 0){
        PID(last_pulse);
      } else {
        PID(fixVal);
      }
    }
  } 
  else if (pulse < -420){
    if (com == 2){
      count = 0;
      fixState = 0;
      analogWrite(rpwm, 0);
      analogWrite(lpwm, 75);
    } 
    else if (com == 3){
      fixState = 1;
    } 
    else {
      count++;
      if (count == 1){
        last_pulse = pulse;
      }
      if (fixState == 0){
        PID(last_pulse);
      } else {
        PID(fixVal);
      }
    }
  }
  else {
    if (com == 1){
      count = 0;
      fixState = 0;
      analogWrite(rpwm, 100);
      analogWrite(lpwm, 0);
    }
    else if (com == 2){
      count = 0;
      fixState = 0;
      analogWrite(rpwm, 0);
      analogWrite(lpwm, 75);
    }
    else if (com == 3){
      fixState = 1;
    }
    else {
      count++;
      if (count == 1){
        last_pulse = pulse;
      }
      if (fixState == 0){
        PID(last_pulse);
      } else {
        PID(fixVal);
      }
    }
  }
}

void servo(){
  if (pos == 4){
    if (!lastPos){
      servoState = !servoState;
      if (servoState){
        srv.write(0);
        srv2.write(0);
      } else {
        srv.write(90);
        srv2.write(90);
      }
    }
    lastPos = true;
  } else {
    lastPos = false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  slave.start();

  srv.attach(srvPwm);
  srv2.attach(srvPwm2);

  srv.write(0);
  srv2.write(0);

  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);
  pinMode(ENA, INPUT_PULLUP);
  pinMode(ENB, INPUT_PULLUP);

  analogWrite(rpwm, 0);
  analogWrite(lpwm, 0);

  attachInterrupt(digitalPinToInterrupt(ENA), enc, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 2);
  com = (int16_t)au16data[0];
  pos = (int16_t)au16data[1];

  interrupts(); 
  // Serial.print("pulse: ");
  // Serial.println(com);
  Serial.print("pos: ");
  Serial.println(pos);

  lift();
  servo();
}