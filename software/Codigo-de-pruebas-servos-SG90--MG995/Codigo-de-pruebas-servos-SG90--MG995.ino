#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Servo.h>

// Crear los objetos para el sensor y el servo
Adafruit_MPU6050 mpu;
Servo miServo;

// Definición del pin del servo
const int pinServo = 6;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // Esperar a que se abra el monitor serie

  Serial.println("Inicializando el sistema...");

  // Inicializar el MPU6050
  if (!mpu.begin()) {
    Serial.println("¡Error! No se pudo encontrar el chip MPU6050");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 encontrado con éxito.");

  // Configurar rangos del sensor (opcional, por defecto son adecuados)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Vincular el servo al pin correspondiente
  miServo.attach(pinServo);
  
  // Posición inicial del servo (centro)
  miServo.write(90); 
  delay(1000);
}

void loop() {
  // Crear estructuras para almacenar los eventos del sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* 
     El acelerómetro mide en m/s². 
     Cuando el sensor está plano, el eje X es ~0. 
     Si se inclina 90 grados hacia un lado mide cerca de 9.8, hacia el otro -9.8.
  */
  float valorX = a.acceleration.x;

  // Limitar los valores leídos para evitar errores en el mapeo
  if (valorX > 9.8)  valorX = 9.8;
  if (valorX < -9.8) valorX = -9.8;

  // Mapear el valor de la aceleración (-9.8 a 9.8) al rango del servo (0 a 180)
  // Nota: Si el servo se mueve al revés de lo que deseas, invierte el 0 y el 180.
  int anguloServo = map(valorX * 100, -980, 980, 0, 180);

  // Mover el servo a la posición calculada
  miServo.write(anguloServo);

  // Imprimir datos en el monitor serie para depuración
  Serial.print("Aceleración X: ");
  Serial.print(valorX);
  Serial.print(" m/s² -> Ángulo Servo: ");
  Serial.println(anguloServo);

  // Pequeña pausa para estabilizar las lecturas y el movimiento
  delay(20);
}

