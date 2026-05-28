/**
 * @file    MisClases.h
 * @brief   Definición de las clases de abstracción de hardware y servicios.
 *
 * Encapsula el acceso a cada periférico y servicio externo en una clase
 * independiente, siguiendo el principio de responsabilidad única (SRP).
 * Clases incluidas:
 *   - Led           → control del LED de alerta
 *   - Boton         → lectura con debounce del pulsador
 *   - LDR           → lectura del sensor de luz ambiente
 *   - SensorDHT     → lectura de temperatura y humedad (DHT22)
 *   - ConexionWiFi  → gestión de la conectividad WiFi
 *   - ThingSpeakClient → envío de datos a la plataforma IoT ThingSpeak
 */

#ifndef MIS_CLASES_H
#define MIS_CLASES_H

#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>


// ============================================================
// CLASE Led
// Controla un LED digital de salida. Mantiene el estado interno
// para evitar lecturas innecesarias del hardware.
// ============================================================

class Led {

  private:
    uint8_t _pin;    ///< Número de GPIO al que está conectado el LED.
    bool    _estado; ///< Estado lógico actual: true = encendido, false = apagado.

  public:

    /**
     * @brief Constructor. Asocia el objeto a un pin GPIO.
     * @param pin  Número de GPIO de salida del LED.
     */
    Led(uint8_t pin)
      : _pin(pin), _estado(false) {}

    /**
     * @brief Configura el pin como salida y apaga el LED.
     *        Debe llamarse una vez en setup().
     */
    void begin() {
      pinMode(_pin, OUTPUT);
      apagar();
    }

    /**
     * @brief Pone el LED en estado HIGH (encendido) y actualiza el estado interno.
     */
    void encender() {
      digitalWrite(_pin, HIGH);
      _estado = true;
    }

    /**
     * @brief Pone el LED en estado LOW (apagado) y actualiza el estado interno.
     */
    void apagar() {
      digitalWrite(_pin, LOW);
      _estado = false;
    }

    /**
     * @brief Consulta el estado actual del LED sin acceder al hardware.
     * @return true si el LED está encendido, false si está apagado.
     */
    bool estaEncendido() const {
      return _estado;
    }
};


// ============================================================
// CLASE Boton
// Lee un pulsador con lógica de debounce por tiempo.
// Utiliza INPUT_PULLUP, por lo que el pin lee LOW al presionar.
// ============================================================

class Boton {

  private:

    uint8_t       _pin;               ///< GPIO de entrada del botón.
    bool          _ultimoEstadoLectura; ///< Último nivel físico leído en el pin.
    bool          _estadoEstable;       ///< Estado validado tras superar el debounce.
    unsigned long _ultimoCambio;        ///< Marca de tiempo del último cambio detectado (ms).

    /** Tiempo mínimo de estabilidad requerido para aceptar un cambio (ms). */
    const unsigned long DEBOUNCE_MS = 50;

  public:

    /**
     * @brief Constructor. Inicializa el botón en estado no presionado (HIGH).
     * @param pin  Número de GPIO de entrada del botón.
     */
    Boton(uint8_t pin)
      : _pin(pin),
        _ultimoEstadoLectura(HIGH),
        _estadoEstable(HIGH),
        _ultimoCambio(0) {}

    /**
     * @brief Configura el pin como entrada con resistencia pull-up interna.
     *        Debe llamarse una vez en setup().
     */
    void begin() {
      pinMode(_pin, INPUT_PULLUP);
    }

    /**
     * @brief Detecta si el botón fue presionado desde la última llamada.
     *
     * Implementa debounce por tiempo: un cambio de nivel solo se acepta
     * cuando permanece estable durante al menos DEBOUNCE_MS milisegundos.
     * Devuelve true exactamente una vez por pulsación (flanco de bajada).
     *
     * @return true en el ciclo en que se confirma una pulsación válida.
     */
    bool fuePulsado() {

      bool lecturaActual = digitalRead(_pin);

      // Registrar el instante del cambio físico para iniciar el temporizador de debounce.
      if (lecturaActual != _ultimoEstadoLectura) {
        _ultimoCambio        = millis();
        _ultimoEstadoLectura = lecturaActual;
      }

      // Validar que el nivel se mantuvo estable el tiempo mínimo de debounce.
      if ((millis() - _ultimoCambio) > DEBOUNCE_MS) {

        if (lecturaActual != _estadoEstable) {
          _estadoEstable = lecturaActual;

          // Flanco de bajada → botón presionado (lógica invertida por PULLUP).
          if (_estadoEstable == LOW) {
            return true;
          }
        }
      }

      return false;
    }
};


// ============================================================
// CLASE LDR
// Lee un fotoresistor (LDR) conectado a un pin analógico del ESP32.
// Ofrece lectura cruda (0–4095) y lectura normalizada (0–100 %).
// ============================================================

class LDR {

  private:

    uint8_t _pin; ///< GPIO analógico (ADC) al que está conectado el LDR.

  public:

    /**
     * @brief Constructor. Asocia el objeto al pin analógico indicado.
     * @param pin  Número de GPIO analógico del LDR.
     */
    LDR(uint8_t pin)
      : _pin(pin) {}

    /**
     * @brief Configura el pin como entrada analógica.
     *        Debe llamarse una vez en setup().
     */
    void begin() {
      pinMode(_pin, INPUT);
    }

    /**
     * @brief Lee el valor crudo del ADC de 12 bits.
     * @return Entero en el rango [0, 4095].
     */
    int leer() {
      return analogRead(_pin);
    }

    /**
     * @brief Convierte la lectura ADC a un porcentaje de luz ambiente.
     *
     * Mapea el rango completo del ADC (0–4095) al rango porcentual (0–100).
     * 0 % = oscuridad total, 100 % = máxima iluminación.
     *
     * @return Porcentaje de luz en el rango [0, 100].
     */
    int leerPorcentaje() {
      int raw = analogRead(_pin);
      return map(raw, 0, 4095, 0, 100);
    }
};


// ============================================================
// CLASE SensorDHT
// Abstrae la lectura del sensor de temperatura y humedad DHT22.
// Implementa reintentos automáticos ante lecturas inválidas (NaN).
// ============================================================

class SensorDHT {

  private:

    DHT   _dht;         ///< Instancia de la librería DHT para comunicación 1-Wire.
    float _temperatura; ///< Último valor de temperatura leído (°C).
    float _humedad;     ///< Último valor de humedad relativa leído (%).

  public:

    /**
     * @brief Constructor. Configura el pin y el tipo de sensor.
     * @param pin   GPIO de datos del sensor DHT.
     * @param tipo  Modelo del sensor: DHT11, DHT21 o DHT22 (por defecto DHT22).
     */
    SensorDHT(uint8_t pin, uint8_t tipo = DHT22)
      : _dht(pin, tipo),
        _temperatura(0.0f),
        _humedad(0.0f) {}

    /**
     * @brief Inicializa la comunicación con el sensor DHT.
     *        Debe llamarse una vez en setup().
     */
    void begin() {
      _dht.begin();
    }

    /**
     * @brief Realiza la lectura del sensor con hasta 3 reintentos.
     *
     * El DHT22 puede devolver NaN si el bus está ocupado o hay ruido.
     * Se reintenta hasta 3 veces con pausa de 500 ms entre intentos.
     * Si la lectura es válida, actualiza los valores internos.
     *
     * @return true si se obtuvo al menos una lectura válida, false si todos los intentos fallaron.
     */
    bool leer() {

      for (int i = 0; i < 3; i++) {

        float t = _dht.readTemperature();
        float h = _dht.readHumidity();

        // isnan() detecta valores inválidos devueltos por la librería DHT.
        if (!isnan(t) && !isnan(h)) {
          _temperatura = t;
          _humedad     = h;
          return true;
        }

        delay(500); // Pausa antes del siguiente reintento.
      }

      return false; // No se obtuvo lectura válida tras los reintentos.
    }

    /**
     * @brief Devuelve la temperatura del último ciclo de lectura exitoso.
     * @return Temperatura en grados Celsius (°C).
     */
    float getTemperatura() const {
      return _temperatura;
    }

    /**
     * @brief Devuelve la humedad del último ciclo de lectura exitoso.
     * @return Humedad relativa en porcentaje (%).
     */
    float getHumedad() const {
      return _humedad;
    }
};


// ============================================================
// CLASE ConexionWiFi
// Gestiona la conexión del ESP32 a una red WiFi.
// Ofrece conexión inicial, verificación de estado y reconexión automática.
// ============================================================

class ConexionWiFi {

  private:

    const char* _ssid;     ///< SSID de la red WiFi destino.
    const char* _password; ///< Contraseña de la red WiFi destino.

  public:

    /**
     * @brief Constructor. Almacena las credenciales de red.
     * @param ssid      Nombre de la red WiFi.
     * @param password  Contraseña de la red WiFi.
     */
    ConexionWiFi(const char* ssid, const char* password)
      : _ssid(ssid),
        _password(password) {}

    /**
     * @brief Inicia el proceso de conexión WiFi y espera hasta 20 segundos.
     *
     * Realiza hasta 40 intentos con intervalo de 500 ms cada uno.
     * Imprime el progreso y la IP asignada por DHCP si la conexión es exitosa.
     *
     * @return true si se estableció la conexión, false si agotó los intentos.
     */
    bool conectar() {

      WiFi.begin(_ssid, _password);
      Serial.print("Conectando WiFi");

      int intentos = 0;

      // Esperar hasta 20 s (40 × 500 ms) a que el AP acepte la conexión.
      while (WiFi.status() != WL_CONNECTED && intentos < 40) {
        delay(500);
        Serial.print(".");
        intentos++;
      }

      Serial.println();

      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
      }

      Serial.println("No conectado");
      return false;
    }

    /**
     * @brief Consulta si el ESP32 tiene conexión WiFi activa.
     * @return true si está conectado, false en caso contrario.
     */
    bool estaConectado() const {
      return WiFi.status() == WL_CONNECTED;
    }

    /**
     * @brief Reconecta automáticamente si la conexión se perdió.
     *        Llamar periódicamente en loop() para mantener la conectividad.
     */
    void reconectar() {
      if (!estaConectado()) {
        conectar();
      }
    }
};


// ============================================================
// CLASE ThingSpeakClient
// Envía datos de sensores al canal ThingSpeak mediante HTTP GET.
// Publica tres campos: luz (field1), temperatura (field2) y humedad (field3).
// ============================================================

class ThingSpeakClient {

  private:

    const char* _server; ///< URL base del endpoint de actualización de ThingSpeak.
    const char* _apiKey; ///< API Key de escritura del canal ThingSpeak.

  public:

    /**
     * @brief Constructor. Almacena el servidor y la clave de API.
     * @param server  URL del endpoint HTTP de ThingSpeak.
     * @param apiKey  Clave de escritura del canal.
     */
    ThingSpeakClient(const char* server, const char* apiKey)
      : _server(server),
        _apiKey(apiKey) {}

    /**
     * @brief Publica los tres valores de sensores en ThingSpeak.
     *
     * Construye la URL con los parámetros GET y realiza la petición HTTP.
     * Solo intenta el envío si hay conexión WiFi activa.
     * ThingSpeak responde con HTTP 200 si los datos se almacenaron correctamente.
     *
     * @param luz         Nivel de luz ambiente (0–100 %).
     * @param temperatura Temperatura en grados Celsius (°C).
     * @param humedad     Humedad relativa en porcentaje (%).
     * @return Código de respuesta HTTP (> 0 si fue exitoso, negativo si hubo error).
     */
    int enviar(int luz, float temperatura, float humedad) {

      // Verificar conectividad antes de intentar el envío.
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Sin WiFi");
        return -1;
      }

      HTTPClient http;

      // Construir la URL con los tres campos en los parámetros de la petición GET.
      String url = String(_server)
                   + "?api_key=" + _apiKey
                   + "&field1="  + String(luz)
                   + "&field2="  + String(temperatura, 1)
                   + "&field3="  + String(humedad, 1);

      http.begin(url);
      int codigoHTTP = http.GET();

      if (codigoHTTP > 0) {
        Serial.println("ThingSpeak OK");
      } else {
        Serial.print("Error HTTP: ");
        Serial.println(codigoHTTP);
      }

      http.end(); // Liberar recursos del cliente HTTP.
      return codigoHTTP;
    }
};


#endif // MIS_CLASES_H