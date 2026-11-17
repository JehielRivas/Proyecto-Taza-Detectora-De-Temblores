#include <Wire.h>
#include <Servo.h>
#include <math.h>

// MPU6050
const byte MPU = 0x68;

// Pines de los servomotores
const byte PIN_SERVO_SG90 = 6;
const byte PIN_SERVO_MG995 = 11;

// Servomotores
Servo servoSG90;
Servo servoMG995;

// Datos del acelerometro
int16_t accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

// Datos del giroscopio
int16_t gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

// Angulos
float pitch = 0;
float roll = 0;

// Filtro complementario
const float ALPHA = 0.90;
unsigned long tiempoAnterior;
float dt;

// Calibracion del giroscopio
float biasGyroX = 0;
float biasGyroY = 0;
float biasGyroZ = 0;

// Configuracion de los servos
const int CENTRO = 90;
const int LIMITE_MIN = 45;
const int LIMITE_MAX = 135;

// Sentido de correccion
const int SENTIDO_PITCH = -1;
const int SENTIDO_ROLL = -1;

// Ganancias
const float GANANCIA_PITCH = 1.0;
const float GANANCIA_ROLL = 1.05;

// Posiciones actuales
int posSG90 = CENTRO;
int posMG995 = CENTRO;

// Zona muerta
const float ZONA_MUERTA = 1.0;

// Monitor serie
unsigned long ultimoReporte = 0;

// Leer MPU6050
bool leerMPU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(MPU, (uint8_t)14, (uint8_t)true) != 14) {
    return false;
  }

  accelX = (int16_t)((Wire.read() << 8) | Wire.read());
  accelY = (int16_t)((Wire.read() << 8) | Wire.read());
  accelZ = (int16_t)((Wire.read() << 8) | Wire.read());

  Wire.read();
  Wire.read();

  gyroX = (int16_t)((Wire.read() << 8) | Wire.read());
  gyroY = (int16_t)((Wire.read() << 8) | Wire.read());
  gyroZ = (int16_t)((Wire.read() << 8) | Wire.read());

  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;

  rotX = gyroX / 131.0 - biasGyroX;
  rotY = gyroY / 131.0 - biasGyroY;
  rotZ = gyroZ / 131.0 - biasGyroZ;

  return true;
}

// Configurar MPU6050
void setupMPU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);

  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR MPU6050");
    while (true) {
      delay(100);
    }
  }

  Wire.beginTransmission(MPU);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(MPU);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(100);
}

// Calibrar giroscopio
void calibrarMPU() {
  const int MUESTRAS = 200;

  float sumaGyroX = 0;
  float sumaGyroY = 0;
  float sumaGyroZ = 0;

  int validas = 0;

  Serial.println("CALIBRANDO...");
  Serial.println("Mantener el sistema quieto");

  for (int i = 0; i < MUESTRAS; i++) {
    if (leerMPU()) {
      sumaGyroX += gyroX / 131.0;
      sumaGyroY += gyroY / 131.0;
      sumaGyroZ += gyroZ / 131.0;
      validas++;
    }

    delay(5);
  }

  if (validas < 150) {
    Serial.println("ERROR DE CALIBRACION");

    while (true) {
      delay(100);
    }
  }

  biasGyroX = sumaGyroX / validas;
  biasGyroY = sumaGyroY / validas;
  biasGyroZ = sumaGyroZ / validas;

  leerMPU();

  pitch = atan2(gForceX, gForceZ) * 180.0 / PI;
  roll = atan2(gForceY, gForceZ) * 180.0 / PI;

  Serial.println("CALIBRACION TERMINADA");
}

// Inicio del sistema
void setup() {
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);

  setupMPU();

  servoSG90.attach(PIN_SERVO_SG90);
  servoMG995.attach(PIN_SERVO_MG995);

  servoSG90.write(CENTRO);
  servoMG995.write(CENTRO);

  posSG90 = CENTRO;
  posMG995 = CENTRO;

  delay(1000);

  calibrarMPU();

  tiempoAnterior = micros();

  Serial.println("SISTEMA INICIADO");
  Serial.println("Buscando posicion recta...");
}

// Funcionamiento principal
void loop() {
  unsigned long tiempoActual = micros();

  dt = (tiempoActual - tiempoAnterior) / 1000000.0;
  tiempoAnterior = tiempoActual;

  if (dt <= 0 || dt > 0.05) {
    dt = 0.01;
  }

  if (!leerMPU()) {
    return;
  }

  float pitchAccel = atan2(gForceX, gForceZ) * 180.0 / PI;
  float rollAccel = atan2(gForceY, gForceZ) * 180.0 / PI;

  pitch = ALPHA * (pitch + rotY * dt)
        + (1.0 - ALPHA) * pitchAccel;

  roll = ALPHA * (roll + rotX * dt)
       + (1.0 - ALPHA) * rollAccel;

  float pitchControl = pitch;
  float rollControl = roll;

  if (fabs(pitchControl) < ZONA_MUERTA) {
    pitchControl = 0;
  }

  if (fabs(rollControl) < ZONA_MUERTA) {
    rollControl = 0;
  }

  // Aplicar ganancias
  float ajustePitch = pitchControl * GANANCIA_PITCH;
  float ajusteRoll = rollControl * GANANCIA_ROLL;

  // Limitar correcciones
  int correccionPitch = constrain(
    (int)round(ajustePitch), -45, 45
  );

  int correccionRoll = constrain(
    (int)round(ajusteRoll), -45, 45
  );

  // Calcular posiciones de los servos
  int nuevaPosSG90 = CENTRO
                   + SENTIDO_PITCH * correccionPitch;

  int nuevaPosMG995 = CENTRO
                    + SENTIDO_ROLL * correccionRoll;

  nuevaPosSG90 = constrain(
    nuevaPosSG90, LIMITE_MIN, LIMITE_MAX
  );

  nuevaPosMG995 = constrain(
    nuevaPosMG995, LIMITE_MIN, LIMITE_MAX
  );

  // Actualizar SG90
  if (nuevaPosSG90 != posSG90) {
    posSG90 = nuevaPosSG90;
    servoSG90.write(posSG90);
  }

  // Actualizar MG995
  if (nuevaPosMG995 != posMG995) {
    posMG995 = nuevaPosMG995;
    servoMG995.write(posMG995);
  }

  // Mostrar datos por monitor serie
  if (millis() - ultimoReporte >= 100) {
    ultimoReporte = millis();

    Serial.print("Pitch: ");
    Serial.print(pitch);

    Serial.print(" | Roll: ");
    Serial.print(roll);

    Serial.print(" | SG90: ");
    Serial.print(posSG90);

    Serial.print(" | MG995: ");
    Serial.println(posMG995);
  }
}