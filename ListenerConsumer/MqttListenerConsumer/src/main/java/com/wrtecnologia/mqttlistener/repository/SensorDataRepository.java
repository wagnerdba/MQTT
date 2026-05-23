package com.wrtecnologia.mqttlistener.repository;

import com.wrtecnologia.mqttlistener.entity.SensorData;
import org.springframework.data.jpa.repository.JpaRepository;

public interface SensorDataRepository extends JpaRepository<SensorData, Long> {

}
