package com.wrtecnologia.mqttlistener;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.integration.config.EnableIntegration;

@SpringBootApplication
@EnableIntegration
public class MqttListenerApplication {

	public static void main(String[] args) {
		SpringApplication.run(MqttListenerApplication.class, args);
	}

}
