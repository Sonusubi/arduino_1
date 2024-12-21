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

    server.send(200, "text/plain", "Status Updated");
  } else {
    server.send(400, "text/plain", "Missing Parameters");
  }
}

void setup() {
  Serial.begin(115200);

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
