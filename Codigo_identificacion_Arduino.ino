#include <Servo.h>

Servo esc;
int pinESC  = 9;
int trigPin = 6;
int echoPin = 7;
float distanciaInicial = 0;

// --- Filtro de mediana (ventana de 5 muestras) ---
// Elimina picos falsos del sensor
#define VENTANA 5
float buffer[VENTANA] = {0};
int bufIdx = 0;

float mediana(float arr[], int n) {
  float temp[n];
  for (int i = 0; i < n; i++) temp[i] = arr[i];
  // Ordenar burbuja
  for (int i = 0; i < n-1; i++)
    for (int j = 0; j < n-i-1; j++)
      if (temp[j] > temp[j+1]) {
        float t = temp[j];
        temp[j] = temp[j+1];
        temp[j+1] = t;
      }
  return temp[n/2];
}

// --- Filtro pasa bajos (promedio exponencial) ---
// Alpha cercano a 0 = más suavizado, cercano a 1 = menos suavizado
#define ALPHA 0.3
float alturaFiltrada = 0;
bool primeraLectura = true;

const int NUM_PASOS = 2;

int secuencia[NUM_PASOS][2] = {
  {0000, 5000},    // equilibrio 5 s
  {1280, 20000}    // escalón 20 s
};

int pasoActual = 0;
unsigned long tiempoInicioPaso = 0;
bool secuenciaTerminada = false;

float leerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH, 30000);
  if (duracion == 0) return -1;
  return (duracion * 0.034) / 2;
}

float leerAlturaFiltrada() {
  float dist = leerDistancia();
  if (dist < 0) return alturaFiltrada; // Si falla, retorna último valor válido

  float altura = dist - distanciaInicial;

  // 1. Filtro de mediana: elimina picos
  buffer[bufIdx] = altura;
  bufIdx = (bufIdx + 1) % VENTANA;
  float med = mediana(buffer, VENTANA);

  // 2. Filtro exponencial: suaviza el ruido
  if (primeraLectura) {
    alturaFiltrada = med;
    primeraLectura = false;
  } else {
    alturaFiltrada = ALPHA * med + (1 - ALPHA) * alturaFiltrada;
  }

  return alturaFiltrada;
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  esc.attach(pinESC);

  esc.writeMicroseconds(1000);
  delay(3000);

  float suma = 0;
  for (int i = 0; i < 10; i++) {
    suma += leerDistancia();
    delay(100);
  }
  distanciaInicial = suma / 10.0;
  Serial.println("tiempo_ms,velocidad_us,altura_cm");

  tiempoInicioPaso = millis();
  esc.writeMicroseconds(secuencia[0][0]);
}

void loop() {
  if (secuenciaTerminada) {
    esc.writeMicroseconds(1000);
    return;
  }

  unsigned long ahora = millis();

  if (ahora - tiempoInicioPaso >= (unsigned long)secuencia[pasoActual][1]) {
    pasoActual++;
    if (pasoActual >= NUM_PASOS) {
      secuenciaTerminada = true;
      esc.writeMicroseconds(1000);
      Serial.println("FIN_SECUENCIA");
      return;
    }
    esc.writeMicroseconds(secuencia[pasoActual][0]);
    tiempoInicioPaso = ahora;
  }

  float altura = leerAlturaFiltrada();

  Serial.print(ahora);
  Serial.print(",");
  Serial.print(secuencia[pasoActual][0]);
  Serial.print(",");
  Serial.println(altura);

  delay(100);
}