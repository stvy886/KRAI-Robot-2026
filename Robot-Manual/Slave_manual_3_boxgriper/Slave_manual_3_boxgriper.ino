#include <ModbusRtu.h>
#include <Servo.h>

#define addr 3
#define TXEN 4

Modbus slave(addr, Serial, TXEN);

uint16_t au16data[2] = {0, 0};

Servo srv1;
Servo srv2;

const int pwmMotor1 = 5;
const int pwmMotor2 = 6;  

const int pinMotor1 = 2;
const int pinMotor2 = 3;

int data = 0;
int motorData = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  slave.start();

  srv1.attach(10);
  srv2.attach(9);

  srv1.write(180);
  srv2.write(0);

  pinMode(pinMotor1, OUTPUT);
  pinMode(pinMotor2, OUTPUT);
  pinMode(pwmMotor1, OUTPUT);
  pinMode(pwmMotor2, OUTPUT);

  digitalWrite(pinMotor1, LOW);
  digitalWrite(pinMotor2, LOW);
  analogWrite(pwmMotor1, 0);
  analogWrite(pwmMotor2, 0);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 2);

  data = (int16_t)au16data[0];
  motorData = (int16_t)au16data[1];

  Serial.print("data: ");
  Serial.println(au16data[0]);
  Serial.print("motor: ");
  Serial.println(au16data[1]);
  Serial.print("lifter: ");
  Serial.println(au16data[3]);

  // GRIPPER
  if (data == 1){
    srv1.write(50);
    srv2.write(90);
  }
  else {
    srv1.write(180);
    srv2.write(0); 
  }
  
  //MOTOR
  if (motorData == 2) {
    digitalWrite(pinMotor1, HIGH);
    digitalWrite(pinMotor2, LOW);
    analogWrite(pwmMotor1, 175);
    analogWrite(pwmMotor2, 0);
  }
  else if (motorData == 3) {
    digitalWrite(pinMotor1, LOW);
    digitalWrite(pinMotor2, HIGH);
    analogWrite(pwmMotor1, 0);
    analogWrite(pwmMotor2, 175);
  }
  else {
    digitalWrite(pinMotor1, LOW);
    digitalWrite(pinMotor2, LOW);
    analogWrite(pwmMotor1, 0);
    analogWrite(pwmMotor2, 0);
  }
}