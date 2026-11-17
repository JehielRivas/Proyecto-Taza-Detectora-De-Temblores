#include <Wire.h>

const int MPU = 0x68;

int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;

void setup() {

  Wire.begin();

  Serial.begin(9600);

  // Despertar el MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("MPU6050 iniciado");
  Serial.println("----------------------");
}

void loop() {

  // Pedir datos al MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 14, true);

  // Acelerómetro
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  // Temperatura
  Wire.read();
  Wire.read();

  // Giroscopio
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // Mostrar datos
  Serial.print("AcX: ");
  Serial.print(AcX);

  Serial.print(" | AcY: ");
  Serial.print(AcY);

  Serial.print(" | AcZ: ");
  Serial.print(AcZ);

  Serial.print(" | GyX: ");
  Serial.print(GyX);

  Serial.print(" | GyY: ");
  Serial.print(GyY);

  Serial.print(" | GyZ: ");
  Serial.println(GyZ);

  delay(200);
}