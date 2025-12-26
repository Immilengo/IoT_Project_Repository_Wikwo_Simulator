#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- PINOS ----------
#define PRESSURE_PIN 34
#define ONE_WIRE_BUS 15

// ---------- SENSOR TEMP ----------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ---------- PARÂMETROS ----------
const float PRESSAO_MAX = 6.0;   // bar
const float TEMP_REF = 25.0;     // °C
const float COEF_TEMP = 0.06;    // bar/°C

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

float lerPressao() {
  int raw = analogRead(PRESSURE_PIN);
  float tensao = raw * (3.3 / 4095.0);
  return (tensao / 3.3) * PRESSAO_MAX;
}

float corrigirPressao(float p, float t) {
  return p + COEF_TEMP * (t - TEMP_REF);
}

void loop() {
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  float pressaoRaw = lerPressao();
  float pressaoCorrigida = corrigirPressao(pressaoRaw, temperatura);

  Serial.println("----- ENVIO PARA BACKEND -----");
  Serial.print("{ \"pressao\": ");
  Serial.print(pressaoCorrigida);
  Serial.print(", \"temperatura\": ");
  Serial.print(temperatura);
  Serial.println(" }");

  delay(5000);
}


/*#include <WiFi.h>
#include <FirebaseClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- WIFI ----------
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// ---------- FIREBASE ----------
#define API_KEY "SUA_API_KEY"
#define DATABASE_URL "https://SEU_PROJETO.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------- PINOS ----------
#define PRESSURE_PIN 34
#define ONE_WIRE_BUS 15

// ---------- SENSORES ----------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ---------- PARÂMETROS ----------
const float PRESSAO_MAX = 6.0;   // bar
const float PRESSAO_MIN = 0.5;   // bar
const float TEMP_REF = 25.0;     // °C
const float COEF_TEMP = 0.06;    // bar/°C

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  sensors.begin();
}

float lerPressao() {
  int raw = analogRead(PRESSURE_PIN);
  float tensao = raw * (3.3 / 4095.0);
  float pressao = (tensao / 3.3) * PRESSAO_MAX;
  return pressao;
}

float corrigirPressao(float p, float t) {
  return p + COEF_TEMP * (t - TEMP_REF);
}

void loop() {
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  float pressaoRaw = lerPressao();
  float pressaoCorrigida = corrigirPressao(pressaoRaw, temperatura);

  Serial.println("----- LEITURA -----");
  Serial.print("Pressão (raw): ");
  Serial.print(pressaoRaw);
  Serial.println(" bar");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Pressão corrigida: ");
  Serial.print(pressaoCorrigida);
  Serial.println(" bar");

  // Envio Firebase
  if (Firebase.ready()) {
    FirebaseJson json;
    json.set("pressao", pressaoCorrigida);
    json.set("temperatura", temperatura);
    json.set("timestamp", millis());

    Firebase.RTDB.pushJSON(&fbdo, "/botijas/A123/leituras", &json);
  }

  delay(60000); // 1 minuto
}
*/ 