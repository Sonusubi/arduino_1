#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Adafruit_INA219.h>
#include <Autobonics_Soc.h>
#define BUTTON_PIN 32         // Pin connected to the push button
#define VOLTAGE_PIN 36        // Pin to read battery voltage
#define BUTTON_ONE 39         // Secondary button for other functionalities
#define DEBOUNCE_DELAY 50     // Debounce delay for button press
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define EEPROM_SIZE 512
#define SOC_ADDRESS 0
#define SOH_ADDRESS 4
Adafruit_INA219 ina219(0x40);
// Voltage divider constants
const float R1 = 29900.0;   // Resistor R1 value (30kΩ)
const float R2 = 7400.0;    // Resistor R2 value (7.4kΩ)
const float VREF = 3.3;     // Reference voltage for ADC
const int ADC_MAX = 4096;   // ADC resolution (12-bit)
const int TOTAL_SOC = 10000;

// Initial values
float initialSOC = 100.0;   // Start with 100% SOC
float previousSOC = 100.0;  // Track previous SOC
float totalSOCDrop = 0.0;   // Accumulate the SOC drop
int chargeCycles = 0;       // Track number of charge cycles
float SOH = 100.0;          // Start with 100% SOH

volatile bool isLithiumProgram = true; // Toggle flag for battery type (use volatile for ISR)
bool buttonPressed = false;            // Button state to handle ISR logic in `loop()`
SOC_Library socLib;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Voltage-to-SOC lookup table
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

// Forward declarations
float analogReadVoltage();
void reset();
int findSOC(float voltage);

// ISR function
void IRAM_ATTR toggleProgram() {
    isLithiumProgram = !isLithiumProgram; // Toggle the program flag
    buttonPressed = true;                 // Mark button as pressed
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure button pin as input with pull-up
    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(BUTTON_ONE, INPUT);
    EEPROM.begin(EEPROM_SIZE);
    lcd.begin();
    lcd.backlight();

    // Attach the ISR to the button pin
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleProgram, FALLING);

    // Initialize variables from EEPROM
    totalSOCDrop = EEPROM.readFloat(SOC_ADDRESS);
    if (isnan(totalSOCDrop) || totalSOCDrop < 0) {
        totalSOCDrop = 0.0;
    }

    SOH = EEPROM.readFloat(SOH_ADDRESS);
    if (isnan(SOH) || SOH < 0 || SOH > 100) {
        SOH = 100.0;
    }
  Serial.println("Initializing INA219...");
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 at 0x41");
    while (1) { delay(10); } // Halt if INA219 is not found
  }
    Serial.println("System initialized.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  IOT ENABLED");
      lcd.setCursor(0, 1);
      lcd.print("      BMS");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  PRESS RED");
      lcd.setCursor(0, 1);
      lcd.print("     BUTTON");

}

void loop() {
    if (buttonPressed) {
        // Update the LCD display based on the selected program
        lcd.clear();
        lcd.setCursor(0, 0);
        if (isLithiumProgram) {
            lcd.print("Program: Lithium");
        } else {
            lcd.print("Program: Lead Acid");
        }
        buttonPressed = false; // Clear button state
    }

    if (isLithiumProgram) {
        // Lithium-ion battery program
           float batteryVoltage = socLib.readBatteryVoltage(VOLTAGE_PIN); // Read voltage from A0
           int soc = socLib.getSOC(batteryVoltage); // Get SOC
           bool status =socLib.getStatus(batteryVoltage);
           Serial.print("status :");
           Serial.print(status);
           if(status==0)
           {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Battery not");
            lcd.setCursor(0, 1);
            lcd.print("connected");
            delay(1000);
            return;           
           }
        else{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SOC: ");
        lcd.print(soc);
        lcd.print("%");
        lcd.setCursor(0, 1);
        lcd.print("SOH: ");
        lcd.print(soc, 2);
        lcd.print("%");
           }

    } else {
        // Lead-acid battery program
        float voltage = analogReadVoltage();
        if (voltage < 11.80) {
            Serial.println("Battery not connected or voltage out of range.");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Battery not");
            lcd.setCursor(0, 1);
            lcd.print("connected");
            delay(1000);
            return; // Exit the loop if the battery is not connected
        }
      int buttonState = digitalRead(BUTTON_ONE);


        // Calculate SOC
        int soc = findSOC(voltage);

        // Update SOC and SOH if SOC drops
        if (soc < previousSOC) {
            float socDrop = previousSOC - soc;
            totalSOCDrop += socDrop;

            // Cap totalSOCDrop to TOTAL_SOC
            if (totalSOCDrop > TOTAL_SOC) {
                totalSOCDrop = TOTAL_SOC;
            }

            if (TOTAL_SOC > 0) {
                SOH = ((TOTAL_SOC - totalSOCDrop) / TOTAL_SOC) * 100;
            } else {
                SOH = 0;
            }

            // Save updated values to EEPROM
            EEPROM.writeFloat(SOC_ADDRESS, totalSOCDrop);
            EEPROM.writeFloat(SOH_ADDRESS, SOH);
            EEPROM.commit();

            previousSOC = soc; // Update previous SOC
        }
        if (buttonState == HIGH) {
            reset();
        }
            // Display results on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SOC: ");
        lcd.print(soc);
        lcd.print("%");
        lcd.setCursor(0, 1);
        lcd.print("SOH: ");
        lcd.print(SOH, 2);
        lcd.print("%");

        Serial.print("Voltage: ");
        Serial.print(voltage, 2);
        Serial.print(" V, SOC: ");
        Serial.print(soc);
        Serial.print("%, SOH: ");
        Serial.print(SOH, 2);
        Serial.println("%");
        delay(2000);
        timeRemaining(7000,SOH);
    }
}

// Function to find SOC based on voltage
int findSOC(float voltage) {
    for (int i = 0; i < 101; i++) {
        if (voltage >= voltageTable[i]) {
            return 100 - i; // Return the corresponding SOC (100 - index)
        }
    }
    return 0; // Return 0% SOC if voltage is lower than minimum
}

// Function to read voltage
float analogReadVoltage() {
    const int numReadings = 100; // Number of readings to average
    long total = 0;             // Variable to accumulate readings

    // Take multiple readings and accumulate them
    for (int i = 0; i < numReadings; i++) {
        total += analogRead(VOLTAGE_PIN);
        delay(10); // Small delay between readings
    }

    // Calculate the average ADC value
    int adcValue = total / numReadings;

    // Convert the ADC value to voltage
    float adcVoltage = (adcValue / float(ADC_MAX)) * VREF;
    float batteryVoltage = adcVoltage * (R1 + R2) / R2;

    return batteryVoltage;
}

// Function to reset SOC and SOH
void reset() {
    totalSOCDrop = 0;
    SOH = 100.0;

    // Save reset values to EEPROM
    EEPROM.writeFloat(SOC_ADDRESS, totalSOCDrop);
    EEPROM.writeFloat(SOH_ADDRESS, SOH);
    EEPROM.commit();

    Serial.println("System reset.");
}
void timeRemaining( float totalCapacity,float soc)
{
  float batteryCapacity=(totalCapacity*(soc/100));
    float current = ina219.getCurrent_mA();
  if (current < 10) {
    current = 0;  // Ensure no negative current values
  }

  // Calculate remaining time in hours (battery capacity / current)
  if (current > 0) {
    float remainingTime = batteryCapacity / current;  // hours
    Serial.print("Current: "); Serial.print(current); Serial.println(" mA");
    Serial.print("Remaining Time: "); Serial.print(remainingTime); Serial.println(" hours");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Remaining Time:");
    lcd.setCursor(0, 1);
    lcd.print(remainingTime, 2);
    lcd.print("hr");
  } else {
    Serial.println("Current is too low or zero; cannot calculate remaining time.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Load not");
    lcd.setCursor(0, 1);
    lcd.print("connected");
  }

  delay(2000); // Wait 1 second before the next reading
}
