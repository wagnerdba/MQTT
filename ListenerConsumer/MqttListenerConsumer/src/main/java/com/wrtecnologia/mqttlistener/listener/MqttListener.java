package com.wrtecnologia.mqttlistener.listener;

import com.wrtecnologia.mqttlistener.service.SensorDataService;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;

@Service
public class MqttListener {

	private final SensorDataService sensorDataService;

	public MqttListener(SensorDataService sensorDataService) {
		this.sensorDataService = sensorDataService;
	}

	@ServiceActivator(inputChannel = "mqttInputChannel")
	public void receive(Message<?> message) {

		System.out.println();
		System.out.println("🔍 MQTT RECEIVED - TOPIC: " + message.getHeaders().get("mqtt_receivedTopic"));
		System.out.println("📥 PAYLOAD: " + message.getPayload());

		sensorDataService.processMessage(message.getPayload().toString());
	}
}