#include <Wire.h>
#include <Adafruit_INA219.h>

// Voltage divider constants
const float R1 = 30000.0; // Resistor R1 value in ohms
const float R2 = 7500.0;  // Resistor R2 value in ohms
const int analogPin = 36; // Analog input pin
const float Vref = 3.3;   // Reference voltage of ESP32
const int ADC_resolution = 4095; // 12-bit ADC for ESP32

// Relay control pin
const int relayPin = 15; // GPIO pin connected to the relay module
const float voltageThreshold = 12.0; // Threshold voltage for relay control

// INA219 object
Adafruit_INA219 ina219;

// Function declarations
void currentDc();
void voltageDc();
void powerCalculation();
void controlRelay(float voltage);

float inputVoltage = 0.0; // Global variable for voltage
float current_mA = 0.0;   // Global variable for current

void setup() {
  Serial.begin(9600); // Initialize serial communication
  while (!Serial) {
    delay(1); // Wait for Serial to initialize
  }

  // Initialize the relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure relay is OFF at startup

  // Initialize the INA219 sensor
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  // Optional: Configure INA219 calibration range
  // ina219.setCalibration_32V_1A(); // For 32V, 1A range
  // ina219.setCalibration_16V_400mA(); // For 16V, 400mA range

  Serial.println("Measuring voltage, current, and power...");
}

void loop() {
  voltageDc();         // Measure and display voltage
  currentDc();         // Measure and display current
  powerCalculation();  // Calculate and display power
  controlRelay(inputVoltage); // Control relay based on voltage
  delay(1000);         // Delay for readability
}

// Function to measure current using INA219
void currentDc() {
  current_mA = ina219.getCurrent_mA(); // Get current in mA
  Serial.print("Current:       ");
  Serial.print(current_mA);
  Serial.println(" mA");
}

// Function to measure voltage using voltage divider
void voltageDc() {
  int sensorValue = analogRead(analogPin); // Read analog value (0-4095 for ESP32)
  float voltageAtPin = (sensorValue * Vref) / ADC_resolution; // Voltage at analog pin
  inputVoltage = voltageAtPin * ((R1 + R2) / R2); // Calculate input voltage

  Serial.print("Measured Voltage: ");
  Serial.print(inputVoltage);
  Serial.println(" V");
}

// Function to calculate and display power
void powerCalculation() {
  float current_A = current_mA / 1000.0; // Convert mA to A
  float power_W = inputVoltage * current_A; // Power in watts (P = V * I)

  Serial.print("Power:         ");
  Serial.print(power_W);
  Serial.println(" W");
}

// Function to control relay based on input voltage
void controlRelay(float voltage) {
  if (voltage > voltageThreshold) {
    digitalWrite(relayPin, HIGH); // Turn ON relay
    Serial.println("Relay ON: Voltage exceeded threshold");
  } else {
    digitalWrite(relayPin, LOW); // Turn OFF relay
    Serial.println("Relay OFF: Voltage below threshold");
  }
}
