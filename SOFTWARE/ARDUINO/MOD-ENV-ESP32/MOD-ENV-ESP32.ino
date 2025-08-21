// -----------------------------------------------------------------------------
// Demo project for Olimex ESP32-POE2 and Olimex MOD-ENV
// Uses Adafruit libraries for BME280 and CCS811 sensors.
// Can be adapted for other boards by changing the I2C pins in Wire.begin().
// -----------------------------------------------------------------------------
//
// Required Libraries (install via Arduino Library Manager):
//   - Adafruit BME280 Library
//   - Adafruit CCS811 Library
//   - Adafruit Unified Sensor
// -----------------------------------------------------------------------------

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"
#include "Adafruit_CCS811.h"

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------
#define SEALEVELPRESSURE_HPA  1013.25   // Required for MOD-BME280 altitude

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
Adafruit_BME280 bme;
Adafruit_CCS811 ccs;

// -----------------------------------------------------------------------------
// Print BME280 values
// -----------------------------------------------------------------------------
void print_BME_Values()
{
  char buff[256];

  float temperature = bme.readTemperature();
  float pressure    = bme.readPressure() / 100.0F;
  float altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity    = bme.readHumidity();

  sprintf(buff,
          "BME280 Sensor Readings:\n"
          "  Temperature = %7.2f °C\n"
          "  Pressure    = %7.2f hPa\n"
          "  Altitude    = %7.2f m\n"
          "  Humidity    = %7.2f %%\n",
          temperature, pressure, altitude, humidity);

  Serial.print(buff);
}

// -----------------------------------------------------------------------------
// Print CCS811 values
// -----------------------------------------------------------------------------
void print_CCS811_Values()
{
  char buff[256];

  if (ccs.available())
  {
    if (!ccs.readData())
    {
      float co2  = (float)ccs.geteCO2();
      float tvoc = (float)ccs.getTVOC();

      sprintf(buff,
              "CCS811 Sensor Readings:\n"
              "  CO2         = %7.2f ppm\n"
              "  TVOC        = %7.2f ppb\n",
              co2, tvoc);

      Serial.print(buff);
    }
    else
    {
      Serial.println("CCS811 Sensor Readings: ERROR!");
    }
  }
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);        // Initialize serial communication
  Wire.begin(13, 33);          // Initialize I2C and pins. For ESP32-POE2: SDA=13, SCL=33
                               // Other Olimex ESP32 boards: usually SDA=13, SCL=16

  if (!bme.begin(0x77))        // I2C address of BME280 sensor
  {
    Serial.println("Error: Could not find BME280 sensor!");
    while (1);
  }

  if (!ccs.begin(0x5A))        // I2C address of CCS811 sensor
  {
    Serial.println("Error: Could not find CCS811 sensor!");
    while (1);
  }

  Serial.println("Sensors initialized successfully.\n");
}

// -----------------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------------
void loop()
{
  print_BME_Values();
  print_CCS811_Values();

  Serial.println();
  delay(1000);
}
