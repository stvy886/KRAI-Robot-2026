#include <ModbusRtu.h>

// PIN
#define rpw1 3
#define lpw1 5
#define rpw2 6
#define lpw2 11
#define en 4 

// Data Modbus
uint16_t au16data[2] = {0, 0};

// Inisiasi modbus slave 1
Modbus slave(3, Serial, en); 

// Motor depan
void updateMotor(){
  // Get values from Modbus registers and cast to signed 16-bit
  int m = (int16_t)au16data[0];
  int k = (int16_t)au16data[1];

  // Serial.print("m: ");
  // Serial.println(m);
  // Serial.print("k: ");
  // Serial.println(k);

  if (m > 0){
    analogWrite(rpw1, m);
    analogWrite(lpw1, 0);
  }
  else {
    analogWrite(rpw1, 0);
    analogWrite(lpw1, -m);
  }

  if (k > 0){
    analogWrite(rpw2, k);
    analogWrite(lpw2, 0);
  }
  else {
    analogWrite(rpw2, 0);
    analogWrite(lpw2, -k);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(rpw1, OUTPUT);
  pinMode(rpw2, OUTPUT);
  pinMode(lpw1, OUTPUT);
  pinMode(lpw2, OUTPUT);
  pinMode(en, OUTPUT);
  
  slave.start();
}

void loop() {
  // Check for incoming Modbus telegrams from ESP32
  slave.poll(au16data, 2);
  
  updateMotor();
  
}