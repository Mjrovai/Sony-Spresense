#include <Arduino.h>
#include <SDHCI.h>
#include <File.h>

SDClass SD; 
File myFile;

#define FREQUENCY_HZ        1
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ))

static unsigned long last_interval_ms = 0;
static unsigned long sample_time = 0;

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;

void setup()
{
  Serial.begin(115200);
  while (!Serial) {}
  
   /* Initialize SD */
  Serial.print("Insert SD card.");
  if (!SD.begin(4)) {
    Serial.println("SD Error");
    return;
  }
  Serial.println("SD Started");

  if(!SD.exists("datalog.csv"))
  {
      myFile = SD.open("datalog.csv", FILE_WRITE);
      if (myFile) {
        myFile.println("Time,Sensor1,Sensor2,Sensor3");
        myFile.close();
      } else {

        Serial.println("Error creating datalog.csv");
      }
  }
}
static unsigned long ini_millis = millis();
void loop()
{
  if (millis() > last_interval_ms + INTERVAL_MS) {

  last_interval_ms = millis();
  sample_time = last_interval_ms - ini_millis;
  
    myFile = SD.open("datalog.csv", FILE_WRITE);
    
    if (myFile) { 

      updateValues ();
      Serial.print("Writing to datalog.csv");
      myFile.print(sample_time);
      myFile.print(",");
      myFile.print(sensor1);
      myFile.print(",");
      myFile.print(sensor2);
      myFile.print(",");
      myFile.println(sensor3);
      
      myFile.close();
      
      Serial.print("Time(ms)=");
      Serial.print(sample_time);
      Serial.print(", sensor1=");
      Serial.print(sensor1);
      Serial.print(", sensor2=");
      Serial.print(sensor2);
      Serial.print(", sensor3=");
      Serial.println(sensor3);       
                      
    } else {
      Serial.println("Error opening file");
    }
  }
}

void updateValues (){
  sensor1 = analogRead(0);
  sensor2 = analogRead(1);
  sensor3 = analogRead(2);
}