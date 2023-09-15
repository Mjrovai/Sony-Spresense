/**
 * @file read_write.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief SD card read/write sample application.
 */
 
#include <Arduino.h>
#include <SDHCI.h>
#include <File.h>

SDClass SD; 
File myFile; 

void setup() {

  Serial.begin(115200);
  while (!Serial) {}

  /* Initialize SD */
  Serial.print("Insert SD card.");
  while (!SD.begin()) {}

  /* Create a new directory */
  SD.mkdir("dir/");

  /* Open the file. Note that only one file can be open at a time,
     so you have to close this one before opening another. */
  myFile = SD.open("dir/test.txt", FILE_WRITE);

  /* If the file opened okay, write to it */
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    /* Close the file */
    myFile.close();
    Serial.println("done.");
  } else {
    /* If the file didn't open, print an error */
    Serial.println("error opening test.txt");
  }

  /* Re-open the file for reading */
  myFile = SD.open("dir/test.txt");

  if (myFile) {
    Serial.println("test.txt:");

    /* Read from the file until there's nothing else in it */
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    /* Close the file */
    myFile.close();
  } else {
    /* If the file didn't open, print an error */
    Serial.println("error opening test.txt");
  }
}


void loop() {

}
