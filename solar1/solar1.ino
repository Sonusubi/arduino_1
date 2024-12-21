int max = 0, min = 0;
const int analogPin = A0;  // Pin connected to the potentiometer
const int motorPin1 = 9;   // Motor driver pin 1
const int motorPin2 = 10;  // Motor driver pin 2

const float R_min = 3.4;    // Minimum resistance value (3.4 ohms)
const float R_max = 30.6;   // Maximum resistance value (30.6 ohms)
const float threshold = 0.5;  // Dead zone threshold to prevent vibrations (adjust as needed)

float getResistanceFromSensor();
float lastResistance = 0;   // To store the last resistance value
float R_unknown = getResistanceFromSensor();

void setup() {
  Serial.begin(9600);

  // Set motor pins as output
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
}

void loop() {
  R_unknown = getResistanceFromSensor(); // Get the updated resistance

  Serial.print("Unknown Resistance: ");
  Serial.println(R_unknown);

  // Check for forward motion when resistance is within range and max is not set
  if (R_unknown < R_max && R_unknown >= R_min && max == 0) {
    moveForward();
    if (R_unknown >= R_max - threshold) {
      min = 0;
      max = 1;
    }
  }

  // Check for backward motion when resistance is within range and min is not set
  if (R_unknown > R_min && R_unknown <= R_max && min == 0) {
    moveBackward();
    if (R_unknown <= R_min + threshold) {
      min = 1;
      max = 0;
    }
  }

  // Add a small delay to avoid rapid switching
  delay(100);
}

// Function to move motor backward
void moveBackward() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
}

// Function to move motor forward
void moveForward() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
}

// Function to get resistance (use your existing method here)
float getResistanceFromSensor() {
  int rawValue = analogRead(analogPin); // Read potentiometer value
  float Vout = (rawValue / 1023.0) * 5.0;  // Convert ADC value to voltage
  const float R_known = 54.3;  // Known resistor value (adjust as needed)
  return R_known * (Vout / (5.0 - Vout));  // Calculate resistance
}
