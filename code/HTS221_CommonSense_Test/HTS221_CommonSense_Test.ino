/*
   HTS221_CommonSense_Teste.ino
   Testing The CommonSense LPS22HH Pressure sensor
   By: MJRovai @September, 2023
*/

// Includes.
#include <HTS221Sensor.h>

#define dev_interface       Wire

// Components.
HTS221Sensor  HumTemp(&dev_interface);

void setup() {
  // Initialize serial for output.
  Serial.begin(9600);

  // Initialize I2C bus.
  dev_interface.begin();

  // Initlialize components.
  HumTemp.begin();
  HumTemp.Enable();
}

void loop() {

  // Read humidity and temperature.
  float humidity, temperature;
  HumTemp.GetHumidity(&humidity);
  HumTemp.GetTemperature(&temperature);

  // Output data.
  Serial.print("Hum[%]: ");
  Serial.print(humidity, 2);
  Serial.print(" | Temp[C]: ");
  Serial.println(temperature, 2);

  delay(1000);
}
