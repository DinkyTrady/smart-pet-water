#include "Arduino.h"
#include "HardwareSerial.h"
#include "Wire.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal-ledc.h"
#include "esp32-hal.h"

#define TRIG_PIN 22
#define ECHO_PIN 23
#define TDS_PIN 34
#define BUZZER_PIN 21
#define LED_RED 19
#define LED_YELLOW 18
#define LED_GREEN 5

// Ambang batas (Threshold)
const float DISTANCE_EMPTY = 15.0; // Jarak ultrasonik saat air mau habis (cm)
const float TDS_DIRTY = 300.0;     // Batas TDS air mulai kotor (PPM)

// Buzzeer global set
const int BUZZER_CHANNEL = 0;
const int BUZZER_FREQ = 2000;
const int BUZZER_BIT = 8;

// Offset kalibrasi TDS (sesuaikan jika perlu)
float tdsOffset = 0.0;


void bunyikanBuzzer(int jumlahBip, int durasiMs);
void nyalakanLed(char tipeLed);

void setup() {
  Serial.begin(115200);
  Wire.begin(26, 25);
  // Wire.begin()

  // Konfigurasi Pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);
  // LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Konfigurasi Buzzer
  // pinMode(BUZZER_PIN, OUTPUT); // not used since we choose the ledc approach
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_BIT);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);

  // Konfigurasi ADC ESP32
  analogReadResolution(12);       // Resolusi 12-bit (0-4095)
  analogSetAttenuation(ADC_11db); // Pengaturan rentang baca 0 - 3.3V

  Serial.println("===============================================");
  Serial.println("  SMART PET WATER FOUNTAIN - MONITOR MONITOR  ");
  Serial.println("===============================================");
  delay(1000);
}

void loop() {
  // 1. BACA SENSOR ULTRASONIK (Jarak / Ketinggian Air)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const char *status;

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2.0; // Konversi ke cm

  // 2. BACA SENSOR TDS (Kejernihan Air)
  long analogSum = 0;
  int sampleCount = 30;

  for (int i = 0; i < sampleCount; i++) {
    analogSum += analogRead(TDS_PIN);
    delay(10);
  }
  int rawADC = (float)analogSum / sampleCount;
  float voltage = rawADC * (3.3 / 4095.0);

  // Rumus Konversi Tegangan ke TDS PPM
  float temperature = 25.0;
  float compensationCoefficent = 1.0 + 0.02 * (temperature - 25.0);
  float compensationVoltage = voltage;
  float tdsValue = (133.42 * compensationVoltage * compensationVoltage *
                        compensationVoltage -
                    255.86 * compensationVoltage * compensationVoltage +
                    857.39 * compensationVoltage) *
                   0.5;
  tdsValue -= tdsOffset;
  if (tdsValue < 0)
    tdsValue = 0;

  // 4. LOGIKA PERINGATAN (BUZZER ONLY)
  bool airHabis = (distance >= DISTANCE_EMPTY);
  bool airKotor = (tdsValue >= TDS_DIRTY);

  if (airHabis && airKotor) {
    // Serial.println("PERINGATAN! Air Habis & Kotor!");
    status = "Habis & Kotor";
    nyalakanLed('r');
    bunyikanBuzzer(3, 1000); // Bip cepat 3x
  } else if (airHabis) {
    status = "Air Mau Habis!";
    nyalakanLed('y');
    bunyikanBuzzer(200, 1500); // Bip 2x
  } else if (airKotor) {
    status = "Air Kotor / Perlu Diganti!";
    nyalakanLed('r');
    bunyikanBuzzer(1, 3000); // Bip 1x agak panjang
  } else {
    status = "Air Aman & Bersih";
    nyalakanLed('g');
    digitalWrite(BUZZER_PIN, LOW);
  }

  // 3. TAMPILKAN DATA KE SERIAL MONITOR
  Serial.print("Jarak Air: ");
  Serial.print(distance, 1);
  Serial.print(" cm | ADC TDS: ");
  Serial.print(rawADC);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.print(" | Kejernihan Air: ");
  Serial.print(tdsValue, 1);
  Serial.print(" PPM -> Status: ");
  Serial.print(status);
  Serial.println();

  delay(1000); // Update data setiap 1 detik
}

// Fungsi pembantu untuk bunyi alarm buzzer
void bunyikanBuzzer(int durasiDelay, int durasiMs) {
  // tone(BUZZER_PIN, 2000);
  // delay(durasiMs);
  // noTone(BUZZER_PIN);
  // digitalWrite(BUZZER_PIN, LOW);
  // delay(durasiDelay);

  ledcWriteTone(BUZZER_CHANNEL, 2000);
  delay(durasiMs);

  ledcWriteTone(BUZZER_CHANNEL, 0);
  delay(durasiDelay);
}

void nyalakanLed(char tipeLed) {
  digitalWrite(LED_RED, tipeLed == 'r' ? HIGH : LOW);
  digitalWrite(LED_YELLOW, tipeLed == 'y' ? HIGH : LOW);
  digitalWrite(LED_GREEN, tipeLed == 'g' ? HIGH : LOW);
}
