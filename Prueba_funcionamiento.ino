#include <Servo.h>

// -------- ESC --------
Servo esc;
int pinESC = 9;

// -------- HC-SR04 --------
int trigPin = 6;
int echoPin = 7;

long duracion;
float distancia;
float distanciaInicial = 0;

// -------- Control --------
// Variar estos valores dependiendo del comportamiento de la planta
int velocidad;
int velocidadBase = 1800; 
int velocidadMax = 2000;
int velocidadMin = 1100;

float alturaObjetivo = 20.0; // cm respecto al punto inicial

// -------- Función de lectura --------
float leerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 30000);
  return (duracion * 0.034) / 2;
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  esc.attach(pinESC);

  // Armar ESC
  esc.writeMicroseconds(1000);
  delay(3000);

  // -------- CALIBRACIÓN --------
  Serial.println("Calibrando altura inicial...");
  delay(2000);

  float suma = 0;
  for (int i = 0; i < 10; i++) {
    suma += leerDistancia();
    delay(100);
  }
  distanciaInicial = suma / 10.0;

  Serial.print("Altura inicial (cero): ");
  Serial.println(distanciaInicial);

  Serial.println("Sistema listo");
}

void loop() {

  distancia = leerDistancia();

  // Altura relativa
  float altura = distancia - distanciaInicial;

  Serial.print("Altura relativa: ");
  Serial.println(altura);

  // -------- CONTROL --------
  velocidad = velocidadBase;

  if (altura < alturaObjetivo - 5) {
    velocidad += 300; // subir
  }
  else if (altura > alturaObjetivo + 5) {
    velocidad -= 300; // bajar
  }

  velocidad = constrain(velocidad, velocidadMin, velocidadMax);

  esc.writeMicroseconds(velocidad);

  delay(100);
}