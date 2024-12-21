#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
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
int totalSoc = 10000;

// Initial values
float initialSOC = 100.0;   // Start with 100% SOC
float previousSOC = 100.0;  // To keep track of previous SOC
float totalSOCDrop = 0.0;   // To accumulate the SOC drop
int chargeCycles = 0;       // To track the number of charge cycles
float SOH = 100.0;          // Start with 100% SOH (State of Health)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
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
  for (int i = 0; i < 101; i++) {
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
  lcd.begin();
  lcd.backlight(); // Turn on the backlight
  // Initialize variables from EEPROM
  totalSOCDrop = EEPROM.readFloat(SOC_ADDRESS);
  if (isnan(totalSOCDrop) || totalSOCDrop < 0) {
    totalSOCDrop = 0.0; // Reset invalid values
  }

  SOH = EEPROM.readFloat(SOH_ADDRESS);
  if (isnan(SOH) || SOH < 0 || SOH > 100) {
    SOH = 100.0; // Reset invalid values
  }

  Serial.println("System initialized.");
}

void loop() {
  // Read the battery voltage
  float voltage = analogReadVoltage();

  // Validate voltage
  if (voltage < 11.80 ) {
    Serial.println("Battery not connected or voltage out of range.");
    lcd.clear();
    lcd.setCursor(0, 0); // Set cursor to column 0, row 0
    lcd.print("Battery not ");
    lcd.setCursor(0, 1); // Set cursor to column 0, row 0
    lcd.print("connected");
    delay(1000);
    return;
  }

  // Calculate SOC
  int soc = findSOC(voltage);

  // Update SOC and SOH if SOC drops
  if (soc < previousSOC) {
    float socDrop = previousSOC - soc;

    totalSOCDrop += socDrop;
    if (totalSOCDrop > totalSoc) {
      totalSOCDrop = totalSoc; // Cap totalSOCDrop to totalSoc
    }

    if (totalSoc > 0) {
      SOH = ((totalSoc - totalSOCDrop) / totalSoc) * 100;
    } else {
      SOH = 0; // Default to 0 if totalSoc is invalid
    }

    // Save updated values to EEPROM
    EEPROM.writeFloat(SOC_ADDRESS, totalSOCDrop);
    EEPROM.writeFloat(SOH_ADDRESS, SOH);
    EEPROM.commit();

    // Print results
    Serial.print("Battery Voltage: ");
    Serial.print(voltage, 2);
    Serial.print(" V, SOC: ");
    Serial.print(soc);
    Serial.print(" %, SOH: ");
    Serial.print(SOH, 2);
    Serial.println(" %");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SOC :");
    lcd.setCursor(7, 0); // Set cursor to column 0, row 0
    lcd.print(soc);
    lcd.setCursor(10,0);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("SOH :");
    lcd.setCursor(7, 1); // Set cursor to column 0, row 0
    lcd.print(SOH,2);
    lcd.setCursor(12,1);
    lcd.print("%");

    previousSOC = soc; // Update previousSOC
  }

  delay(1000); // Wait before next reading
}

float analogReadVoltage() {
  int adcValue = analogRead(voltagePin);
  float adcVoltage = (adcValue / float(ADC_MAX)) * VREF;
  float batteryVoltage = adcVoltage * (R1 + R2) / R2;
  return batteryVoltage;
}

