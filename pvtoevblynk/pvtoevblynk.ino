#include "SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h"

SFE_MAX1704X fuelGauge1(MAX1704X_MAX17043, 0x36);
SFE_MAX1704X fuelGauge2(MAX1704X_MAX17043, 0x32);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (fuelGauge1.begin(Wire)) {
    Serial.println("Fuel gauge 1 initialized!");
  } else {
    Serial.println("Fuel gauge 1 not detected.");
  }

  if (fuelGauge2.begin(Wire)) {
    Serial.println("Fuel gauge 2 initialized!");
  } else {
    Serial.println("Fuel gauge 2 not detected.");
  }
}

void loop() {
  Serial.print("Fuel gauge 1 voltage: ");
  Serial.println(fuelGauge1.getVoltage());

  Serial.print("Fuel gauge 2 voltage: ");
  Serial.println(fuelGauge2.getVoltage());

  delay(1000);
}
