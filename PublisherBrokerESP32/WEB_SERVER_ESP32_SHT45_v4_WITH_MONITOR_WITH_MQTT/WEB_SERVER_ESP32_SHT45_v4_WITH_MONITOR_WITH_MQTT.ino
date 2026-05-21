#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <esp_sntp.h>
#include <esp_chip_info.h>

// MQTT
#include <PubSubClient.h>

#define SCL_PIN 21
#define SDA_PIN 22

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

//----------------------------------
// MQTT
//----------------------------------
const char *mqtt_server = "192.168.1.14";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMqttPublish = 0;

//----------------------------------
// Definir credenciais Wi-Fi
//----------------------------------
const char *ssid = "GABRIEL_HOME";
const char *password = "@FlakE2021#";

//----------------------------------
// Configurar IP estático
//----------------------------------
IPAddress local_IP(192, 168, 1, 103);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

//----------------------------------
// Configurar porta http
//----------------------------------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

unsigned long lastPrint = 0;
static unsigned long lastNtpSync = 0;

//-------------------------------------------------
// Declaração antecipada
//------------------------------------------------
void connectWiFi();
void handleRoot(AsyncWebServerRequest *request);
void handleJSON(AsyncWebServerRequest *request);
void syncTime();

// MQTT
void reconnectMQTT();
void publishSensorData();

void sendSystemInfo() {
  String json = generateJSON();
  ws.textAll(json);
}

//------------------------------------------------
String getCurrentDateTime(int attempts = 4) {
  String dateTime = "❌ Erro ao obter data e hora";

  while (attempts-- > 0) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
      dateTime = String(buffer);
      break;
    }
    delay(1000);
  }
  return dateTime;
}

//------------------------------------------------
bool tryReadSensor(float &temperatureCelsius, float &temperatureFahrenheit, float &humidity, bool origem, int attempts = 4) {
  while (attempts-- > 0) {
    sensors_event_t humidityEvent, tempEvent;
    sht4.getEvent(&humidityEvent, &tempEvent);

    temperatureCelsius = tempEvent.temperature;
    humidity = humidityEvent.relative_humidity;

    if (isnan(temperatureCelsius) || isnan(humidity)) {
      if (origem)
        Serial.println("❌ Falha ao ler SHT45... Tentativas restantes: " + String(attempts));
      delay(2000);
      continue;
    }

    temperatureFahrenheit = temperatureCelsius * 1.8 + 32.0;
    return true;
  }

  return false;
}

//------------------------------------------------
String getUptime() {
  uint64_t us = esp_timer_get_time();
  uint64_t s = us / 1000000;

  uint32_t sec = s % 60;
  uint32_t min = (s / 60) % 60;
  uint32_t hr = (s / 3600) % 24;
  uint32_t days = s / 86400;

  char buffer[32];
  sprintf(buffer, "%u:%02u:%02u:%02u", days, hr, min, sec);
  return String(buffer);
}

//------------------------------------------------
void setup() {
  Serial.begin(115200);

  Wire.setTimeOut(50);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!sht4.begin()) {
    Serial.println("❌ SHT45 não encontrado");
    while (1)
      ;
  }

  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);

  Serial.println("✅ Sensor SHT45 iniciado com sucesso");

  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 240000,
  };

  esp_task_wdt_add(NULL);

  connectWiFi();

  // MQTT
  client.setServer(mqtt_server, 1883);

  syncTime();

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                void *arg, uint8_t *data, size_t len) {});
  server.addHandler(&ws);

  server.on("/esp32/api/temperatura", HTTP_GET, [](AsyncWebServerRequest *request) {
    float temperatureCelsius, temperatureFahrenheit, humidity;

    if (tryReadSensor(temperatureCelsius, temperatureFahrenheit, humidity, false)) {

      String dateTime = getCurrentDateTime();
      String upTime = getUptime();
      String sensorIp = WiFi.localIP().toString();
      String sensorRssi = String(WiFi.RSSI());

      StaticJsonDocument<256> jsonDoc;

      jsonDoc["temperatura_celsius"] = temperatureCelsius;
      jsonDoc["temperatura_fahrenheit"] = temperatureFahrenheit;
      jsonDoc["umidade"] = humidity;
      jsonDoc["data_hora"] = dateTime;
      jsonDoc["uptime"] = upTime;
      jsonDoc["sensor_ip"] = sensorIp;
      jsonDoc["rssi"] = sensorRssi;

      String jsonString;
      serializeJson(jsonDoc, jsonString);

      AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(jsonDoc, *response);

      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

      request->send(response);
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "❌ Pagina nao encontrada...");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });

  server.on("/esp32/api/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleJSON(request);
  });

  server.begin();
}

//------------------------------------------------
void loop() {
  esp_task_wdt_reset();

  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();

  // MQTT
  if (!client.connected())
  reconnectMQTT();

  client.loop();

  delay(200);

  if (millis() - lastPrint > 2000) {
    lastPrint = millis();
    String json = generateJSON();
    // Serial.println(json);
    ws.textAll(json);
  }

    if (millis() - lastNtpSync > 3600000) {  // 1 hora
      if (WiFi.status() == WL_CONNECTED) {
        lastNtpSync = millis();
        sntp_restart();
      }
    }

  // MQTT
  if (millis() - lastMqttPublish > 60000) {
    lastMqttPublish = millis();
    publishSensorData();
  }

}

float toKB(uint32_t bytes) {
  return bytes / 1024.0;
}

String formatUptime(unsigned long seconds) {
  int d = seconds / 86400;
  int h = (seconds % 86400) / 3600;
  int m = (seconds % 3600) / 60;
  int s = seconds % 60;

  char buf[20];
  sprintf(buf, "%dd %02d:%02d:%02d", d, h, m, s);
  return String(buf);
}

String generateJSON() {

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  float heapTotal = toKB(ESP.getHeapSize());
  float heapFree = toKB(ESP.getFreeHeap());
  float heapMin = toKB(ESP.getMinFreeHeap());
  float heapUsedPercent = 100.0 - ((heapFree / heapTotal) * 100.0);

  UBaseType_t stackWords = uxTaskGetStackHighWaterMark(NULL);
  uint32_t stackBytes = stackWords * 4;

  StaticJsonDocument<384> doc;

  uint64_t us = esp_timer_get_time();
  doc["uptime"] = formatUptime(us / 1000000);
  doc["cpu_mhz"] = getCpuFrequencyMhz();
  doc["sdk"] = ESP.getSdkVersion();

  doc["chip_cores"] = chip_info.cores;
  doc["chip_revision"] = chip_info.revision;

  doc["flash_kb"] = toKB(ESP.getFlashChipSize());

  doc["heap_total_kb"] = heapTotal;
  doc["heap_free_kb"] = heapFree;
  doc["heap_min_kb"] = heapMin;
  doc["heap_used_percent"] = heapUsedPercent;

  doc["stack_free_bytes"] = stackBytes;

  doc["wifi_ip"] = WiFi.localIP().toString();
  doc["wifi_rssi"] = WiFi.RSSI();

  String output;
  serializeJson(doc, output);

  return output;
}

void handleJSON(AsyncWebServerRequest *request) {
  request->send(200, "application/json", generateJSON());
}

void handleRoot(AsyncWebServerRequest *request) {

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>ESP32 System Monitor - Real-Time Embedded Telemetry Dashboard</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
*{box-sizing:border-box;margin:0;padding:0}

body{
  font-family:Segoe UI, sans-serif;
  background:linear-gradient(135deg,#0f172a,#1e293b,#0f172a);
  color:white;
  padding:25px;
}

.title{text-align:center;margin-bottom:5px;}
.subtitle{text-align:center;color:#94a3b8;margin-bottom:10px;font-size:14px;letter-spacing:1px;}
.subtitle-emp{text-align:center;color:#94a3b8;margin-top:-10px;margin-bottom:30px;font-size:10px;letter-spacing:1px;}

.title h1{
  font-size:42px;
  font-weight:900;
  letter-spacing:1px;
  background:linear-gradient(90deg,#22c55e,#38bdf8,#a855f7,#f43f5e,#22c55e);
  background-size:300% 300%;
  -webkit-background-clip:text;
  -webkit-text-fill-color:transparent;
  animation:gradientMove 6s ease infinite;
  text-shadow:0 0 25px rgba(56,189,248,0.35);
}

@keyframes gradientMove{
  0%{background-position:0% 50%;}
  50%{background-position:100% 50%;}
  100%{background-position:0% 50%;}
}

.dashboard{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(180px,1fr));
  gap:14px;
  margin-bottom:30px;
}

.card{
  background:rgba(255,255,255,0.06);
  backdrop-filter:blur(12px);
  border-radius:14px;
  padding:12px;
  text-align:center;
}

.label{font-size:12px;color:#94a3b8;}
.value{font-size:16px;margin-top:4px;font-weight:600;}

.progress{
  height:6px;
  background:rgba(255,255,255,0.1);
  border-radius:6px;
  margin-top:6px;
  overflow:hidden;
}

.progress-fill{
  height:100%;
  width:0%;
  background:linear-gradient(90deg,#22c55e,#38bdf8);
  transition:0.4s ease;
}

.rssi-status{
  margin-top:6px;
  display:flex;
  align-items:center;
  justify-content:center;
  gap:6px;
  font-size:13px;
}

.rssi-dot{
  width:10px;
  height:10px;
  border-radius:50%;
}

.charts{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:25px;
}

@media(max-width:768px){
  .charts{grid-template-columns:1fr;}
}

.chart-card{
  background:rgba(255,255,255,0.06);
  border-radius:18px;
  padding:20px;
  height:350px;
}

canvas{width:100%!important;height:100%!important;}
</style>
</head>

<body>

<div class="title">
  <h1>ESP32 Hardware Monitor</h1>
</div>
<div class="subtitle">
  Painel de Telemetria Embarcada - Tempo Real
</div>
<div class="subtitle-emp">
  Captação de temperatura e umidade
</div>

<div class="dashboard">

  <div class="card"><div class="label">Uptime</div><div class="value" id="uptime"></div></div>
  <div class="card"><div class="label">CPU (MHz)</div><div class="value" id="cpu"></div></div>
  <div class="card"><div class="label">SDK</div><div class="value" id="sdk"></div></div>
  <div class="card"><div class="label">Cores</div><div class="value" id="cores"></div></div>
  <div class="card"><div class="label">Revision</div><div class="value" id="revision"></div></div>
  <div class="card"><div class="label">Flash (KB)</div><div class="value" id="flash"></div></div>

  <div class="card">
    <div class="label">Heap Used (%)</div>
    <div class="value" id="heapPercent"></div>
    <div class="progress"><div class="progress-fill" id="heapBar"></div></div>
  </div>

  <div class="card"><div class="label">Heap Total (KB)</div><div class="value" id="heapTotal"></div></div>
  <div class="card"><div class="label">Heap Free (KB)</div><div class="value" id="heapFree"></div></div>
  <div class="card"><div class="label">Heap Min (KB)</div><div class="value" id="heapMin"></div></div>

  <div class="card">
    <div class="label">WiFi RSSI</div>
    <div class="value" id="rssi"></div>
    <div class="progress"><div class="progress-fill" id="rssiBar"></div></div>
        <div class="rssi-status">
      <div class="rssi-dot" id="rssiDot"></div>
      <div id="rssiText"></div>
    </div>
  </div>

  <div class="card"><div class="label">IP</div><div class="value" id="ip"></div></div>

</div>

<div class="charts">
  <div class="chart-card"><canvas id="heapChart"></canvas></div>
  <div class="chart-card"><canvas id="wifiChart"></canvas></div>
</div>

<script>

let heapData=[], wifiData=[], labels=[];

const heapChart=new Chart(document.getElementById('heapChart'),{
  type:'line',
  data:{
    labels:labels,
    datasets:[{
      label:'Heap Usage (%)',
      data:heapData,
      borderColor:'#22c55e',
      fill:false,
      tension:0.3,
      borderWidth:4,
      pointRadius: 2,
      pointHoverRadius: 6
    }]
  },
  options:{responsive:true,maintainAspectRatio:false}
});

const wifiChart=new Chart(document.getElementById('wifiChart'),{
  type:'line',
  data:{
    labels:labels,
    datasets:[{
      label:'WiFi RSSI (dBm)',
      data:wifiData,
      borderColor:'#38bdf8',
      fill:false,
      tension:0.3,
      borderWidth:4,
      pointRadius: 2,
      pointHoverRadius: 6
    }]
  },
  options:{responsive:true,maintainAspectRatio:false}
});

function getRssiStatus(rssi){
  if(rssi >= -50) return {color:"#22c55e",text:"Excelente"};
  if(rssi >= -67) return {color:"#22c55e",text:"Bom"};
  if(rssi >= -75) return {color:"#facc15",text:"Regular"};
  if(rssi >= -85) return {color:"#fb923c",text:"Fraco"};
  return {color:"#ef4444",text:"Muito fraco"};
}

const ws = new WebSocket(`ws://${location.host}/ws`);

ws.onmessage = function(event){

  const j = JSON.parse(event.data);

  document.getElementById("uptime").innerText=j.uptime;
  document.getElementById("cpu").innerText=j.cpu_mhz;
  document.getElementById("sdk").innerText=j.sdk;
  document.getElementById("cores").innerText=j.chip_cores;
  document.getElementById("revision").innerText=j.chip_revision;
  document.getElementById("flash").innerText=j.flash_kb;

  document.getElementById("heapTotal").innerText=j.heap_total_kb;
  document.getElementById("heapFree").innerText=j.heap_free_kb;
  document.getElementById("heapMin").innerText=j.heap_min_kb;
  document.getElementById("heapPercent").innerText=j.heap_used_percent+"%";

  document.getElementById("rssi").innerText=j.wifi_rssi+" dBm";
  document.getElementById("ip").innerText=j.wifi_ip;

  document.getElementById("heapBar").style.width=j.heap_used_percent+"%";

  let rssiPercent=((j.wifi_rssi+100)/60)*100;
  rssiPercent=Math.max(0,Math.min(100,rssiPercent));
  document.getElementById("rssiBar").style.width=rssiPercent+"%";

  const status=getRssiStatus(j.wifi_rssi);
  document.getElementById("rssiDot").style.background=status.color;
  document.getElementById("rssiText").innerText=status.text;

  if(labels.length>30){
    labels.shift();
    heapData.shift();
    wifiData.shift();
  }

  labels.push("");
  heapData.push(j.heap_used_percent);
  wifiData.push(j.wifi_rssi);

  heapChart.update();
  wifiChart.update();
};

</script>

</body>
</html>
)rawliteral";

  request->send(200, "text/html", html);
}

void syncTime() {
  Serial.println("🕒 Iniciando sincronização NTP");

  setenv("TZ", "BRT3", 1);
  tzset();

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_setservername(1, "time.nist.gov");

  // sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

  sntp_set_sync_interval(3600000);
  sntp_init();

  struct tm timeinfo;
  int retry = 0;

  while (!getLocalTime(&timeinfo) && retry < 15) {
    Serial.print(".");
    delay(200);
    retry++;
  }

  if (retry < 15) {
    Serial.println("✅ Sincronização NTP executada");
  } else {
    Serial.println("");
    Serial.println("❌ Falha ao sincronizar horário");
  }
}

// -------------------
// Wi-Fi resiliente
// -------------------
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    Serial.println("❌ Falha ao configurar IP");

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP32WEBSERVER");
  WiFi.begin(ssid, password);

  Serial.println("⌛ Conectando-se à rede");
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - startAttemptTime > 30000) {
      Serial.println("❌ WiFi não conectou — Reiniciando ESP");
      ESP.restart();
    }
  }

  if (millis() - lastNtpSync > 3600000) {  // 1 hora
    lastNtpSync = millis();
    sntp_restart();
  }

  Serial.println("🌐 Conexão estabelecida IP: " + WiFi.localIP().toString());
  Serial.println("🛜 Hostname: " + String(WiFi.getHostname()));
}

// -------------------
// MQTT reconnect
// -------------------
void reconnectMQTT() {

  while (!client.connected()) {

    Serial.println("📡 Conectando ao MQTT...");

    String clientId = "ESP32-SHT45-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("✅ MQTT conectado ao broker: " + clientId);

    } else {

      Serial.print("❌ MQTT falhou rc=");
      Serial.println(client.state());

      delay(5000);
    }
  }
}

// -------------------
// Publish MQTT
// -------------------
void publishSensorData() {

  float temperatureCelsius, temperatureFahrenheit, humidity;

  if (tryReadSensor(temperatureCelsius, temperatureFahrenheit, humidity, true)) {

    String dateTime = getCurrentDateTime();
    String upTime = getUptime();
    String sensorIp = WiFi.localIP().toString();
    String sensorRssi = String(WiFi.RSSI());

    StaticJsonDocument<256> jsonDoc;

    jsonDoc["temperatura_celsius"] = temperatureCelsius;
    jsonDoc["temperatura_fahrenheit"] = temperatureFahrenheit;
    jsonDoc["umidade"] = humidity;
    jsonDoc["data_hora"] = dateTime;
    jsonDoc["uptime"] = upTime;
    jsonDoc["sensor_ip"] = sensorIp;
    jsonDoc["rssi"] = sensorRssi;

    String jsonString;

    serializeJson(jsonDoc, jsonString);

    client.publish(
      "sensores/esp32/sht45",
      jsonString.c_str()
    );

    for (int i = 0; i < 200; i++) Serial.print('-'); Serial.println();
    Serial.print("📤 MQTT SENT: "); Serial.println(jsonString);
    for (int i = 0; i < 200; i++) Serial.print('-'); Serial.println(); 
  }
}