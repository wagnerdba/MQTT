# ESP32 MQTT Telemetry Monitor

Sistema IoT para monitoramento de temperatura e umidade utilizando ESP32, MQTT (Message Queuing Telemetry Transport), Spring Boot e PostgreSQL.

---

# Arquitetura

```text
ESP32 ---> MQTT Broker ---> Spring Boot ---> PostgreSQL
```

O ESP32 realiza a leitura do sensor SHT45 e publica os dados via MQTT em intervalos de 1 minuto.

O backend Spring Boot atua como subscriber MQTT, recebendo as mensagens em tempo real e preparando os dados para persistência no banco PostgreSQL.

---

# Tecnologias utilizadas

## Embedded
- ESP32
- Sensor SHT45
- Arduino Framework
- PubSubClient
- ESPAsyncWebServer

## Backend
- Java 21
- Spring Boot
- Spring Integration MQTT
- Eclipse Paho MQTT

## Broker
- Mosquitto MQTT

## Banco
- PostgreSQL

---

# Funcionalidades

- Leitura de temperatura e umidade
- Publicação MQTT
- Dashboard Web embarcado no ESP32
- Telemetria em tempo real
- Reconexão automática Wi-Fi
- Reconexão automática MQTT
- Sincronização NTP
- Monitoramento de heap e RSSI
- Backend MQTT Subscriber
- Persistência em banco de dados

---

# Exemplo de payload MQTT

```json
{
  "temperatura_celsius": 27.5,
  "temperatura_fahrenheit": 81.5,
  "umidade": 56.7,
  "data_hora": "2026-05-21 10:56:56",
  "uptime": "0:00:42:02",
  "sensor_ip": "192.168.1.103",
  "rssi": "-41"
}
```

---

# Tópico MQTT

```text
sensores/esp32/sht45
```

---

# Objetivo

O projeto tem como objetivo criar uma arquitetura IoT leve, resiliente e escalável para monitoramento ambiental utilizando comunicação orientada a eventos via MQTT.