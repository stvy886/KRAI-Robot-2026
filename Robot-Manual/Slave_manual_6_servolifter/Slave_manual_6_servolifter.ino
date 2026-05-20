#include <ModbusRtu.h>
#include <Wire.h>
#include <Servo.h>

Servo srv1;
Servo srv2;

// Pin and Address
#define AS5600 0x36
#define ID 6
#define TXEN 4
#define rpwm 5
#define lpwm 6

// Variable
float set_angle;
int lastkuadran = 0;
int rotation = 0;
float full_angle = 0.0;
bool first_condition = true;
float final_angle = 0;

bool servoState = false;
bool lastState = false;

int store = 0;
int servo = 0;

Modbus slave(ID, Serial, TXEN);
uint16_t au16data[2] = {0,0};

float ASread(){
  float tot_angle = Rotation(CorrectAngle(Output(AS5600), set_angle), &lastkuadran, &rotation, &full_angle, &first_condition);
  return tot_angle;
}

float setAngle(float angle){
  float set_angle = angle;
  return set_angle;
}

float Output(int addr){
  Wire.beginTransmission(addr);
  Wire.write(0x0C);
  Wire.endTransmission();
  Wire.requestFrom(addr, 2);

  if (Wire.available() >= 2){

    int Highbyte = Wire.read();
    int Lowbyte = Wire.read();

    uint16_t raw_angle = (((Highbyte << 8) | Lowbyte) & 0xFFF);
    float angle = raw_angle* 360.0/4096.0;
    return angle;
  }
  
}

float CorrectAngle(float angle, float set_angle){
  float corr_angle = angle - set_angle;
  if (corr_angle < 0){
    corr_angle = corr_angle + 360;
  }
  return corr_angle;
}

float Rotation(float corr_angle, int *last_kuadran, int *rotation, float *total_angle, bool *first_condition){
  int kuadran;

  if ((corr_angle >= 0) && (corr_angle <=90)){
    kuadran = 1;
  }
  else if ((corr_angle > 90) && (corr_angle <=180)){
    kuadran = 2;
  }
  else if ((corr_angle >180)&&(corr_angle <= 270)){
    kuadran = 3;
  }
  else {
    kuadran = 4;
  }

  if (*first_condition) {
    *last_kuadran = kuadran;
    *first_condition = false;
  }

  if ((kuadran == 1) && (*last_kuadran == 4)){
    (*rotation)++;
  }
  else if ((kuadran == 4) && (*last_kuadran == 1)){
    (*rotation)--;
  }

  *last_kuadran = kuadran;

  *total_angle = (*rotation)*360 + corr_angle;

  return *total_angle;
}

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
  Wire.begin();
  slave.start();

  set_angle = setAngle(Output(AS5600));

  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);

  analogWrite(rpwm, 0);
  analogWrite(lpwm, 0);

  srv1.attach(9);
  srv2.attach(10);

  srv1.write(70);
  srv2.write(120);
}


void loop() {
  // put your main code here, to run repeatedly:
  slave.poll(au16data, 2);
  store = (int16_t)au16data[0];
  servo = (int16_t)au16data[1];

  final_angle = ASread();
  Serial.print("Angle: ");
  Serial.println(final_angle);

  griper();
}
