/*
 * ========================================
 * GUARDIÃO DE HABITATS EXTREMOS v2.0
 * Lunar/Mars Colony Atmospheric Monitor
 * ========================================
 */

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
WebServer server(80);

const int PRESSURE_SENSOR = 34;
const int RADIATION_SENSOR = 35;

const int LED_GREEN = 25;
const int LED_YELLOW = 26;
const int LED_RED = 27;

int pressureValue = 0;
int radiationValue = 0;
String statusAtual = "INICIALIZANDO";
String statusCor = "#FFD700";
unsigned long lastUpdate = 0;

enum HabitatStatus {
  SEGURO = 0,
  ALERTA = 1,
  CRITICO = 2
};

HabitatStatus classificarAmbiente(int pressure, int radiation) {
  if (pressure < 300 || pressure > 800 || radiation < 100 || radiation > 600) {
    return CRITICO;
  }
  
  if ((pressure >= 300 && pressure < 380) ||
      (pressure > 680 && pressure <= 800) ||
      (radiation >= 100 && radiation < 180) ||
      (radiation > 480 && radiation <= 600)) {
    return ALERTA;
  }
  
  return SEGURO;
}

void atualizarStatus() {
  pressureValue = analogRead(PRESSURE_SENSOR);
  radiationValue = analogRead(RADIATION_SENSOR);
  
  HabitatStatus status = classificarAmbiente(pressureValue, radiationValue);
  
  switch(status) {
    case SEGURO:
      statusAtual = "SEGURO";
      statusCor = "#00DD00";
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);
      break;
      
    case ALERTA:
      statusAtual = "ALERTA";
      statusCor = "#FFDD00";
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_RED, LOW);
      break;
      
    case CRITICO:
      statusAtual = "CRITICO";
      statusCor = "#FF0000";
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, HIGH);
      break;
  }
  
  lastUpdate = millis();
  
  Serial.println("Pressao: " + String(pressureValue) + " | Radiacao: " + String(radiationValue) + " | Status: " + statusAtual);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Lunar Habitat</title>";
  html += "<style>body{background:#000a0f;color:#00ff99;font-family:monospace;display:flex;justify-content:center;align-items:center;min-height:100vh;}";
  html += ".container{background:rgba(10,20,50,0.9);border:3px solid #00ff99;border-radius:15px;padding:40px;max-width:600px;}";
  html += "h1{text-align:center;margin-bottom:30px;}";
  html += ".status{font-size:3em;text-align:center;color:" + statusCor + ";margin:20px 0;}";
  html += ".sensor{display:grid;grid-template-columns:1fr 1fr;gap:15px;margin:20px 0;}";
  html += ".box{background:#0a1a2e;border:1px solid #00ff99;padding:20px;text-align:center;}";
  html += ".value{font-size:2em;color:#00ff99;font-weight:bold;}</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>GUARDIAO DE HABITATS EXTREMOS</h1>";
  html += "<div class='status'>" + statusAtual + "</div>";
  html += "<div class='sensor'>";
  html += "<div class='box'><strong>PRESSAO</strong><div class='value'>" + String(pressureValue) + "</div></div>";
  html += "<div class='box'><strong>RADIACAO</strong><div class='value'>" + String(radiationValue) + "</div></div>";
  html += "</div>";
  html += "<p style='text-align:center;margin-top:20px;font-size:0.9em;'>";
  html += "Seguro: 400-600 mbar | Alerta: Desvios ±80 | Critico: >100 desvio";
  html += "</p></div></body></html>";
  
  server.send(200, "text/html; charset=UTF-8", html);
}

void handleJSON() {
  String json = "{\"status\":\"" + statusAtual + "\",\"pressure\":" + String(pressureValue) + ",\"radiation\":" + String(radiationValue) + ",\"color\":\"" + statusCor + "\"}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(PRESSURE_SENSOR, INPUT);
  pinMode(RADIATION_SENSOR, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  
  Serial.println("\n===== GUARDIAO DE HABITATS EXTREMOS v2.0 =====");
  
  Serial.println("Conectando ao WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.println("IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi offline - continuando...");
  }
  
  server.on("/", handleRoot);
  server.on("/api/status", handleJSON);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("Webserver iniciado na porta 80");
  Serial.println("Monitorando sensores...\n");
  
  atualizarStatus();
}

void loop() {
  server.handleClient();
  
  static unsigned long lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate > 500) {
    atualizarStatus();
    lastSensorUpdate = millis();
  }
  
  delay(10);
}
