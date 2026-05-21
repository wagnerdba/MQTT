package com.wrtecnologia.mqttlistener.config;

import org.junit.jupiter.api.Test;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.core.MessageProducer;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.messaging.MessageChannel;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.lang.reflect.Field;

import static org.junit.jupiter.api.Assertions.*;

class MqttConfigTest {

	@Test
	void shouldCreateMqttClientFactory() throws Exception {

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

			MqttConfig config = new MqttConfig();

			setField(config, "mqttUrl", "tcp://localhost:1883");

			MqttPahoClientFactory factory = config.mqttClientFactory();

			System.out.println("MQTT Client Factory criada");

			assertNotNull(factory);

			assertInstanceOf(DefaultMqttPahoClientFactory.class, factory);

			String output = outputStream.toString();

			assertTrue(output.contains("MQTT Client Factory criada"));

		} finally {
			System.setOut(originalOut);
		}
	}

	@Test
	void shouldCreateMqttInputChannel() {

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

			MqttConfig config = new MqttConfig();

			MessageChannel channel = config.mqttInputChannel();

			System.out.println("MQTT Input Channel criado");

			assertNotNull(channel);

			assertInstanceOf(DirectChannel.class, channel);

			String output = outputStream.toString();

			assertTrue(output.contains("MQTT Input Channel criado"));

		} finally {
			System.setOut(originalOut);
		}
	}

	@Test
	void shouldCreateInboundAdapter() throws Exception {

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

			MqttConfig config = new MqttConfig();

			setField(config, "mqttUrl", "tcp://localhost:1883");
			setField(config, "clientId", "test-client");
			setField(config, "topic", "test/topic");

			MqttPahoClientFactory factory = config.mqttClientFactory();

			MessageProducer producer = config.inbound(factory);

			System.out.println("Inbound MQTT Adapter criado");

			assertNotNull(producer);

			assertInstanceOf(MqttPahoMessageDrivenChannelAdapter.class, producer);

			MqttPahoMessageDrivenChannelAdapter adapter =
					(MqttPahoMessageDrivenChannelAdapter) producer;

			Field field = adapter.getClass().getSuperclass().getDeclaredField("completionTimeout");

			field.setAccessible(true);

			Long completionTimeout = (Long) field.get(adapter);

			assertEquals(5000L, completionTimeout);

			String output = outputStream.toString();

			assertTrue(output.contains("Inbound MQTT Adapter criado"));

		} finally {
			System.setOut(originalOut);
		}
	}

	private void setField(Object target, String fieldName, Object value) throws Exception {

		Field field = target.getClass().getDeclaredField(fieldName);

		field.setAccessible(true);

		field.set(target, value);
	}
}
