package com.wrtecnologia.mqttlistener.listener;

import com.wrtecnologia.mqttlistener.service.SensorDataService;
import org.junit.jupiter.api.Test;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.Message;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.Mockito.mock;

public class MqttListenerTest {

	@Test
	void shouldReceiveAndPrintMessage() {

		ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

		PrintStream customOut = new PrintStream(outputStream);

		PrintStream tee = new PrintStream(System.out) {

			@Override
			public void println(String x) {
				super.println(x);
				customOut.println(x);
			}
		};

		PrintStream originalOut = System.out;

		System.setOut(tee);

		try {

			SensorDataService sensorDataService = mock(SensorDataService.class);

			MqttListener listener = new MqttListener(sensorDataService);

			Message<String> message = MessageBuilder.withPayload("TEST MESSAGE").setHeader("mqtt_receivedTopic", "test/topic").build();

			listener.receive(message);

			String output = outputStream.toString();

			assertTrue(output.contains("🔍 MQTT RECEIVED - TOPIC: test/topic"));

			assertTrue(output.contains("📥 PAYLOAD: TEST MESSAGE"));

		} finally {
			System.setOut(originalOut);
		}
	}
}