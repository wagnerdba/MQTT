package com.wrtecnologia.mqttlistener.listener;

import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;

@Service
public class MqttListener {

	@ServiceActivator(inputChannel = "mqttInputChannel")
	public void receive(Message<?> message) {

		System.out.println("📥 MQTT RECEIVED - TOPIC: " + message.getHeaders().get("mqtt_receivedTopic"));
		System.out.println("   PAYLOAD:" + message.getPayload());
		System.out.println();

	}
}