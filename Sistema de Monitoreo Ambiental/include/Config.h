#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// WiFi
// ============================================================

/** SSID de la red WiFi. */
const char* WIFI_SSID = "Wokwi-GUEST";

/** Contraseña WiFi. */
const char* WIFI_PASSWORD = "";


// ============================================================
// ThingSpeak
// ============================================================

/** Endpoint HTTP de actualización de ThingSpeak. */
const char* TS_SERVER = "http://api.thingspeak.com/update";

/** API Key de escritura del canal ThingSpeak. */
const char* TS_API_KEY = "F5YYVEKP9051VYJF";


// ============================================================
// Pines ESP32
// ============================================================

/** Pin analógico del LDR */
#define PIN_LDR 34

/** Pin del DHT22 */
#define PIN_DHT 4

/** Pin del LED */
#define PIN_LED 2

/** Pin del botón */
#define PIN_BOTON 15


// ============================================================
// Configuración sistema
// ============================================================

/** Velocidad Serial */
#define SERIAL_BAUDRATE 115200

/** Intervalo de envío ThingSpeak */
#define INTERVALO_ENVIO 20000UL

/** Temperatura de alerta */
#define TEMP_ALERTA 30.0f

#endif