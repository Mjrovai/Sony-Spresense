/*
   LPS22HH_CommonSense_Teste.ino
   Based on: LPS22HH_DataLogTerminal.ino by Frederic Pillon <frederic.pillon@st.com>
   Testing The CommonSense LPS22HH Pressure sensor
   By: MJRovai @September, 2023
*/


// Includes
#include <LPS22HHSensor.h>
#define dev_interface       Wire

LPS22HHSensor PressTemp(&dev_interface);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Initialize bus interface
  dev_interface.begin();

  // Initlialize sensor
  PressTemp.begin();
  PressTemp.Enable();
}

void loop() {

  // Read pressure
  float pressure, temperature;
  PressTemp.GetPressure(&pressure);
  PressTemp.GetTemperature(&temperature);

  Serial.print("Pres[hPa]:");
  Serial.print(pressure, 2);
  Serial.print(", Temp[C]:");
  Serial.println(temperature, 2);

  delay (1000);
}
