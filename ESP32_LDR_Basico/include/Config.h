#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Config.h
// Archivo de configuración central del proyecto.
// Aquí se definen credenciales, servidor, pines y parámetros
// generales para facilitar mantenimiento y reutilización.
// ============================================================

// ------------------------------------------------------------
// Configuración de red WiFi
// ------------------------------------------------------------

// Nombre de la red WiFi.
const char* WIFI_SSID = "Wokwi-GUEST";

// Contraseña de la red WiFi.
const char* WIFI_PASSWORD = "";

// ------------------------------------------------------------
// Configuración de ThingSpeak
// ------------------------------------------------------------

// Endpoint para actualizar datos en ThingSpeak.
const char* TS_SERVER = "http://api.thingspeak.com/update";

// Clave de escritura del canal de ThingSpeak.
const char* TS_API_KEY = "TU_PROPIA_KEY";

// ------------------------------------------------------------
// Configuración de hardware y depuración
// ------------------------------------------------------------

// Pin donde está conectado el sensor LDR.
#define LDR_PIN 2

// Velocidad del monitor serial.
#define SERIAL_BAUDRATE 115200

#endif