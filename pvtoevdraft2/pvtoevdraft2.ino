#include "EmonLib.h"  // Include EmonLib for energy monitoring
#include <EEPROM.h>  // Include EEPROM library for storing data
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;
#define acVoltsensor 36
#define acCurrentsensor 39

#define relay 15
#define relay2 4
const int voltagePin = 34;  // Analog pin connected to the voltage divider
const float R1 = 30000.0;   // Resistor R1 value in ohms (10kΩ)
const float R2 = 7500.0;    // Resistor R2 value in ohms (3.3kΩ)
const float adcResolution = 4095.0;  // 12-bit ADC resolution (0-4095)
const float referenceVoltage = 3.3;  // ESP32 ADC reference voltage (3.3V)
float Vin;
// Constants for calibration
const float vCalibration = 70.5;  // Voltage calibration factor
const float currCalibration = 0.02;  // Current calibration factor
float power_mW;
// EnergyMonitor instance
EnergyMonitor emon;  // Create an instance of EnergyMonitor

// Variables for energy calculation
float kWh = 0.0;  // Variable to store energy consumed in kWh
float cost = 0.0;  // Variable to store cost of energy consumed
const float ratePerkWh = 6.5;  // Cost rate per kWh
unsigned long lastMillis = millis();  // Variable to store last time in milliseconds

// EEPROM addresses for each variable
const int addrKWh = 12;  // EEPROM address for kWh
const int addrCost = 16;  // EEPROM address for cost

// Reset button pin
const int resetButtonPin = 13;  // Pin for reset button (change to your button pin)

// Function prototypes
void readEnergyDataFromEEPROM();  // Prototype for reading energy data from EEPROM
void saveEnergyDataToEEPROM();  // Prototype for saving energy data to EEPROM
void resetEEPROM();  // Prototype for resetting EEPROM data
void dcVoltage();
void dcCurrent(float);
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);  // Start Serial communication at 115200 baud rate
  Serial.println("Smart Energy Meter Initialized");

  // Initialize EEPROM
  EEPROM.begin(32);  // Initialize EEPROM with 32 bytes of storage

  // Initialize the reset button pin
  pinMode(resetButtonPin, INPUT_PULLUP);  // Set reset button pin as input with pull-up resistor
  pinMode(relay,OUTPUT);
  pinMode(relay2,OUTPUT);
  // Read stored data from EEPROM
  readEnergyDataFromEEPROM();  // Read energy data from EEPROM

  // Setup voltage and current inputs
  emon.voltage(acVoltsensor, vCalibration, 1.7);  // Configure voltage measurement: input pin, calibration, phase shift
  emon.current(acCurrentsensor, currCalibration);  // Configure current measurement: input pin, calibration
 if (! ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    while (1) 
    {
      delay(10);
    }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range, call:
  ina219.setCalibration_16V_400mA();
 
  Serial.println("Measuring voltage, current, and power with INA219 ...");
}


void loop() {
  // Perform energy monitoring
  emon.calcVI(20, 2000);  // Calculate voltage and current: number of half wavelengths (crossings), time-out
  float Vrms = emon.Vrms;  // Get root mean square voltage
  float Irms = emon.Irms;  // Get root mean square current
  float apparentPower = emon.apparentPower;  // Get apparent power

  // Handle low voltage scenario
  if (Vrms < 15.0) {
    Vrms = 0.0;
    apparentPower = 0.0;
  }
  if (apparentPower>2.5)
  {
    digitalWrite(relay,HIGH);
  }
  else{
    digitalWrite(relay,LOW);
  }

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();  // Get current time in milliseconds
  kWh += apparentPower * (currentMillis - lastMillis) / 3600000000.0;  // Update kWh
  lastMillis = currentMillis;  // Update last time

  // Calculate the cost based on the rate per kWh
  cost = kWh * ratePerkWh;  // Calculate cost

  // Save the latest values to EEPROM
  saveEnergyDataToEEPROM();  // Save energy data to EEPROM
  dcVoltage();
  dcCurrent(Vin);
  Serial.println(power_mW);
    if ( power_mW>=0)
  {
    digitalWrite(relay2,HIGH);
  }
  else{
    digitalWrite(relay2,LOW);
  }
  // Print data to Serial Monitor
  Serial.print("Voltage (Vrms): ");
  Serial.print(Vrms);
  Serial.print(" V, Current (Irms): ");
  Serial.print(Irms);
  Serial.print(" A, Power: ");
  Serial.print(apparentPower);
  Serial.println(" W");

  Serial.print("Energy Consumed: ");
  Serial.print(kWh);
  Serial.print(" kWh, Cost: ");
  Serial.print(cost);
  Serial.println(" INR");

  // Check if the reset button is pressed
  if (digitalRead(resetButtonPin) == LOW) {  // If reset button is pressed (assuming button press connects to ground)
    delay(200);  // Debounce delay
    resetEEPROM();  // Reset EEPROM data
    Serial.println("EEPROM Data Reset!");
  }

  delay(20);  // Delay for 2 seconds before the next iteration
}

void readEnergyDataFromEEPROM() {
  EEPROM.get(addrKWh, kWh);  // Read kWh from EEPROM
  EEPROM.get(addrCost, cost);  // Read cost from EEPROM

  // Initialize to zero if values are invalid
  if (isnan(kWh)) {
    kWh = 0.0;  // Set kWh to 0 if invalid
    saveEnergyDataToEEPROM();  // Save to EEPROM
  }
  if (isnan(cost)) {
    cost = 0.0;  // Set cost to 0 if invalid
    saveEnergyDataToEEPROM();  // Save to EEPROM
  }
}

void saveEnergyDataToEEPROM() {
  EEPROM.put(addrKWh, kWh);  // Save kWh to EEPROM
  EEPROM.put(addrCost, cost);  // Save cost to EEPROM
  EEPROM.commit();  // Commit EEPROM changes
}

void resetEEPROM() {
  kWh = 0.0;  // Reset kWh to 0
  cost = 0.0;  // Reset cost to 0
  saveEnergyDataToEEPROM();  // Save to EEPROM
}
void dcVoltage()
{
 int rawADC = analogRead(voltagePin);

  // Convert raw ADC value to voltage
  float Vout = (rawADC / adcResolution) * referenceVoltage;

  // Calculate the input voltage using the voltage divider formula
   Vin = Vout * ((R1 + R2) / R2);

  // Print the measured voltage to the Serial Monitor
  Serial.print("Measured Voltage: ");
  Serial.print(Vin);
  Serial.println(" V");
}
void dcCurrent(float Vin)
{
    float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  power_mW = 0;
 
  // shuntvoltage = ina219.getShuntVoltage_mV();
  // busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = Vin*current_mA;
  loadvoltage = busvoltage + (shuntvoltage / 1000);
 
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

}