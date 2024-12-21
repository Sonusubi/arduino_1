#include <EEPROM.h>
#define EEPROM_SIZE 512
#define SOC_ADDRESS 0
#define SOH_ADDRESS 4

// Pin Definitions
const int batteryPin = 36;  // Analog input pin connected to the voltage divider
const float R1 = 29900.0;   // 30kΩ
const float R2 = 7400.0;    // 7.4kΩ
const int voltagePin = A0;
const float VREF = 3.3;
const int ADC_MAX = 4096;
int totalSoc=10000;

// Initial values
float initialSOC = 100.0;   // Start with 100% SOC
float previousSOC = 100.0;  // To keep track of previous SOC
float totalSOCDrop = 0.0;  // To accumulate the SOC drop
int chargeCycles = 0;      // To track the number of charge cycles
float SOH = 100.0;         // Start with 100% SOH (State of Health)

// Define the lookup table with voltage values for each 1% SOC step
const float voltageTable[101] = {
  12.80, 12.79, 12.78, 12.77, 12.76, 12.75, 12.74, 12.73, 12.72, 12.71,
  12.70, 12.69, 12.68, 12.67, 12.66, 12.65, 12.64, 12.63, 12.62, 12.61,
  12.60, 12.59, 12.58, 12.57, 12.56, 12.55, 12.54, 12.53, 12.52, 12.51,
  12.50, 12.49, 12.48, 12.47, 12.46, 12.45, 12.44, 12.43, 12.42, 12.41,
  12.40, 12.39, 12.38, 12.37, 12.36, 12.35, 12.34, 12.33, 12.32, 12.31,
  12.30, 12.29, 12.28, 12.27, 12.26, 12.25, 12.24, 12.23, 12.22, 12.21,
  12.20, 12.19, 12.18, 12.17, 12.16, 12.15, 12.14, 12.13, 12.12, 12.11,
  12.10, 12.09, 12.08, 12.07, 12.06, 12.05, 12.04, 12.03, 12.02, 12.01,
  12.00, 11.99, 11.98, 11.97, 11.96, 11.95, 11.94, 11.93, 11.92, 11.91,
  11.90, 11.89, 11.88, 11.87, 11.86, 11.85, 11.84, 11.83, 11.82, 11.81,
  11.80
};

// Function to find the SOC based on the measured voltage
int findSOC(float voltage) {
  // Find the closest matching voltage in the lookup table
  for (int i = 0; i < 101; i++) {
    // If the measured voltage is greater than or equal to the value in the table
    if (voltage >= voltageTable[i]) {
      return 100 - i;  // Return the corresponding SOC (100 - index)
    }
  }
  return 0;  // Return 0% SOC if voltage is lower than the minimum value
}

void setup() {
  Serial.begin(9600);  // Start serial communication
  pinMode(batteryPin, INPUT);  // Set the battery pin as input
  EEPROM.begin(EEPROM_SIZE);
  SOH = EEPROM.readFloat(SOH_ADDRESS);
  totalSOCDrop = EEPROM.readFloat(SOC_ADDRESS);
}

void loop() {
  // Read the battery voltage from the analog pin
  float voltage = analogReadVoltage();

  // Check if the battery is connected by verifying voltage is in a valid range
  if (voltage < 11.80 || voltage > 12.80) {
    // If voltage is outside the valid range, display a message and skip SOC calculation
    Serial.println("Battery not connected or voltage out of range.");
    delay(1000);  // Wait before next reading
    return;  // Skip further calculations if battery is disconnected or voltage is invalid
  }

  // Find the State of Charge (SOC) using the lookup table
  int soc = findSOC(voltage);

  // Check if SOC has dropped
  if (soc < previousSOC) {
    // Calculate the SOC drop
    float socDrop = previousSOC - soc;

    // Update total SOC drop
    totalSOCDrop += socDrop;
    EEPROM.writeFloat(SOC_ADDRESS, totalSOCDrop);
    // Update charge cycles based on SOC drop
    // Example: If SOC drops by more than 10%, increment charge cycle (you can adjust this threshold)
    // if (socDrop > 10) {
    //   chargeCycles++;
    // }

    // Update SOH (State of Health) based on SOC drop and charge cycles
    // For simplicity, decrease SOH by a small percentage for every charge cycle and SOC drop
    SOH = ((totalSoc-totalSOCDrop)/totalSoc)*100; // Adjust this formula as needed
   EEPROM.writeFloat(SOH_ADDRESS, SOH);
   EEPROM.writeFloat(SOC_ADDRESS, totalSOCDrop);
   EEPROM.commit();

    // Print the results to the Serial Monitor
    Serial.print("Battery Voltage: ");
    Serial.print(voltage, 2);  // Print the voltage with 2 decimal places
    Serial.print(" V, SOC: ");
    Serial.print(soc);  // Print the SOC
    Serial.print(" %, SOH: ");
    Serial.print(SOH);  // Print the SOH
    Serial.print(" %, Charge Cycles: ");
    Serial.println(chargeCycles);  // Print the number of charge cycles

    // Update previous SOC value
    previousSOC = soc;
  }

  // Delay for 1 second before the next reading
  delay(1000);
}

float analogReadVoltage() {
  // Read the raw ADC value from the voltage divider
  int adcValue = analogRead(voltagePin);

  // Convert ADC value to voltage
  float adcVoltage = (adcValue / float(ADC_MAX)) * VREF;

  // Calculate the actual battery voltage using the voltage divider formula
  float batteryVoltage = adcVoltage * (R1 + R2) / R2;

  return batteryVoltage;
}
