package com.wrtecnologia.mqttlistener;

import com.wrtecnologia.mqttlistener.listener.MqttListener;
import org.junit.jupiter.api.Test;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.messaging.Message;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import static org.junit.jupiter.api.Assertions.assertTrue;

@SpringBootTest
class MqttListenerApplicationTests {

	@Test
	void contextLoads() {
	}


}
