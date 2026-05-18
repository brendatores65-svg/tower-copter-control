#include <Servo.h>

Servo esc;

// =========================
// Pines
// =========================
const int pinESC  = 9;
const int trigPin = 6;
const int echoPin = 7;

// =========================
// PWM del sistema (Ajustar)
// =========================

const int PWM_MIN         = 1280;
const int PWM_MAX         = 1350;
const int PWM_EQUILIBRIO  = 1300;


// =========================
// PID (Ajustar)
// =========================
float Kp = 0.532; 
float Ki = 1.528; 
float Kd = 1.299;  

float setpoint = 17.0; // cm

float error = 0;
float errorAnterior = 0;

float integral = 0;

float alturaAnterior = 0;

float salidaPID = 0;

// =========================
// Tiempo
// =========================
unsigned long tiempoAnterior = 0;

// =========================
// Sensor
// =========================
float distanciaInicial = 0;

// =========================
// Filtro mediana
// =========================
#define VENTANA 5

float buffer[VENTANA] = {0};
int bufIdx = 0;

// =========================
// Filtro exponencial
// =========================
#define ALPHA 0.3

float alturaFiltrada = 0;
bool primeraLectura = true;

// ======================================================
// FUNCIÓN CONSTRAINT
// ======================================================
float limitar(float valor, float minimo, float maximo) {
  
  if (valor < minimo) return minimo;
  
  if (valor > maximo) return maximo;
  
  return valor;
}

// ======================================================
// FILTRO DE MEDIANA
// ======================================================
float mediana(float arr[], int n) {

  float temp[n];

  for (int i = 0; i < n; i++) {
    temp[i] = arr[i];
  }

  for (int i = 0; i < n - 1; i++) {

    for (int j = 0; j < n - i - 1; j++) {

      if (temp[j] > temp[j + 1]) {

        float t = temp[j];
        temp[j] = temp[j + 1];
        temp[j + 1] = t;
      }
    }
  }

  return temp[n / 2];
}

// ======================================================
// LEER DISTANCIA HC-SR04
// ======================================================
float leerDistancia() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 30000);

  if (duracion == 0) {
    return -1;
  }

  float distancia = (duracion * 0.034) / 2.0;

  return distancia;
}

// ======================================================
// ALTURA FILTRADA
// ======================================================
float leerAlturaFiltrada() {

  float dist = leerDistancia();

  if (dist < 0) {
    return alturaFiltrada;
  }

  float altura = dist - distanciaInicial;

  // ------------------------
  // Filtro mediana
  // ------------------------
  buffer[bufIdx] = altura;

  bufIdx = (bufIdx + 1) % VENTANA;

  float med = mediana(buffer, VENTANA);

  // ------------------------
  // Filtro exponencial
  // ------------------------
  if (primeraLectura) {

    alturaFiltrada = med;
    primeraLectura = false;

  } else {

    alturaFiltrada = ALPHA * med +
                     (1 - ALPHA) * alturaFiltrada;
  }

  return alturaFiltrada;
}

// ======================================================
// SETUP
// ======================================================
void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  esc.attach(pinESC);

  // ------------------------
  // ARMAR ESC
  // ------------------------
  esc.writeMicroseconds(1000);

  delay(3000);

  // ------------------------
  // Calibración sensor
  // ------------------------
  float suma = 0;

  for (int i = 0; i < 20; i++) {

    suma += leerDistancia();

    delay(100);
  }

  distanciaInicial = suma / 20.0;

  Serial.println("Sistema iniciado");

  tiempoAnterior = millis();
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  unsigned long tiempoActual = millis();

  float dt = (tiempoActual - tiempoAnterior) / 1000.0;

  // Evitar dt muy pequeño
  if (dt <= 0) {
    return;
  }

  // =========================
  // LEER ALTURA
  // =========================
  float altura = leerAlturaFiltrada();

  // =========================
  // ERROR
  // =========================
  error = setpoint - altura;

  // =========================
  // INTEGRAL
  // =========================
  integral += error * dt;

  // Anti-windup integral
  integral = limitar(integral, -20, 20);

  // =========================
  // DERIVATIVA
  // =========================
  float derivativa =
      (alturaAnterior - altura) / dt;

  // =========================
  // PID
  // =========================
  salidaPID =
      (Kp * error) +
      (Ki * integral) +
      (Kd * derivativa);

  // =========================
  // CONTROL SOBRE EQUILIBRIO
  // =========================
  float pwmControl =
      PWM_EQUILIBRIO + salidaPID;

  // =========================
  // SATURADOR
  // =========================
  int pwm =
      limitar(pwmControl,
               PWM_MIN,
               PWM_MAX);

  // =========================
  // ANTI-WINDUP
  // =========================
  if (pwm == PWM_MIN ||
      pwm == PWM_MAX) {

    integral -= error * dt;
  }

  // =========================
  // APLICAR PWM
  // =========================
  esc.writeMicroseconds(pwm);

  // =========================
  // TELEMETRÍA
  // =========================
  Serial.print("SP:");
  Serial.print(setpoint);

  Serial.print(",");

  Serial.print("H:");
  Serial.print(altura);

  Serial.print(",");

  Serial.print("E:");
  Serial.print(error);

  Serial.print(",");

  Serial.print("PID:");
  Serial.print(salidaPID);

  Serial.print(",");

  Serial.print("PWM:");
  Serial.println(pwm);

  // =========================
  // ACTUALIZAR VARIABLES
  // =========================
  alturaAnterior = altura;

  errorAnterior = error;

  tiempoAnterior = tiempoActual;

  // =========================
  // FRECUENCIA DE CONTROL
  // =========================
  delay(20);
}