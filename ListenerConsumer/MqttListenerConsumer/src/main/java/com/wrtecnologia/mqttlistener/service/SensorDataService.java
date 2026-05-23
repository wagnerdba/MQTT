package com.wrtecnologia.mqttlistener.service;

import com.wrtecnologia.mqttlistener.dto.SensorDataDTO;
import com.wrtecnologia.mqttlistener.entity.SensorData;
import com.wrtecnologia.mqttlistener.repository.SensorDataRepository;
import org.springframework.stereotype.Service;
import tools.jackson.databind.JsonNode;
import tools.jackson.databind.ObjectMapper;

import java.sql.Timestamp;

@Service
public class SensorDataService {

	private final SensorDataRepository repository;

	private final ObjectMapper objectMapper = new ObjectMapper();

	public SensorDataService(SensorDataRepository repository) {
		this.repository = repository;
	}

	public void processMessage(String payload) {

		try {

			JsonNode json = objectMapper.readTree(payload);

			SensorDataDTO dto = new SensorDataDTO();

			dto.setTemperaturaCelsius(
					json.get("temperatura_celsius").asDouble()
			);

			dto.setTemperaturaFahrenheit(
					json.get("temperatura_fahrenheit").asDouble()
			);

			dto.setUmidade(
					json.get("umidade").asDouble()
			);

			dto.setDataHora(
					json.get("data_hora").asText()
			);

			dto.setUptime(
					json.get("uptime").asText()
			);

			dto.setSensorIp(
					json.get("sensor_ip").asText()
			);

			dto.setRssi(
					json.get("rssi").asInt()
			);

			SensorData entity = new SensorData();

			entity.setTemperaturaCelsius(dto.getTemperaturaCelsius());
			entity.setTemperaturaFahrenheit(dto.getTemperaturaFahrenheit());
			entity.setUmidade(dto.getUmidade());

			entity.setDataHora(
					Timestamp.valueOf(dto.getDataHora())
			);

			entity.setUptime(dto.getUptime());
			entity.setSensorIp(dto.getSensorIp());
			entity.setRssi(dto.getRssi());

			repository.save(entity);

			System.out.println("✅ DATA SAVED IN DATABASE");
			System.out.println();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}