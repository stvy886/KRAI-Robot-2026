#include <ModbusRtu.h>
#include <Arduino.h>
#include <Ps3Controller.h>

#define EN_PIN 4

#define MAX_SPEED       250
#define BASE_SPEED      100
#define SPEED_INCREMENT 7
#define ACCEL_COEFF     0.3f

// Variabel global
int realSpeed = BASE_SPEED;
int lX = 0, lY = 0, rX = 0, rY = 0;
int gripState = 0;
int pg36State = 0;
int posisi = 0;
int lift = 0;

int targetM[4] = {0, 0, 0, 0};
float currentM[4] = {0.0f, 0.0f, 0.0f, 0.0f};

// Setup MODBUS
uint16_t au16data[4];
Modbus master(0, Serial, EN_PIN);
modbus_t telegram;

// SLAVE ADDRESS 
#define MOTOR_F 1
#define MOTOR_B 2
#define SLAVE_C 3
#define LIFTER 4

// === FUNCTION DEFINITIONS ===
inline void readInputs();
inline void readInputsr1();
inline void updateMotorSpeeds();
inline void updateMotorSpeedsr1();
inline void R1();
void sendToSlave(uint8_t id, int val1);
void sendToSlave(uint8_t id, int val1, int val2);
void sendToSlave(uint8_t id, int val1, int val2, int val3);

//koneksi ps 3
void onConnect() {
  Serial.println("Connected.");
}
void onDisconnect() {
  Serial.println("Disconnected.");
}

void setup() {
  Serial.begin(115200);

  master.start();
  master.setTimeOut(20);

  // pinMode(EN_PIN, OUTPUT);
  // digitalWrite(EN_PIN, HIGH); // Aktifkan komunikasi (misal master mode)

  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisconnect);
  Ps3.begin("e8:6b:ea:c3:e4:3a");
  // Ps3.begin("e4:65:b8:12:2c:46");
}

void loop() {
  if (Ps3.isConnected()) {
    if (Ps3.data.button.r1){
      Grip();
    } else{
      readInputs();
      updateMotorSpeeds();
    }
  }
  master.poll();
}

// input stick ps
inline void readInputs() {
  lX = map(Ps3.data.analog.stick.lx, -128, 127, -realSpeed, realSpeed);
  lY = map(Ps3.data.analog.stick.ly, -128, 127, realSpeed, -realSpeed);
  rX = -map(Ps3.data.analog.stick.rx, -128, 127, -realSpeed, realSpeed);
  rY = map(Ps3.data.analog.stick.ry, -128, 127, realSpeed, -realSpeed);

  //Update kecepatan
  if (Ps3.data.button.triangle) {
    realSpeed = min(realSpeed + SPEED_INCREMENT, MAX_SPEED);
  } else if (Ps3.data.button.cross) {
    realSpeed = max(realSpeed - 60, BASE_SPEED);
  } 
}

// input stick ps (R1)
inline void readInputsr1() {
  lX = map(Ps3.data.analog.stick.lx, -128, 127, -realSpeed, realSpeed);
  lY = map(Ps3.data.analog.stick.ly, -128, 127, realSpeed, -realSpeed);
  rX = -map(Ps3.data.analog.stick.rx, -128, 127, -realSpeed, realSpeed);
  rY = map(Ps3.data.analog.stick.ry, -128, 127, realSpeed, -realSpeed);
}

// Update kecepatan motor
inline void updateMotorSpeeds() { 
  float vel = min(sqrt((float)(lX * lX + lY * lY)), (float)realSpeed);
  float arah = atan2(lY, lX);
  float putar = rX * 0.4f;

  targetM[0] = vel * sin(arah - 0.785f) + putar;
  targetM[1] = vel * sin(arah + 0.785f) - putar;
  targetM[2] = -vel * sin(arah - 2.356f) + putar;
  targetM[3] = -vel * sin(arah + 2.356f) - putar;

  for (int i = 0; i < 4; i++) {
    currentM[i] += (targetM[i] - currentM[i]) * ACCEL_COEFF;
    currentM[i] = constrain(currentM[i], -realSpeed, realSpeed);
  }

  sendToSlave(MOTOR_F, (int)currentM[0],(int)currentM[1]);
  sendToSlave(MOTOR_B, (int)currentM[2],(int)currentM[3]);

}

// Update kecepatan motor (R1)
inline void updateMotorSpeedsr1() { 
  float vel = min(sqrt((float)(lX * lX + lY * lY)), (float)realSpeed);
  float arah = atan2(lY, lX);
  float putar = rX * 0.4f;

  targetM[0] = vel * sin(arah - 0.785f) + putar;
  targetM[1] = vel * sin(arah + 0.785f) - putar;
  targetM[2] = -vel * sin(arah - 2.356f) + putar;
  targetM[3] = -vel * sin(arah + 2.356f) - putar;

  for (int i = 0; i < 4; i++) {
    currentM[i] += (targetM[i] - currentM[i]) * ACCEL_COEFF;
    currentM[i] = constrain(currentM[i], -realSpeed, realSpeed);
  }

  sendToSlave(MOTOR_F, (int)currentM[0],(int)currentM[1], posisi);
  sendToSlave(MOTOR_B, (int)currentM[2],(int)currentM[3]);

}

// Mode R1
inline void R1(){
  posisi = 0;
  gripState = 0;
  pg36State = 0;
  lift = 0;

  if (Ps3.data.button.triangle){
    posisi = 1;
  } else if (Ps3.data.button.square){
    posisi = 2;
  } else if (Ps3.data.button.cross){
    posisi = 3;
  } 
  if (Ps3.data.button.circle){
    gripState = 1;
  } else if (Ps3.data.button.up){
    pg36State = 2;
  } else if (Ps3.data.button.down){
    pg36State = 3;
  } 
  if (Ps3.data.button.l1){
    lift = 1;
  } else if (Ps3.data.button.l2){
    lift = 2;
  } 

  readInputsr1();
  updateMotorSpeedsr1();
  
  sendToSlave(SLAVE_C, gripState, pg36State);
  sendToSlave(LIFTER, lift);
}



// Serial Communication MODBUS
void sendToSlave(uint8_t id, int val1) {
  au16data[0] = (uint16_t)val1;

  telegram.u8id = id;          // Target ID (1, 2, atau 3)
  telegram.u8fct = 16;         // Function code (Write Multiple Registers)
  telegram.u16RegAdd = 0;      // Start address di slave
  telegram.u16CoilsNo = 1;     // Jumlah data yang dikirim (2 register)
  telegram.au16reg = au16data; // Pointer data

  master.query(telegram);

  master.poll();
}

void sendToSlave(uint8_t id, int val1, int val2) {
  au16data[0] = (uint16_t)val1;
  au16data[1] = (uint16_t)val2;

  telegram.u8id = id;          // Target ID (1, 2, atau 3)
  telegram.u8fct = 16;         // Function code (Write Multiple Registers)
  telegram.u16RegAdd = 0;      // Start address di slave
  telegram.u16CoilsNo = 2;     // Jumlah data yang dikirim (2 register)
  telegram.au16reg = au16data; // Pointer data

  master.query(telegram);

  master.poll();
}
void sendToSlave(uint8_t id, int val1, int val2, int val3) {
  au16data[0] = (uint16_t)val1;
  au16data[1] = (uint16_t)val2;
  au16data[2] = (uint16_t)val3;

  telegram.u8id = id;          // Target ID (1, 2, atau 3)
  telegram.u8fct = 16;         // Function code (Write Multiple Registers)
  telegram.u16RegAdd = 0;      // Start address di slave
  telegram.u16CoilsNo = 3;     // Jumlah data yang dikirim (2 register)
  telegram.au16reg = au16data; // Pointer data

  master.query(telegram);

  master.poll();
}
