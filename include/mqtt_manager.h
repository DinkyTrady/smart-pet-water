#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

/**
 * @brief Initializes network hardware, targets, and buffer configurations.
 * Configures WiFi station mode and binds PubSubClient to the broker host/port.
 */
void setupCommunication();

/**
 * @brief Drives the non-blocking networking state machine.
 * Monitors WiFi association, executes periodic reconnection attempts, and
 * services the MQTT socket loop to sustain keepalive handshakes.
 */
void communicationLoop();

/**
 * @brief Checks if the client has an active authenticated broker session.
 * @return true if connected to broker, false otherwise.
 */
bool isMqttConnected();

/**
 * @brief Formats and publishes real-time sensor metrics as a JSON document.
 * @param distance Distance in centimeters from sensor to water surface.
 * @param tds Dissolved solids measurement in parts-per-million (PPM).
 * @param status Qualitative classification of water condition.
 */
void publishTelemetry(float distance, float tds, const char *status);

#endif // MQTT_MANAGER_H
