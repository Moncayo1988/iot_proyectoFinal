#ifndef MIS_CLASES_H
#define MIS_CLASES_H

#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ============================================================
// CLASE LED
// ============================================================

class Led {

  private:
    uint8_t _pin;
    bool _estado;

  public:

    Led(uint8_t pin)
      : _pin(pin), _estado(false) {}

    void begin() {

      pinMode(_pin, OUTPUT);

      apagar();
    }

    void encender() {

      digitalWrite(_pin, HIGH);

      _estado = true;
    }

    void apagar() {

      digitalWrite(_pin, LOW);

      _estado = false;
    }

    bool estaEncendido() const {

      return _estado;
    }
};


// ============================================================
// CLASE BOTON
// ============================================================

class Boton {

  private:

    uint8_t _pin;

    bool _ultimoEstadoLectura;

    bool _estadoEstable;

    unsigned long _ultimoCambio;

    const unsigned long DEBOUNCE_MS = 50;

  public:

    Boton(uint8_t pin)
      : _pin(pin),
        _ultimoEstadoLectura(HIGH),
        _estadoEstable(HIGH),
        _ultimoCambio(0) {}

    void begin() {

      pinMode(_pin, INPUT_PULLUP);
    }

    bool fuePulsado() {

      bool lecturaActual = digitalRead(_pin);

      // Detectar cambio físico
      if (lecturaActual != _ultimoEstadoLectura) {

        _ultimoCambio = millis();

        _ultimoEstadoLectura = lecturaActual;
      }

      // Validar estabilidad
      if ((millis() - _ultimoCambio) > DEBOUNCE_MS) {

        if (lecturaActual != _estadoEstable) {

          _estadoEstable = lecturaActual;

          // PRESIONADO
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
// ============================================================

class LDR {

  private:

    uint8_t _pin;

  public:

    LDR(uint8_t pin)
      : _pin(pin) {}

    void begin() {

      pinMode(_pin, INPUT);
    }

    int leer() {

      return analogRead(_pin);
    }

    int leerPorcentaje() {

      int raw = analogRead(_pin);

      return map(raw, 0, 4095, 0, 100);
    }
};


// ============================================================
// CLASE SENSOR DHT22
// ============================================================

class SensorDHT {

  private:

    DHT _dht;

    float _temperatura;

    float _humedad;

  public:

    SensorDHT(uint8_t pin, uint8_t tipo = DHT22)
      : _dht(pin, tipo),
        _temperatura(0.0f),
        _humedad(0.0f) {}

    void begin() {

      _dht.begin();
    }

    bool leer() {

      for (int i = 0; i < 3; i++) {

        float t = _dht.readTemperature();

        float h = _dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {

          _temperatura = t;

          _humedad = h;

          return true;
        }

        delay(500);
      }

      return false;
    }

    float getTemperatura() const {

      return _temperatura;
    }

    float getHumedad() const {

      return _humedad;
    }
};


// ============================================================
// CLASE WIFI
// ============================================================

class ConexionWiFi {

  private:

    const char* _ssid;

    const char* _password;

  public:

    ConexionWiFi(const char* ssid, const char* password)
      : _ssid(ssid),
        _password(password) {}

    bool conectar() {

      WiFi.begin(_ssid, _password);

      Serial.print("Conectando WiFi");

      int intentos = 0;

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

    bool estaConectado() const {

      return WiFi.status() == WL_CONNECTED;
    }

    void reconectar() {

      if (!estaConectado()) {

        conectar();
      }
    }
};


// ============================================================
// CLASE THINGSPEAK
// ============================================================

class ThingSpeakClient {

  private:

    const char* _server;

    const char* _apiKey;

  public:

    ThingSpeakClient(const char* server, const char* apiKey)
      : _server(server),
        _apiKey(apiKey) {}

    int enviar(int luz, float temperatura, float humedad) {

      if (WiFi.status() != WL_CONNECTED) {

        Serial.println("Sin WiFi");

        return -1;
      }

      HTTPClient http;

      String url = String(_server)
                   + "?api_key=" + _apiKey
                   + "&field1=" + String(luz)
                   + "&field2=" + String(temperatura, 1)
                   + "&field3=" + String(humedad, 1);

      http.begin(url);

      int codigoHTTP = http.GET();

      if (codigoHTTP > 0) {

        Serial.println("ThingSpeak OK");
      }
      else {

        Serial.print("Error HTTP: ");

        Serial.println(codigoHTTP);
      }

      http.end();

      return codigoHTTP;
    }
};

#endif