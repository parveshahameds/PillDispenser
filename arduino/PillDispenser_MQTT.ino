#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// =========================
// Wi-Fi configuration
// =========================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// =========================
// HiveMQ Cloud configuration
// =========================
const char* mqtt_host = "YOUR_CLUSTER.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";

// =========================
// MQTT topics
// =========================
const char* topic_cmd = "pillbox/pillbox01/cmd";
const char* topic_status = "pillbox/pillbox01/status";

// =========================
// Hardware
// =========================
#define SERVO_PIN 18
#define BUZZER_PIN 19

Servo servoMotor;
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);

void publishStatus(const String& slot) {
  String json = "{\"dispensed\":\"" + slot + "\"}";
  mqtt.publish(topic_status, json.c_str());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT command: ");
  Serial.println(msg);

  int angle = 0;

  if (msg == "morning") {
    angle = 60;
  } else if (msg == "noon") {
    angle = 120;
  } else if (msg == "night") {
    angle = 180;
  } else {
    Serial.println("Unknown command. Ignoring.");
    return;
  }

  Serial.printf("Rotating servo to %d degrees...\n", angle);

  servoMotor.write(angle);
  delay(2000);

  servoMotor.write(0);

  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  publishStatus(msg);

  Serial.println("Dispensing action complete.");
}

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.println("Connecting to MQTT...");

    String clientId = "ESP32-PillBox-" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected.");

      if (mqtt.subscribe(topic_cmd)) {
        Serial.println("Subscribed to command topic.");
      } else {
        Serial.println("Failed to subscribe to command topic.");
      }
    } else {
      Serial.print("MQTT connection failed, state=");
      Serial.println(mqtt.state());
      Serial.println("Retrying in 3 seconds...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);

  connectWiFi();

  // Prototype configuration.
  // Production firmware should verify the HiveMQ CA certificate.
  espClient.setInsecure();

  mqtt.setServer(mqtt_host, mqtt_port);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }

  mqtt.loop();
}
