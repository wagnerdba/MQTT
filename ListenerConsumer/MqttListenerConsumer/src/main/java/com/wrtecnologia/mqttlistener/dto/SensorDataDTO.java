package com.wrtecnologia.mqttlistener.dto;

public class SensorDataDTO {

	private Double temperaturaCelsius;
	private Double temperaturaFahrenheit;
	private Double umidade;
	private String dataHora;
	private String uptime;
	private String sensorIp;
	private Integer rssi;

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

	public String getDataHora() {
		return dataHora;
	}

	public void setDataHora(String dataHora) {
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