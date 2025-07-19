#include "arduino_secrets.h"
#include "thingProperties.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT11.h>

DHT11 dht(18);

// RELAYS
int moisture = 23;
int relay1 = 22;
int relay2 = 21;
int relay3 = 19;
int relay4 = 5;

// Start server on port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  delay(1500);

  pinMode(moisture, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  initProperties();

  // Connect to WiFi
  WiFi.begin("aasmi", "aasmi123");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.println("ESP32 IP address: ");
  Serial.println(WiFi.localIP());  // <== Print ESP32 IP

  // Setup Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Turn all relays off initially
  digitalWrite(relay1, 1);
  digitalWrite(relay2, 1);
  digitalWrite(relay3, 1);
  digitalWrite(relay4, 1);

  // Handle HTTP GET from Flask - human detected
  server.on("/human_detected", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Human detected via Flask. Turning on light and fan.");
    digitalWrite(relay1, 0);  // Fan ON
    digitalWrite(relay2, 0);  // Light ON
    fan = true;
    light = true;
    request->send(200, "text/plain", "Light and fan turned on.");
  });

  // Handle HTTP GET from Flask - no human detected
  server.on("/no_human", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("No human detected via Flask. Turning off light and fan.");
    digitalWrite(relay1, 1);  // Fan OFF
    digitalWrite(relay2, 1);  // Light OFF
    fan = false;
    light = false;
    request->send(200, "text/plain", "Light and fan turned off.");
  });

  server.begin();
}

void loop() {
  ArduinoCloud.update();

  if (digitalRead(moisture) == 1) {
    digitalWrite(relay4, 0); // On
  } else {
    digitalWrite(relay4, 1); // Off
  }

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

// Callback functions for IoT Cloud
void onFanChange() {
  if (fan == 1) {
    digitalWrite(relay1, 0);
    Serial.println("Fan ON");
  } else {
    digitalWrite(relay1, 1);
    Serial.println("Fan OFF");
  }
}

void onLightChange() {
  if (light == 1) {
    digitalWrite(relay2, 0);
    Serial.println("Light ON");
  } else {
    digitalWrite(relay2, 1);
    Serial.println("Light OFF");
  }
}

void onLatchChange() {
  if (latch == 1) {
    digitalWrite(relay3, 0);
  } else {
    digitalWrite(relay3, 1);
  }
}
