# Smart Pet Water Fountain Monitoring System

An ESP32-based IoT monitoring system for pet water fountains. Tracks water availability using an ultrasonic distance
sensor and evaluates water cleanliness using an analog Total Dissolved Solids (TDS) sensor. Alerts are provided via
status LEDs, an active buzzer driven by LEDC PWM, and serial telemetry.

## Hardware Components

- **ESP32 DevKit V1** (30-pin / 36-pin)
- **HC-SR04** Ultrasonic Distance Sensor
- **Analog TDS Sensor** (e.g., Gravity Analog TDS Sensor)
- **Status LEDs**: Red, Yellow, Green (with 220Ω - 330Ω resistors)
- **Passive/Active Buzzer**
- 5V / 3.3V power source and breadboard wiring

## Pin Mapping

| Component    | Pin Function           | ESP32 GPIO   | Notes                                                      |
| ------------ | ---------------------- | ------------ | ---------------------------------------------------------- |
| HC-SR04      | Trigger (`TRIG_PIN`)   | GPIO 22      | Digital Output                                             |
| HC-SR04      | Echo (`ECHO_PIN`)      | GPIO 23      | Digital Input (voltage divider recommended for 5V -> 3.3V) |
| TDS Sensor   | Analog Out (`TDS_PIN`) | GPIO 34      | ADC1 input, 12-bit resolution (0–4095), 11dB attenuation   |
| Status LED   | Red (`LED_RED`)        | GPIO 19      | Alert indicator                                            |
| Status LED   | Yellow (`LED_YELLOW`)  | GPIO 18      | Low-water warning indicator                                |
| Status LED   | Green (`LED_GREEN`)    | GPIO 5       | Normal operation indicator                                 |
| Buzzer       | PWM Pin (`BUZZER_PIN`) | GPIO 21      | LEDC Channel 0, 2000 Hz, 8-bit resolution                  |

## System Logic and Alert Thresholds

- **Distance Threshold (`DISTANCE_EMPTY`)**: `>= 15.0 cm` (indicates water reservoir is low/depleted).
- **TDS Threshold (`TDS_DIRTY`)**: `>= 300.0 PPM` (indicates water contamination requiring replacement).

### Alert States

| Water Level        | Water Quality        | State Label                  | LED State       | Buzzer Pattern     |
| ------------------ | -------------------- | ---------------------------- | --------------- | ------------------ |
| `< 15.0 cm` (Full) | `< 300 PPM` (Clean)  | `Air Aman & Bersih`          | Green (`HIGH`)  | Off                |
| `>= 15.0 cm` (Low) | `< 300 PPM` (Clean)  | `Air Mau Habis!`             | Yellow (`HIGH`) | Intermittent alert |
| `< 15.0 cm` (Full) | `>= 300 PPM` (Dirty) | `Air Kotor / Perlu Diganti!` | Red (`HIGH`)    | Single long beep   |
| `>= 15.0 cm` (Low) | `>= 300 PPM` (Dirty) | `Habis & Kotor`              | Red (`HIGH`)    | 3 rapid beeps      |

## Software and Dependencies

Built with [PlatformIO](https://platformio.org/) using the Arduino framework for ESP32.

### Prerequisites

1. Platformio CLI
   Install it on your device you can use pip or other (i recomended [uv](https://docs.astral.sh/uv/getting-started/installation/#standalone-installer)) or you can read more about this in official docs [platformio installation docs](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html)
2. Any Text Editor or IDE
   - **VSCode**, you need to install the platformio extension
3. USB-to-UART Drivers
   you need install this driver on you system the esp32 (CP2102 or CH304) not automatically recognized
4. Serial permissions (Linux Only)
   ```bash
   sudo usermod -a -G dialout $USER
   ```

Required libraries specified in `platformio.ini`:

- `DFRobot/GravityTDS`

## Build and Flash

1. Clone the repository:

   ```bash
   git clone https://github.com/DinkyTrady/smart-pet-water
   cd smart-pet-water
   ```

2. Compile commands for completion (Only for development)

   ```bash
   pio run -t compiledb
   ```

3. Compile firmware:
   pio run

4. Upload firmware to ESP32:
   pio run --target upload

5. Open Serial Monitor (115200 baud):
   pio device monitor -b 115200
