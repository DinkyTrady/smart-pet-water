#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

// ============================================================================
// WIRELESS LOCAL AREA NETWORK (WLAN) CONFIGURATION
// ============================================================================
// Note: ESP32 hardware radio only supports 2.4 GHz IEEE 802.11 b/g/n bands.
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ============================================================================
// MQTT BROKER CONNECTION PARAMETERS
// ============================================================================
// Broker host IP address on local network, or domain of public test broker.
#define MQTT_SERVER "192.168.0.0"
#define MQTT_PORT 1883

// Optional credentials (leave empty strings for unauthenticated brokers)
#define MQTT_USER ""
#define MQTT_PASSWORD ""

// Unique client identifier registered with the broker
#define MQTT_CLIENT_ID "petfountain_esp32"

// ============================================================================
// MQTT TOPIC TAXONOMY
// ============================================================================
#define TOPIC_TELEMETRY "petfountain/telemetry"
#define TOPIC_STATUS "petfountain/status"
#define TOPIC_COMMAND "petfountain/command/#"
#define TOPIC_ONLINE "petfountain/device/online"

#endif // CONFIG_EXAMPLE_H
