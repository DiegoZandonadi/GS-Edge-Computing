#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// CONFIG WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// CONFIG ThingSpeak
String apiKey = "S45Y30Y1WD4FBVFY";
String server = "http://api.thingspeak.com/update";

// CONFIG Sensores 
#define DHTPIN 21
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LDR_PIN 13

// LIMITES PARA ALERTAS
const float TEMP_MAX = 30.0;   // Calor excessivo
const float TEMP_MIN = 15.0;   // Ambiente frio demais
const float HUM_MIN  = 35.0;   // Ar muito seco
const int LUX_MIN    = 180;    // Baixa luminosidade

// Variáveis para avisos controlados
unsigned long lastAlert = 0;
const unsigned long alertInterval = 30000;  // 30s sem repetir alerta

void setup() {
  Serial.begin(115200);
  dht.begin();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  // Garante que o WiFi está conectado
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Leitura dos sensores
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    int ldrRaw = analogRead(LDR_PIN);

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Erro ao ler o DHT22!");
      return;
    }

    // Envio ao ThingSpeak
    String url = server +
                 "?api_key=" + apiKey +
                 "&field1=" + String(temp) +
                 "&field2=" + String(hum) +
                 "&field3=" + String(ldrRaw);

    http.begin(url);
    int response = http.GET();

    if (response > 0) {
      Serial.println("Dados enviados!");
    } else {
      Serial.println("Erro ao enviar: " + String(response));
    }

    http.end();

    // Sistema de alertas
    unsigned long now = millis();
    if (now - lastAlert > alertInterval) {
      
      if (temp > TEMP_MAX) {
        Serial.println("ALERTA: Temperatura muito alta – risco de desconforto térmico e maior consumo de energia por climatização.");
      }

      if (temp < TEMP_MIN) {
        Serial.println("ALERTA: Temperatura muito baixa – pode aumentar uso de aquecedores.");
      }

      if (hum < HUM_MIN) {
        Serial.println("ALERTA: Ar seco – pode causar irritação ocular e respiratória.");
      }

      if (ldrRaw < LUX_MIN) {
        Serial.println("ALERTA: Baixa luminosidade – iluminação inadequada aumenta fadiga visual e consumo desnecessário de luz artificial.");
      }

      lastAlert = now;
    }
  }

  delay(15000);  // Delay padrão do ThingSpeak
}