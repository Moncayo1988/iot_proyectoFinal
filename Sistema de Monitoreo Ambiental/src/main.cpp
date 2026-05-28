/**
 * @file    main.cpp
 * @brief   Punto de entrada del sistema de monitoreo ambiental con ESP32.
 *
 * Integra los módulos de hardware (LED, botón, LDR, DHT22) con los servicios
 * de red (WiFi y ThingSpeak) para:
 *   - Leer temperatura, humedad y luz ambiente de forma periódica.
 *   - Activar un LED de alerta cuando la temperatura supera el umbral definido.
 *   - Permitir silenciar la alerta mediante un botón físico con debounce.
 *   - Publicar los datos de sensores en ThingSpeak cada INTERVALO_ENVIO ms.
 *
 * Dependencias: Config.h, MisClases.h
 */

#include <Arduino.h>
#include "Config.h"
#include "MisClases.h"


// ======================================================
// INSTANCIAS DE OBJETOS
// Cada objeto encapsula un periférico o servicio.
// Los pines y credenciales provienen de Config.h.
// ======================================================

Led              led(PIN_LED);                     ///< LED de alerta en GPIO PIN_LED.
Boton            boton(PIN_BOTON);                 ///< Pulsador de silencio en GPIO PIN_BOTON.
LDR              ldr(PIN_LDR);                     ///< Fotoresistor en GPIO analógico PIN_LDR.
SensorDHT        sensorDHT(PIN_DHT);               ///< Sensor DHT22 de temperatura/humedad en GPIO PIN_DHT.
ConexionWiFi     wifi(WIFI_SSID, WIFI_PASSWORD);   ///< Gestor de conectividad WiFi.
ThingSpeakClient thingSpeak(TS_SERVER, TS_API_KEY); ///< Cliente HTTP para publicar en ThingSpeak.


// ======================================================
// VARIABLES GLOBALES DE ESTADO
// ======================================================

/** Indica si la alerta de temperatura está actualmente activa. */
bool alertaActiva = false;

/**
 * Bandera de silencio activada por el botón.
 * Impide que la alerta se reactive mientras la temperatura siga alta.
 * Se limpia automáticamente cuando la temperatura vuelve a bajar
 * del umbral TEMP_ALERTA, permitiendo futuros disparos.
 */
bool alertaSilenciada = false;

/** Marca de tiempo (ms) del último envío exitoso a ThingSpeak. */
unsigned long ultimoEnvio = 0;


// ======================================================
// SETUP
// Inicialización de hardware y conexión de red.
// Se ejecuta una sola vez al arrancar el ESP32.
// ======================================================

void setup() {

  Serial.begin(SERIAL_BAUDRATE); // Iniciar puerto serie para depuración.

  led.begin();       // Configurar GPIO del LED como salida (apagado).
  boton.begin();     // Configurar GPIO del botón con pull-up interno.
  ldr.begin();       // Configurar GPIO del LDR como entrada analógica.
  sensorDHT.begin(); // Iniciar comunicación 1-Wire con el DHT22.

  wifi.conectar();   // Conectar a la red WiFi; espera hasta 20 s.

  Serial.println("Sistema iniciado");
}


// ======================================================
// LOOP
// Ciclo principal de control del sistema.
// Se ejecuta de forma continua a una cadencia aproximada de 100 ms.
// ======================================================

void loop() {

  // Reconectar WiFi si la conexión se perdió desde el ciclo anterior.
  wifi.reconectar();


  // ==========================================
  // LECTURA DEL BOTÓN
  // Se evalúa fuera del bloque DHT para no
  // perder el pulso del usuario durante los
  // retardos de lectura del sensor.
  // ==========================================
  if (boton.fuePulsado()) {

    // El usuario silencia la alerta: apagar LED y bloquear reactivación.
    alertaActiva     = false;
    alertaSilenciada = true;

    led.apagar();

    Serial.println("Alerta reiniciada por botón");
  }


  // ==========================================
  // LECTURA DE SENSORES (DHT22 y LDR)
  // sensorDHT.leer() devuelve false si todos
  // los reintentos fallaron; en ese caso se
  // omite el procesamiento de este ciclo.
  // ==========================================
  if (sensorDHT.leer()) {

    float temperatura = sensorDHT.getTemperatura(); // Temperatura en °C.
    float humedad     = sensorDHT.getHumedad();     // Humedad relativa en %.
    int   luz         = ldr.leerPorcentaje();        // Nivel de luz en % (0–100).


    // ==========================================
    // GESTIÓN DE LA LÓGICA DE ALERTA
    //
    // Regla de activación:
    //   - temperatura > TEMP_ALERTA  → activa alerta (si no está silenciada).
    //   - temperatura ≤ TEMP_ALERTA  → desactiva alerta y limpia el silencio,
    //                                  habilitando futuros disparos automáticos.
    // ==========================================

    if (temperatura <= TEMP_ALERTA) {

      // Temperatura dentro del rango normal: restablecer estado de alerta.
      alertaSilenciada = false;
      alertaActiva     = false;

    } else {

      // Temperatura sobre el umbral: activar alerta solo si no fue silenciada.
      if (!alertaSilenciada) {
        alertaActiva = true;
      }
    }


    // ==========================================
    // CONTROL DEL LED DE ALERTA
    // Refleja el estado de alertaActiva en el hardware.
    // ==========================================
    if (alertaActiva) {
      led.encender();
    } else {
      led.apagar();
    }


    // ==========================================
    // REPORTE POR PUERTO SERIE
    // Útil para depuración y verificación en tiempo real.
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
    // ENVÍO A THINGSPEAK
    // Se respeta el intervalo mínimo para no
    // superar el límite de peticiones de la
    // cuenta gratuita (1 envío cada 15 s).
    // ==========================================
    if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

      thingSpeak.enviar(luz, temperatura, humedad); // Publicar los tres campos.

      ultimoEnvio = millis(); // Actualizar marca de tiempo del último envío.
    }
  }

  delay(100); // Pausa de 100 ms para reducir carga de CPU y estabilizar lecturas.
}