#include "mqtt_manager.h"
#include "config.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Static network client instances (scoped to this translation unit)
static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

// Non-blocking timer timestamps
static unsigned long lastReconnectAttempt = 0;
static unsigned long lastWiFiRetry = 0;

/**
 * @brief Translates PubSubClient integer state codes into human-readable strings.
 * @param state Integer state returned by PubSubClient::state()
 * @return Constant string descriptor of connection state
 */
static const char *mqttStateToString(int state) {
  switch (state) {
    case -4: return "CONNECTION_TIMEOUT";
    case -3: return "CONNECTION_LOST";
    case -2: return "CONNECT_FAILED";
    case -1: return "DISCONNECTED";
    case 0:  return "CONNECTED";
    case 1:  return "CONNECT_BAD_PROTOCOL";
    case 2:  return "CONNECT_BAD_CLIENT_ID";
    case 3:  return "CONNECT_UNAVAILABLE";
    case 4:  return "CONNECT_BAD_CREDENTIALS";
    case 5:  return "CONNECT_UNAUTHORIZED";
    default: return "UNKNOWN";
  }
}

/**
 * @brief Initiates non-blocking Wi-Fi station mode connection.
 * Does not spin in a blocking while loop, allowing the main loop to proceed.
 */
static void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  Serial.print("[WiFi] Connecting to SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

/**
 * @brief Handles inbound MQTT messages arriving on subscribed topics.
 * @param topic Inbound topic string
 * @param payload Raw byte array received from broker
 * @param length Byte length of payload
 */
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("[MQTT] Inbound message [");
  Serial.print(topic);
  Serial.print("]: ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Extension point for remote device commands (e.g., reset, calibration)
}

/**
 * @brief Attempts non-blocking connection to the configured MQTT broker.
 * Configures Last Will and Testament (LWT) on TOPIC_ONLINE to automatically
 * broadcast "offline" (retained) if connection drops ungracefully.
 * @return true if connection succeeded, false otherwise
 */
static bool reconnectMQTT() {
  Serial.printf("[MQTT] Connecting to %s:%d (Client ID: %s)...\n",
                MQTT_SERVER, MQTT_PORT, MQTT_CLIENT_ID);

  bool connected = false;

  // Authenticate with credentials if provided, or connect anonymously
  if (strlen(MQTT_USER) > 0) {
    connected = mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD,
                             TOPIC_ONLINE, 1, true, "offline");
  } else {
    connected = mqtt.connect(MQTT_CLIENT_ID,
                             TOPIC_ONLINE, 1, true, "offline");
  }

  if (connected) {
    Serial.println("[MQTT] Connected successfully.");
    // Publish live availability status (retained)
    mqtt.publish(TOPIC_ONLINE, "online", true);
    // Subscribe to inbound command hierarchy
    mqtt.subscribe(TOPIC_COMMAND);
    return true;
  }

  // Diagnostic logging on failure
  int state = mqtt.state();
  Serial.printf("[MQTT] Connection failed, rc=%d (%s)\n",
                state, mqttStateToString(state));
  return false;
}

/**
 * @brief Initializes network settings, broker host, and callback handler.
 */
void setupCommunication() {
  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512); // Buffer size for JSON payload transmission
}

/**
 * @brief Network loop service routine.
 * Must be called continuously in loop(). Handles:
 * - Tracking Wi-Fi status changes and logging assigned IP.
 * - Non-blocking Wi-Fi reconnect attempts every 10 seconds.
 * - Non-blocking MQTT reconnect attempts every 5 seconds.
 * - Servicing PubSubClient internal packet loop (keepalives, incoming frames).
 */
void communicationLoop() {
  static wl_status_t lastWifiStatus = WL_IDLE_STATUS;
  wl_status_t currentWifiStatus = WiFi.status();

  // Log network state transitions
  if (currentWifiStatus != lastWifiStatus) {
    if (currentWifiStatus == WL_CONNECTED) {
      Serial.print("[WiFi] Connected! IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.printf("[WiFi] Status changed: %d\n", currentWifiStatus);
    }
    lastWifiStatus = currentWifiStatus;
  }

  unsigned long now = millis();

  // If Wi-Fi is disconnected, execute non-blocking retry backoff
  if (currentWifiStatus != WL_CONNECTED) {
    if (now - lastWiFiRetry >= 10000) {
      lastWiFiRetry = now;
      Serial.println("[WiFi] Re-attempting connection...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    return;
  }

  // If Wi-Fi is active but MQTT is disconnected, attempt reconnection
  if (!mqtt.connected()) {
    if (now - lastReconnectAttempt >= 5000) {
      lastReconnectAttempt = now;
      reconnectMQTT();
    }
  } else {
    // Process inbound packets and transmit keepalive PINGREQ frames
    mqtt.loop();
  }
}

/**
 * @brief Queries current MQTT connection state.
 */
bool isMqttConnected() {
  return mqtt.connected();
}

/**
 * @brief Formats and publishes telemetry sensor payload to the MQTT broker.
 * @param distance Distance to water surface in cm
 * @param tds Dissolved solids reading in PPM
 * @param status Qualitative water health classification string
 */
void publishTelemetry(float distance, float tds, const char *status) {
  if (!mqtt.connected()) {
    return;
  }

  // Allocate fixed-size JSON document on stack (avoids heap fragmentation)
  JsonDocument doc;
  doc["distance_cm"] = serialized(String(distance, 1));
  doc["tds_ppm"] = serialized(String(tds, 1));
  doc["water_status"] = status;

  char payload[256];
  size_t len = serializeJson(doc, payload, sizeof(payload));

  // Publish un-retained (retained = false)
  bool ok = mqtt.publish(TOPIC_TELEMETRY, payload, false);
  if (!ok) {
    Serial.println("[MQTT] Failed to publish telemetry payload.");
  }
}
