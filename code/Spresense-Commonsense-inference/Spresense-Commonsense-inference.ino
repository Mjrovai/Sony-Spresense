/*
 "Home Smart Localization"
  - Estimating where you are in your home, using a SpreSense-CommonSensor board
   
   Sensors used:
   - VOC - SGP40
   - Temperature & Humidity - HTS221TR
   - Pressure - LPS22HH
   - Light - APDS9250
 * 
 * Inspired on the work by Shawn Hymel:
 * https://www.digikey.com/en/maker/projects/how-to-make-an-ai-powered-artificialnose/3fcf88a89efa47a1b231c5ad2097716a
 * 
 * Adapted by Marcelo Rovai from Shawn original code
 * @ September 13, 2023
 */

#include <Arduino.h>
#include <LPS22HHSensor.h>
#include <HTS221Sensor.h>
#include <SensirionI2CSgp40.h>
#include <Artekit_APDS9250.h>
#include <Wire.h>                              

// Edge Impulse library
#include <CommonSense-Sensor-Fusion-Preprocessed-data-v2_inferencing.h> 

// Definitions for LPS22HH
#define dev_interface       Wire
LPS22HHSensor PressTemp(&dev_interface);

// Definitions for HTS221
HTS221Sensor  HumTemp(&dev_interface);

// Definitions for SGP40
SensirionI2CSgp40 sgp40;

// Definitions for APDS9250
Artekit_APDS9250 myApds9250;

// Settings
#define DEBUG               1                         // 1 to print out debugging info
#define DEBUG_NN            false                     // Print out EI debugging info
#define ANOMALY_THRESHOLD   0.3                       // Scores above this are an "anomaly"
#define SAMPLING_FREQ_HZ    1                         // Inference sampling frequency (Hz)
#define SAMPLING_PERIOD_MS  1000 / SAMPLING_FREQ_HZ   // Inference sampling period (ms)
#define NUM_SAMPLES         EI_CLASSIFIER_RAW_SAMPLE_COUNT  // 3 sample at 0.1 Hz
#define READINGS_PER_SAMPLE EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME // 8
// above definitions come from model_metadata.h

// Constants

// Preprocessing constants (drop the timestamp column)
float mins[] = {
  895.85, 21.02, 30.2, 30066, 5, 4, 1, 4
};
float ranges[] = {
  2.27, 7.10, 10.7, 2133, 954, 1000, 511, 659
};

void setup() {

  Serial.begin(115200);
  while (!Serial) {}

  // Initialize I2C bus.
  dev_interface.begin();
  Wire.begin();

  // Initialize HTS221
  HumTemp.begin();
  HumTemp.Enable();

  // Initialize LPS22HH
  PressTemp.begin();
  PressTemp.Enable();

  // Initialize SGP40
  sgp40.begin(Wire);

  // Initialize APS9250
  myApds9250.begin();
  myApds9250.setMode(modeColorSensor);
  myApds9250.setResolution(res18bit);
  myApds9250.setGain(gain1);
  myApds9250.setMeasurementRate(rate100ms);

}

void loop() {
  
  float pressure, temp;
  float humidity;
  uint16_t defaultRh = 0x8000;
  uint16_t defaultT = 0x6666;
  uint16_t srawVoc = 0;
  uint32_t red, green, blue, ir;
  unsigned long timestamp;
  static float raw_buf[NUM_SAMPLES * READINGS_PER_SAMPLE];
  static signal_t signal;
  int max_idx = 0;
  float max_val = 0.0;
  char str_buf[40];
  
  // Collect samples and perform inference
  for (int i = 0; i < NUM_SAMPLES; i++) {

    // Take timestamp so we can hit our target frequency
    timestamp = millis();

    // read from sensors
    PressTemp.GetPressure(&pressure);
    PressTemp.GetTemperature(&temp);
    HumTemp.GetHumidity(&humidity);
    uint16_t error = sgp40.measureRawSignal(defaultRh, defaultT, srawVoc);
    myApds9250.getAll(&red, &green, &blue, &ir);

    // Store raw data into the buffer
    raw_buf[(i * READINGS_PER_SAMPLE) + 0] = pressure;
    raw_buf[(i * READINGS_PER_SAMPLE) + 1] = temp;
    raw_buf[(i * READINGS_PER_SAMPLE) + 2] = humidity;
    raw_buf[(i * READINGS_PER_SAMPLE) + 3] = srawVoc;
    raw_buf[(i * READINGS_PER_SAMPLE) + 4] = red;
    raw_buf[(i * READINGS_PER_SAMPLE) + 5] = green;
    raw_buf[(i * READINGS_PER_SAMPLE) + 6] = blue;
    raw_buf[(i * READINGS_PER_SAMPLE) + 7] = ir;

    // Perform preprocessing step (normalization) on all readings in the sample
    for (int j = 0; j < READINGS_PER_SAMPLE; j++) {
      temp = raw_buf[(i * READINGS_PER_SAMPLE) + j] - mins[j];
      raw_buf[(i * READINGS_PER_SAMPLE) + j] = temp / ranges[j];
    }

    // Wait just long enough for our sampling period
    while (millis() < timestamp + SAMPLING_PERIOD_MS);
  }

  // Print out our preprocessed, raw buffer
#if DEBUG
  for (int i = 0; i < NUM_SAMPLES * READINGS_PER_SAMPLE; i++) {
    Serial.print(raw_buf[i]);
    if (i < (NUM_SAMPLES * READINGS_PER_SAMPLE) - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();
#endif

  // Turn the raw buffer in a signal which we can the classify
  int err = numpy::signal_from_buffer(raw_buf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
      ei_printf("ERROR: Failed to create signal from buffer (%d)\r\n", err);
      return;
  }

  // Run inference
  ei_impulse_result_t result = {0};
  err = run_classifier(&signal, &result, DEBUG_NN);
  if (err != EI_IMPULSE_OK) {
      ei_printf("ERROR: Failed to run classifier (%d)\r\n", err);
      return;
  }

  // Print the predictions
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)\r\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    ei_printf("\t%s: %.3f\r\n", 
              result.classification[i].label, 
              result.classification[i].value);
  }

  // Print anomaly detection score
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("\tanomaly acore: %.3f\r\n", result.anomaly);
#endif

  // Find maximum prediction
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > max_val) {
      max_val = result.classification[i].value;
      max_idx = i;
    }
  }
}