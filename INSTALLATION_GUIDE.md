# Smart Home Installation Guide

Complete guide to install and configure your Arduino Smart Home system with all advanced features.

---

## 📋 Table of Contents

1. [Hardware Requirements](#hardware-requirements)
2. [Software Requirements](#software-requirements)
3. [Hardware Setup](#hardware-setup)
4. [Software Installation](#software-installation)
5. [Configuration](#configuration)
6. [First Boot](#first-boot)
7. [Troubleshooting](#troubleshooting)

---

## 🔧 Hardware Requirements

### Essential Components

| Component | Model | Quantity | Purpose | Est. Cost |
|-----------|-------|----------|---------|-----------|
| **Microcontroller** | Arduino Uno R3 | 1 | Main controller | $25 |
| **Ethernet Shield** | Arduino Ethernet Shield (W5100) | 1 | Network connectivity | $15 |
| **SD Card** | 2-32GB microSD (FAT16/FAT32) | 1 | Store web files & logs | $5 |
| **Temperature Sensor** | NTC Thermistor 10K | 1 | Temperature monitoring | $1 |
| **Relay Module** | 5V 4/5-Channel Relay | 1 | Control appliances | $10 |
| **Power Supply** | 9V 1A DC adapter | 1 | Power Arduino | $5 |
| **Resistor** | 10K Ohm (for thermistor) | 1 | Temperature circuit | $0.10 |

**Total Cost: ~$61**

### Optional Components (For Advanced Features)

| Component | Model | Purpose | Est. Cost |
|-----------|-------|---------|-----------|
| **RTC Module** | DS3231 or DS1307 | Time-based scheduling | $3 |
| **PIR Sensor** | HC-SR501 | Motion detection | $2 |
| **Light Sensor** | BH1750 or LDR | Ambient light detection | $2 |
| **Current Sensor** | ACS712 (5A/20A/30A) | Power monitoring | $5 each |
| **Buzzer** | Active buzzer 5V | Alerts & notifications | $0.50 |

---

## 💻 Software Requirements

### Arduino IDE

1. **Download Arduino IDE:**
   - Visit: https://www.arduino.cc/en/software
   - Download version 1.8.19 or newer (or Arduino IDE 2.x)
   - Install on your computer (Windows/Mac/Linux)

### Required Libraries

Install these libraries via Arduino Library Manager (`Sketch > Include Library > Manage Libraries`):

| Library | Version | Purpose |
|---------|---------|---------|
| **Ethernet** | Built-in | Ethernet shield support |
| **SD** | Built-in | SD card file operations |
| **SPI** | Built-in | SPI communication |
| **Thermistor** | Custom | Temperature reading |

### Optional Libraries

| Library | Purpose | Installation |
|---------|---------|--------------|
| **RTClib** | Real-time clock support | Library Manager |
| **Time** | Time management | Library Manager |

### Custom Thermistor Library

If not available, create `Thermistor.h`:

```cpp
#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <Arduino.h>

class Thermistor {
public:
  Thermistor(uint8_t pin) : _pin(pin) {}

  uint8_t getTemp() {
    int reading = analogRead(_pin);
    // Convert ADC reading to temperature (simplified)
    // Adjust based on your thermistor specifications
    float resistance = 10000.0 * ((1024.0 / reading) - 1);
    float tempK = 1.0 / (1.0 / 298.15 + (1.0 / 3950.0) * log(resistance / 10000.0));
    float tempC = tempK - 273.15;
    return (uint8_t)tempC;
  }

private:
  uint8_t _pin;
};

#endif
```

---

## 🔌 Hardware Setup

### Step 1: Assemble the Components

#### Temperature Sensor Circuit

```
Thermistor Circuit:
┌─────────────┐
│             │
│   Arduino   │
│     A2 ○────┤ NTC Thermistor (10K)
│             ├── 10K Resistor ─── GND
│     5V ○────┘
│             │
└─────────────┘
```

**Connection:**
1. Connect thermistor between A2 and 5V
2. Connect 10K resistor between A2 and GND
3. This creates a voltage divider circuit

#### Relay Module

```
Relay Connections:
Arduino Pin → Relay Module
─────────────────────────
Pin 5       → IN1 (Living Room)
Pin 6       → IN2 (Master Bed)
Pin 9       → IN3 (Guest Room)
Pin 8       → IN4 (Kitchen)
Pin 7       → IN5 (Wash Room)
5V          → VCC
GND         → GND
```

**IMPORTANT:** Connect relay COM (common) to AC neutral, and NO (normally open) to your appliances.

⚠️ **SAFETY WARNING:**
- **DO NOT** work on mains voltage while powered
- **USE** proper insulation and enclosures
- **CONSULT** a licensed electrician for AC wiring
- **TEST** with low-voltage devices (LEDs) first

#### Ethernet Shield

```
Ethernet Shield Installation:
1. Stack Ethernet shield on top of Arduino Uno
2. Ensure all pins align correctly
3. Insert microSD card into shield's SD slot
4. Connect Ethernet cable to shield
```

**Pin Usage:**
- Pin 10: Ethernet CS (chip select)
- Pin 4: SD card CS
- Pins 11, 12, 13: SPI (MISO, MOSI, SCK)

### Step 2: Optional Components

#### RTC Module (DS3231)

```
RTC Connections:
DS3231 → Arduino
─────────────────
VCC    → 5V
GND    → GND
SDA    → A4
SCL    → A5
```

#### PIR Motion Sensor (Future Feature)

```
PIR Sensor:
HC-SR501 → Arduino
────────────────────
VCC      → 5V
GND      → GND
OUT      → Pin 3
```

---

## 📦 Software Installation

### Step 1: Prepare SD Card

1. **Format SD Card:**
   - Format as FAT16 (for cards ≤2GB) or FAT32 (for cards >2GB)
   - Use SD Card Formatter tool: https://www.sdcard.org/downloads/formatter/

2. **Create Directory Structure:**
   ```
   SD Card Root:
   ├── index.htm          (Web interface - copy from /web/index.htm)
   ├── data/              (Create empty folder)
   └── logs/              (Create empty folder)
   ```

3. **Upload Files:**
   - Copy `web/index.htm` to SD card root
   - Eject SD card safely

### Step 2: Upload Arduino Sketch

1. **Open Arduino IDE**

2. **Load the Sketch:**
   - Open `src/smart_home.ino`

3. **Add Custom Libraries:**
   - Copy `src/Config.h` to sketch folder
   - Copy `src/AutomationController.h` and `.cpp` to sketch folder
   - Copy `src/ScheduleController.h` and `.cpp` to sketch folder
   - Copy `src/SceneController.h` and `.cpp` to sketch folder

4. **Configure Network Settings:**
   ```cpp
   // In Config.h, adjust these values:
   #define DEFAULT_IP_0 192
   #define DEFAULT_IP_1 168
   #define DEFAULT_IP_2 0
   #define DEFAULT_IP_3 120  // Change to available IP on your network
   ```

5. **Verify Sketch:**
   - Click "Verify" button (✓)
   - Check for compilation errors
   - Fix any missing library errors

6. **Upload to Arduino:**
   - Connect Arduino via USB
   - Select correct board: `Tools > Board > Arduino Uno`
   - Select correct port: `Tools > Port > COM3` (or `/dev/ttyUSB0` on Linux)
   - Click "Upload" button (→)
   - Wait for upload to complete

---

## ⚙️ Configuration

### Network Configuration

#### Option 1: Static IP (Recommended)

Edit `src/smart_home.ino`:

```cpp
IPAddress ip(192, 168, 0, 120);  // Change to your network
Ethernet.begin(mac, ip);
```

#### Option 2: DHCP (Dynamic IP)

```cpp
if (Ethernet.begin(mac) == 0) {
  Serial.println(F("Failed to configure Ethernet using DHCP"));
  // Fallback to static IP
  Ethernet.begin(mac, ip);
}
```

**Find Arduino IP:**
```cpp
Serial.print(F("Server IP: "));
Serial.println(Ethernet.localIP());  // Check Serial Monitor
```

### RTC Configuration (If Installed)

1. **Uncomment RTC Code** in `src/ScheduleController.cpp`:

```cpp
// Change this:
// #include <RTClib.h>
// RTC_DS3231 rtc;

// To this:
#include <RTClib.h>
RTC_DS3231 rtc;
```

2. **Uncomment in `begin()` function:**

```cpp
if (rtc.begin()) {
  rtcAvailable = true;
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}
```

3. **Set Time via Serial:**

```cpp
// Add to setup():
// Set time: YYYY, MM, DD, HH, MM, SS
rtc.adjust(DateTime(2025, 11, 18, 14, 30, 0));
```

---

## 🚀 First Boot

### Step 1: Power On

1. Insert SD card into Ethernet shield
2. Connect Ethernet cable to your router
3. Connect Arduino to power supply
4. Wait 5-10 seconds for initialization

### Step 2: Monitor Serial Output

1. **Open Serial Monitor:**
   - Arduino IDE: `Tools > Serial Monitor`
   - Set baud rate to 9600

2. **Expected Output:**
   ```
   =====================================
   Arduino Smart Home v3.0
   =====================================
   Free SRAM: 1024 bytes
   SD card OK
   Relays initialized
   RTC not available
   Automation controller initialized
   Scene controller initialized with 7 scenes
   Server IP: 192.168.0.120
   System ready!
   =====================================
   ```

3. **Troubleshooting Messages:**
   - `ERROR: SD card initialization failed!` → Check SD card format and connections
   - `ERROR: index.htm not found!` → Upload index.htm to SD card
   - `RTC not found` → Normal if RTC not installed

### Step 3: Access Web Interface

1. **Find Arduino IP:**
   - Check Serial Monitor output
   - Or try default: `192.168.0.120`

2. **Open Web Browser:**
   - Navigate to: `http://192.168.0.120`
   - You should see the Smart Home Dashboard

3. **Test Basic Functions:**
   - Toggle a relay ON/OFF
   - Check temperature display
   - Try a scene (e.g., "All OFF")

---

## 🐛 Troubleshooting

### Problem: Can't Access Web Interface

**Symptoms:** Browser shows "Connection timed out" or "Cannot connect"

**Solutions:**
1. **Check Network Cable:**
   - Ensure Ethernet cable is firmly connected
   - Try different cable

2. **Verify IP Address:**
   - Check Serial Monitor for actual IP
   - Ping Arduino: `ping 192.168.0.120`

3. **Check Router:**
   - Ensure Arduino and computer on same network
   - Check for IP conflicts (another device using same IP)

4. **Firewall:**
   - Temporarily disable firewall
   - Add exception for port 80

### Problem: SD Card Errors

**Symptoms:** `ERROR: SD card initialization failed!`

**Solutions:**
1. **Reformat SD Card:**
   - Use SD Card Formatter
   - Format as FAT32
   - Try different SD card (some cards incompatible)

2. **Check Connections:**
   - Ensure SD card fully inserted
   - Try cleaning SD card contacts

3. **Check Pin 4:**
   - Pin 4 must not be used for other purposes
   - Verify no short circuits

### Problem: Temperature Shows Wrong Value

**Symptoms:** Temperature reads 0°C or 255°C or incorrect value

**Solutions:**
1. **Check Thermistor Connections:**
   - Verify wiring matches circuit diagram
   - Check 10K resistor present

2. **Calibrate Thermistor:**
   - Measure actual temperature with thermometer
   - Adjust formula in `Thermistor.h`

3. **Test Analog Read:**
   ```cpp
   Serial.println(analogRead(A2));  // Should be 400-600 at room temp
   ```

### Problem: Relays Not Switching

**Symptoms:** Web interface shows state change but relay doesn't click

**Solutions:**
1. **Check Relay Power:**
   - Verify 5V and GND connected to relay module
   - Measure voltage with multimeter (should be 5V)

2. **Check Signal Pins:**
   - Verify pin numbers match your relay module
   - Some relays are active-LOW (need to invert logic)

3. **Active-LOW Relays:**
   ```cpp
   // Change in setRelay() function:
   digitalWrite(pin, state ? LOW : HIGH);  // For active-LOW relays
   ```

4. **Check Relay Module:**
   - LED on relay should light when active
   - Listen for relay click sound
   - Test with external LED first

### Problem: Low Memory / Crashes

**Symptoms:** System freezes, random resets, corrupted data

**Solutions:**
1. **Check Free RAM:**
   - Should be >512 bytes free
   - Reduce `HTTP_BUFFER_SIZE` if needed

2. **Disable Features:**
   ```cpp
   #define ENABLE_SERIAL_DEBUG false  // Disable debug output
   ```

3. **Reduce Limits:**
   ```cpp
   constexpr uint8_t MAX_AUTOMATION_RULES = 3;  // Reduce from 5
   constexpr uint8_t MAX_SCHEDULES = 3;  // Reduce from 6
   ```

### Problem: Schedules Not Working

**Symptoms:** Scheduled events don't trigger

**Solutions:**
1. **Check RTC:**
   - RTC module must be installed
   - Uncomment RTC code in `ScheduleController.cpp`

2. **Verify Time:**
   ```cpp
   DateTime now = rtc.now();
   Serial.print(now.hour());
   Serial.print(":");
   Serial.println(now.minute());
   ```

3. **Check Schedule:**
   - Verify days of week match
   - Check ON/OFF times are correct

---

## 📚 Next Steps

After successful installation:

1. **Read USER_GUIDE.md** - Learn how to use all features
2. **Read API_REFERENCE.md** - API documentation for custom integrations
3. **Customize Scenes** - Create your own scene presets
4. **Set Up Automation** - Configure temperature-based automation
5. **Add Schedules** - Create time-based schedules (requires RTC)

---

## 🆘 Getting Help

If you encounter issues:

1. **Check Serial Monitor** - Most errors show here
2. **Search GitHub Issues** - Someone may have had the same problem
3. **Create Issue** - Include Serial Monitor output and photos
4. **Email** - carbonjha@gmail.com
5. **Twitter** - @JobayerArman

---

## ⚡ Quick Reference

### Default Configuration

| Setting | Value |
|---------|-------|
| IP Address | 192.168.0.120 |
| Port | 80 (HTTP) |
| Serial Baud | 9600 |
| SD Card Format | FAT16/FAT32 |

### Pin Usage

| Pin | Usage |
|-----|-------|
| 0-1 | Serial (USB) |
| 2 | Thermistor (Analog) |
| 3 | PIR Sensor (Optional) |
| 4 | SD Card CS |
| 5-9 | Relays 1-5 |
| 10 | Ethernet CS |
| 11-13 | SPI (MISO, MOSI, SCK) |
| A0 | Light Sensor (Optional) |
| A4-A5 | I2C (SDA, SCL) for RTC |

### LED Indicators

| LED | Meaning |
|-----|---------|
| Ethernet Shield RX/TX | Network activity |
| Relay Module LEDs | Relay state |
| Arduino Power LED | Arduino powered |
| Arduino Pin 13 LED | Arduino active |

---

**Congratulations!** Your Arduino Smart Home is now installed and ready to use! 🎉
