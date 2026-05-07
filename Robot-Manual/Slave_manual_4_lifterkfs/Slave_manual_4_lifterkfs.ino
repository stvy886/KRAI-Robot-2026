#include <Wire.h>
#include <ModbusRtu.h>

#define AS5600 0x36
#define rpwm 6
#define lpwm 5
#define TXEN 4

Modbus slave(4, Serial, TXEN);
modbus_t telegram;
int16_t data[1] = {0};

// Variable yang digunakan
float set_angle;
int lastkuadran = 0;
int rotation = 0;
float full_angle = 0.0;
bool first_condition = true;
float final_angle = 0;
int com = 0;
int max_val = 60;
int min_val = 58;


void setup() {
  // put your setup code here, to run once:
  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);

  analogWrite(rpwm, 0);
  analogWrite(lpwm, 0);

  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);

  slave.start();

  set_angle = setAngle(Output(AS5600));

}

void loop() {
  // put your main code here, to run repeatedly:
  slave.poll();
  com = (int16_t)data[0];

  final_angle = ASread();
  Serial.print("Angle: ");
  Serial.println(final_angle);

  lifter(0, (min_val + 300), (max_val + 300));
  lifter(1, (min_val + 120), (max_val + 120));
  lifter(2, min_val, max_val);
}
  
void lifter(int id, float max, float min){
  if (com == id){
    if (final_angle < min){
      analogWrite(rpwm, 50);
      analogWrite(lpwm, 0);
    } else if (final_angle > max){
      analogWrite(rpwm, 0);
      analogWrite(lpwm, 50);
    } else {
      analogWrite(rpwm, 0);
      analogWrite(lpwm, 0);
    }
  }    
}

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
