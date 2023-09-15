#include <Wire.h>
#include <Artekit_APDS9250.h>

Artekit_APDS9250 myApds9250;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) {}

  if (myApds9250.begin())
  {
    Serial.println("begin() OK");
  } else {
    Serial.println("begin() FAIL");
  }

  myApds9250.setMode(modeColorSensor);
  myApds9250.setResolution(res18bit);
  myApds9250.setGain(gain1);
  myApds9250.setMeasurementRate(rate100ms);
}

void loop()
{
  uint32_t red, green, blue, ir;
  myApds9250.getAll(&red, &green, &blue, &ir);
  Serial.print("red =");
  Serial.println(red);
  Serial.print("green =");
  Serial.println(green);
  Serial.print("blue =");
  Serial.println(blue);
  Serial.print("ir =");
  Serial.println(ir);
  Serial.println(" ");

  delay(1000);
}
