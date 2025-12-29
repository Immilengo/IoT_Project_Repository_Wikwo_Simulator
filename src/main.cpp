#include <WiFi.h>
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

// ---------- PARAMETROS ----------
const float PRESSAO_MAX = 6.0;
const float TEMP_REF = 25.0;
const float COEF_TEMP = 0.06;

// ---------- FUNCOES ----------
float lerPressao() {
  int raw = analogRead(PRESSURE_PIN);
  float tensao = raw * (3.3 / 4095.0);
  return (tensao / 3.3) * PRESSAO_MAX;
}

float corrigirPressao(float p, float t) {
  return p + COEF_TEMP * (t - TEMP_REF);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  if (temperatura == DEVICE_DISCONNECTED_C) return;

  float pressaoRaw = lerPressao();
  float pressaoCorrigida = corrigirPressao(pressaoRaw, temperatura);

  FirebaseJson json;
  json.set("pressao", pressaoCorrigida);
  json.set("temperatura", temperatura);

  Firebase.RTDB.pushJSON(
    &fbdo,
    "/botijas/A123/leiturasRaw",
    &json
  );

  delay(60000); // 1 minuto
}
