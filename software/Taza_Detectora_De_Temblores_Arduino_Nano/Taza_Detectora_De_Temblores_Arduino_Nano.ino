#include <Wire.h>
#include <Servo.h>

// Dirección I2C del MPU6050
const int MPU = 0x68;

// Crear los objetos de los servos
Servo servo1;
Servo servo2;

// Pines de los servos
const int PIN_SERVO_1 = 10;
const int PIN_SERVO_2 = 11;

// Variables del MPU6050
int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;

// Ángulos
float pitch = 0;
float roll = 0;

// Tiempo
unsigned long tiempoAnterior;

void setup() {

  // Iniciar comunicación I2C
  Wire.begin();

  // Iniciar comunicación con el monitor serie
  Serial.begin(9600);

  // Despertar el MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Configurar servos
  servo1.attach(PIN_SERVO_1);
  servo2.attach(PIN_SERVO_2);

  // Colocar los servos en el centro
  servo1.write(90);
  servo2.write(90);

  // Guardar el tiempo inicial
  tiempoAnterior = millis();

  delay(1000);

  Serial.println("Sistema iniciado");
}

void loop() {

  // LEER DATOS DEL MPU6050

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

  // CALCULAR ÁNGULOS DEL SENSOR

  float pitchAcelerometro =
    atan2(AcY, sqrt(AcX * AcX + AcZ * AcZ))
    * 180.0 / PI;

  float rollAcelerometro =
    atan2(-AcX, AcZ)
    * 180.0 / PI;

  // CALCULAR TIEMPO

  unsigned long tiempoActual = millis();

  float dt =
    (tiempoActual - tiempoAnterior) / 1000.0;

  tiempoAnterior = tiempoActual;

  // GIROSCOPIO

  float velocidadPitch = GyX / 131.0;
  float velocidadRoll  = GyY / 131.0;

  // FILTRO COMPLEMENTARIO

  pitch =
    0.98 * (pitch + velocidadPitch * dt)
    + 0.02 * pitchAcelerometro;

  roll =
    0.98 * (roll + velocidadRoll * dt)
    + 0.02 * rollAcelerometro;

  // CONVERTIR ÁNGULO A SERVO

  int posicionServo1 =
    map((int)pitch, -30, 30, 120, 60);

  int posicionServo2 =
    map((int)roll, -30, 30, 120, 60);

  // LIMITAR LOS SG90
  
  posicionServo1 =
    constrain(posicionServo1, 60, 120);

  posicionServo2 =
    constrain(posicionServo2, 60, 120);

  // MOVER LOS SERVOS

  servo1.write(posicionServo1);
  servo2.write(posicionServo2);

  // MOSTRAR INFORMACIÓN

  Serial.print("Pitch: ");
  Serial.print(pitch);

  Serial.print(" | Roll: ");
  Serial.print(roll);

  Serial.print(" | Servo 1: ");
  Serial.print(posicionServo1);

  Serial.print(" | Servo 2: ");
  Serial.println(posicionServo2);

  // Actualización aproximada
  
  delay(20);
}
