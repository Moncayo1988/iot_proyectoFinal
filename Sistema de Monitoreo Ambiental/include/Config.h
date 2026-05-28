/**
 * @file    Config.h
 * @brief   Parámetros de configuración global del sistema de monitoreo ambiental.
 *
 * Centraliza todas las credenciales de red, claves de API, asignación de pines
 * y constantes de comportamiento del sistema. Modificar únicamente este archivo
 * para adaptar el proyecto a un entorno diferente, sin tocar la lógica principal.
 */

#ifndef CONFIG_H
#define CONFIG_H


// ============================================================
// RED WiFi
// Credenciales de la red inalámbrica a la que se conectará el ESP32.
// ============================================================

/** SSID (nombre) de la red WiFi. Usar "Wokwi-GUEST" en simulación. */
const char* WIFI_SSID     = "Wokwi-GUEST";

/** Contraseña de la red WiFi. Dejar vacío para redes abiertas. */
const char* WIFI_PASSWORD = "";


// ============================================================
// THINGSPEAK
// Parámetros de la plataforma IoT para el envío de datos en la nube.
// ============================================================

/** URL del endpoint HTTP de actualización de ThingSpeak. */
const char* TS_SERVER  = "http://api.thingspeak.com/update";

/** API Key de escritura del canal ThingSpeak. Única por canal. */
const char* TS_API_KEY = "F5YYVEKP9051VYJF";


// ============================================================
// PINES ESP32
// Asignación de GPIO para cada periférico del sistema.
// ============================================================

/** GPIO analógico (ADC) conectado al divisor de tensión del LDR. */
#define PIN_LDR    34

/** GPIO de datos del sensor de temperatura/humedad DHT22. */
#define PIN_DHT     4

/** GPIO de salida para el LED de alerta. */
#define PIN_LED     2

/** GPIO de entrada para el botón de silencio de alerta (pull-up interno). */
#define PIN_BOTON  15


// ============================================================
// PARÁMETROS DEL SISTEMA
// Constantes que controlan el comportamiento general del firmware.
// ============================================================

/** Velocidad de comunicación serie en baudios (bits por segundo). */
#define SERIAL_BAUDRATE   115200

/**
 * Intervalo mínimo entre envíos a ThingSpeak (en milisegundos).
 * ThingSpeak impone un límite de un envío cada 15 s en cuentas gratuitas.
 * Valor actual: 20 000 ms = 20 s.
 */
#define INTERVALO_ENVIO   20000UL

/**
 * Temperatura umbral de alerta en grados Celsius.
 * Cuando la lectura del DHT22 supera este valor, se activa el LED.
 */
#define TEMP_ALERTA       30.0f


#endif // CONFIG_H