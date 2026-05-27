// ============================================================
// main.cpp
// ESP32 + WiFi + LDR + ThingSpeak
// Lee el valor del sensor LDR y lo reporta periódicamente
// a ThingSpeak mediante una petición HTTP GET.
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Config.h"

// ------------------------------------------------------------
// Variables globales de control
// ------------------------------------------------------------

// Guarda el instante del último envío realizado a ThingSpeak.
unsigned long ultimoEnvio = 0;

// Intervalo entre envíos en milisegundos.
// 2000 ms = 2 segundos.
const long intervaloTS = 15000;

// ------------------------------------------------------------
// Función: enviarAThingSpeak
// Propósito:
//   Construye la URL con el valor leído y envía los datos
//   al canal de ThingSpeak usando HTTP GET.
// Parámetro:
//   valor -> lectura analógica del LDR.
// ------------------------------------------------------------
void enviarAThingSpeak(int valor) {
  // Solo intenta enviar si el ESP32 sigue conectado al WiFi.
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construcción de la URL con la API key y el valor del campo 1.
    String url = String(TS_SERVER) +
                 "?api_key=" + TS_API_KEY +
                 "&field1=" + String(valor);

    // Inicia la conexión HTTP hacia ThingSpeak.
    http.begin(url);

    // Ejecuta la petición GET y guarda el código de respuesta.
    int httpCode = http.GET();

    // Muestra resultado del envío por el monitor serial.
    if (httpCode > 0) {
      Serial.println("ThingSpeak actualizado: " +
                     String(valor) +
                     " (HTTP " + String(httpCode) + ")");
    } else {
      Serial.println("Error enviando a ThingSpeak");
    }

    // Libera recursos de la conexión HTTP.
    http.end();
  } else {
    Serial.println("WiFi desconectado, no se pudo enviar a ThingSpeak");
  }
}

// ------------------------------------------------------------
// Función: setup
// Propósito:
//   Configura la comunicación serial, el pin del LDR
//   y la conexión inicial a la red WiFi.
// ------------------------------------------------------------
void setup() {
  // Inicializa el monitor serial.
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);

  // Configura el pin del LDR como entrada analógica.
  pinMode(LDR_PIN, INPUT);

  // Inicia la conexión WiFi con las credenciales definidas.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando WiFi");

  // Espera hasta que la conexión sea exitosa.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Muestra confirmación de conexión e IP asignada.
  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
}

// ------------------------------------------------------------
// Función: loop
// Propósito:
//   Ejecuta el ciclo principal del programa.
//   Cada cierto intervalo:
//   1. Lee el valor del LDR.
//   2. Lo muestra por serial.
//   3. Lo envía a ThingSpeak.
// ------------------------------------------------------------
void loop() {
  // Verifica si ya pasó el tiempo necesario para un nuevo envío.
  if (millis() - ultimoEnvio >= intervaloTS) {
    // Actualiza la marca de tiempo del último envío.
    ultimoEnvio = millis();

    // Lee el valor analógico actual del sensor LDR.
    int valorLuz = analogRead(LDR_PIN);

    // Muestra la lectura localmente por serial.
    Serial.print("Lectura LDR: ");
    Serial.println(valorLuz);

    // Envía la lectura a ThingSpeak.
    enviarAThingSpeak(valorLuz);
  }
}