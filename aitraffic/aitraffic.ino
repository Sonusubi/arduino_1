#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "Autobonics_4G";
const char* password = "autobonics@27";

// Create a WebServer object
WebServer server(80);

// Traffic light statuses
String road1Status = "Red Light";
String road2Status = "Red Light";
String road3Status = "Red Light";

// GPIO pins for traffic lights
const int road1Red = 14;
const int road1Yellow = 12;
const int road1Green = 13;

const int road2Red = 32;
const int road2Yellow = 33;
const int road2Green = 25;

const int road3Red = 26;
const int road3Yellow = 27;
const int road3Green = 23;

// API to receive updates
void handleUpdateLights() {
  if (server.hasArg("road1") && server.hasArg("road2") && server.hasArg("road3")) {
    road1Status = server.arg("road1");
    road2Status = server.arg("road2");
    road3Status = server.arg("road3");

    Serial.println("Updated Traffic Light Status:");
    Serial.println("Road 1: " + road1Status);
    Serial.println("Road 2: " + road2Status);
    Serial.println("Road 3: " + road3Status);

    // Actuate the lights based on the status
    updateTrafficLights();

    server.send(200, "text/plain", "Status Updated");
  } else {
    server.send(400, "text/plain", "Missing Parameters");
  }
}

void updateTrafficLights() {
  // Update traffic light for Road 1
  if (road1Status == "Red Light") {
    digitalWrite(road1Red, HIGH);
    digitalWrite(road1Yellow, LOW);
    digitalWrite(road1Green, LOW);
  } else if (road1Status == "Yellow Light") {
    digitalWrite(road1Red, LOW);
    digitalWrite(road1Yellow, HIGH);
    digitalWrite(road1Green, LOW);
  } else if (road1Status == "Green Light") {
    digitalWrite(road1Red, LOW);
    digitalWrite(road1Yellow, LOW);
    digitalWrite(road1Green, HIGH);
  }

  // Update traffic light for Road 2
  if (road2Status == "Red Light") {
    digitalWrite(road2Red, HIGH);
    digitalWrite(road2Yellow, LOW);
    digitalWrite(road2Green, LOW);
  } else if (road2Status == "Yellow Light") {
    digitalWrite(road2Red, LOW);
    digitalWrite(road2Yellow, HIGH);
    digitalWrite(road2Green, LOW);
  } else if (road2Status == "Green Light") {
    digitalWrite(road2Red, LOW);
    digitalWrite(road2Yellow, LOW);
    digitalWrite(road2Green, HIGH);
  }

  // Update traffic light for Road 3
  if (road3Status == "Red Light") {
    digitalWrite(road3Red, HIGH);
    digitalWrite(road3Yellow, LOW);
    digitalWrite(road3Green, LOW);
  } else if (road3Status == "Yellow Light") {
    digitalWrite(road3Red, LOW);
    digitalWrite(road3Yellow, HIGH);
    digitalWrite(road3Green, LOW);
  } else if (road3Status == "Green Light") {
    digitalWrite(road3Red, LOW);
    digitalWrite(road3Yellow, LOW);
    digitalWrite(road3Green, HIGH);
  }
}

void setup() {
  Serial.begin(115200);

  // Set up GPIO pins
  pinMode(road1Red, OUTPUT);
  pinMode(road1Yellow, OUTPUT);
  pinMode(road1Green, OUTPUT);

  pinMode(road2Red, OUTPUT);
  pinMode(road2Yellow, OUTPUT);
  pinMode(road2Green, OUTPUT);

  pinMode(road3Red, OUTPUT);
  pinMode(road3Yellow, OUTPUT);
  pinMode(road3Green, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up the API endpoint
  server.on("/update-lights", HTTP_POST, handleUpdateLights);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
