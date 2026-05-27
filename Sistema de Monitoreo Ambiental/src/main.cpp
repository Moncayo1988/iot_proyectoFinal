#include <Arduino.h>

#include "Config.h"
#include "MisClases.h"

// ======================================================
// OBJETOS
// ======================================================

Led led(PIN_LED);

Boton boton(PIN_BOTON);

LDR ldr(PIN_LDR);

SensorDHT sensorDHT(PIN_DHT);

ConexionWiFi wifi(WIFI_SSID, WIFI_PASSWORD);

ThingSpeakClient thingSpeak(TS_SERVER, TS_API_KEY);

// ======================================================
// VARIABLES GLOBALES
// ======================================================

bool alertaActiva = false;

// Bandera: el usuario silenció la alerta con el botón.
// Se mantiene activa hasta que la temperatura baje de TEMP_ALERTA,
// momento en que se limpia para poder volver a dispararse.
bool alertaSilenciada = false;

unsigned long ultimoEnvio = 0;


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(SERIAL_BAUDRATE);

  led.begin();

  boton.begin();

  ldr.begin();

  sensorDHT.begin();

  wifi.conectar();

  Serial.println("Sistema iniciado");
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  wifi.reconectar();

  // ==========================================
  // LEER BOTÓN — siempre primero, fuera del
  // bloque DHT, para no perder el pulso.
  // ==========================================
  if (boton.fuePulsado()) {

    alertaActiva    = false;
    alertaSilenciada = true;

    led.apagar();

    Serial.println("Alerta reiniciada por botón");
  }

  // ==========================================
  // LEER DHT22
  // ==========================================
  if (sensorDHT.leer()) {

    float temperatura = sensorDHT.getTemperatura();

    float humedad = sensorDHT.getHumedad();

    int luz = ldr.leerPorcentaje();

    // ==========================================
    // GESTIÓN DE ALERTA
    //
    // Regla: el LED enciende cuando temperatura
    // supera TEMP_ALERTA. El botón la silencia
    // hasta que la temperatura vuelva a bajar
    // del umbral (reset automático del silencio).
    // ==========================================

    if (temperatura <= TEMP_ALERTA) {

      // Temperatura normal: limpiar silencio y alerta
      alertaSilenciada = false;
      alertaActiva     = false;

    } else {

      // Temperatura alta: activar alerta solo si
      // el usuario no la ha silenciado
      if (!alertaSilenciada) {

        alertaActiva = true;
      }
    }

    // ==========================================
    // CONTROL LED
    // ==========================================
    if (alertaActiva) {

      led.encender();

    } else {

      led.apagar();
    }

    // ==========================================
    // SERIAL
    // ==========================================
    Serial.println("==============");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    Serial.print("Luz: ");
    Serial.println(luz);

    Serial.print("Alerta: ");

    if (alertaActiva) {
      Serial.println("ACTIVA");
    } else {
      Serial.println("APAGADA");
    }

    Serial.print("Silenciada: ");
    Serial.println(alertaSilenciada ? "SI" : "NO");

    // ==========================================
    // ENVÍO THINGSPEAK
    // ==========================================
    if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

      thingSpeak.enviar(luz, temperatura, humedad);

      ultimoEnvio = millis();
    }
  }

  delay(100);
}