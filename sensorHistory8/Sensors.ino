



void activateMPU() {

  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    //  Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G); //turn off mpu by default
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);

  mpu.setIntFreeFallEnabled(false);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);

  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setMotionDetectionThreshold(10);
  mpu.setMotionDetectionDuration(10);

  mpu.setZeroMotionDetectionThreshold(10);
  mpu.setZeroMotionDetectionDuration(10);
  // mpu.setSleepEnabled(true);

}

void activateMAG() {
  mag.initialize(); //Initializes the mag sensor
  mag.start();      //Puts the sensor in active mode
}


