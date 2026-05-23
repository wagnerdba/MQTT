package com.wrtecnologia.mqttlistener.entity;

import jakarta.persistence.*;

import java.sql.Timestamp;

@Entity
@Table(name = "sensor_data")
public class SensorData {

	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;

	@Column(name = "temperatura_celsius")
	private Double temperaturaCelsius;

	@Column(name = "temperatura_fahrenheit")
	private Double temperaturaFahrenheit;

	@Column(name = "umidade")
	private Double umidade;

	@Column(name = "data_hora")
	private Timestamp dataHora;

	@Column(name = "uptime")
	private String uptime;

	@Column(name = "sensor_ip")
	private String sensorIp;

	@Column(name = "rssi")
	private Integer rssi;

	public Long getId() {
		return id;
	}

	public Double getTemperaturaCelsius() {
		return temperaturaCelsius;
	}

	public void setTemperaturaCelsius(Double temperaturaCelsius) {
		this.temperaturaCelsius = temperaturaCelsius;
	}

	public Double getTemperaturaFahrenheit() {
		return temperaturaFahrenheit;
	}

	public void setTemperaturaFahrenheit(Double temperaturaFahrenheit) {
		this.temperaturaFahrenheit = temperaturaFahrenheit;
	}

	public Double getUmidade() {
		return umidade;
	}

	public void setUmidade(Double umidade) {
		this.umidade = umidade;
	}

	public Timestamp getDataHora() {
		return dataHora;
	}

	public void setDataHora(Timestamp dataHora) {
		this.dataHora = dataHora;
	}

	public String getUptime() {
		return uptime;
	}

	public void setUptime(String uptime) {
		this.uptime = uptime;
	}

	public String getSensorIp() {
		return sensorIp;
	}

	public void setSensorIp(String sensorIp) {
		this.sensorIp = sensorIp;
	}

	public Integer getRssi() {
		return rssi;
	}

	public void setRssi(Integer rssi) {
		this.rssi = rssi;
	}
}