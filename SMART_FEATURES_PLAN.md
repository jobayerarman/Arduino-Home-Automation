# Smart Home Enhancement Plan

## Overview
Transform the Arduino home automation from simple relay control to an intelligent system with automation, scheduling, and analytics.

---

## 🎯 NEW FEATURES TO IMPLEMENT

### 1. **Temperature-Based Automation** (Climate Control)
**Use Case:** Automatically control heating/cooling based on temperature

**Features:**
- Set temperature thresholds (min/max)
- Automatic relay triggering when temperature crosses thresholds
- Configurable actions per relay (e.g., "Turn ON when temp > 28°C")
- Hysteresis to prevent relay chattering

**Example:**
```
If temperature > 28°C → Turn ON Fan (Relay 3)
If temperature < 20°C → Turn ON Heater (Relay 4)
If 20°C < temperature < 28°C → Turn OFF both
```

**API:**
```
GET /api/automation?relay=3&mode=auto&trigger=temp_high&threshold=28
GET /api/automation?relay=3&mode=manual
```

---

### 2. **Scheduling System** (Time-Based Control)
**Use Case:** Turn devices on/off at specific times

**Features:**
- Daily schedules (e.g., "Turn lights ON at 6:00 PM, OFF at 11:00 PM")
- Weekday/weekend schedules
- One-time timers (e.g., "Turn OFF in 30 minutes")
- Countdown timers visible on web interface

**Example:**
```
Schedule 1: Living Room Light ON at 18:00, OFF at 23:00 (Daily)
Schedule 2: Kitchen Light ON at 06:00, OFF at 08:00 (Weekdays)
Timer: Guest Room Fan OFF in 60 minutes
```

**API:**
```
GET /api/schedule?relay=1&on_hour=18&on_min=0&off_hour=23&off_min=0
GET /api/timer?relay=2&duration=60
GET /api/schedules (list all schedules)
```

---

### 3. **Scene Management** (Preset Configurations)
**Use Case:** One-click presets for common scenarios

**Scenes:**
- **Good Morning**: Turn ON Living Room + Kitchen lights, Turn OFF bedroom lights
- **Good Night**: Turn OFF all lights, Turn ON security mode
- **Away From Home**: Turn OFF all devices except security
- **Party Mode**: Turn ON all lights
- **Movie Night**: Dim lights, close blinds
- **Custom Scenes**: User-defined combinations

**Example:**
```json
{
  "scene": "good_night",
  "relays": {
    "living_room": false,
    "master_bed": false,
    "guest_room": false,
    "kitchen": false,
    "wash_room": true
  }
}
```

**API:**
```
GET /api/scene?name=good_night
GET /api/scene?name=good_morning
GET /api/scenes (list all scenes)
POST /api/scene/create (create custom scene)
```

---

### 4. **Data Logging & History** (Analytics)
**Use Case:** Track device usage and environmental data

**Features:**
- Log every relay state change with timestamp
- Log temperature readings every 5 minutes
- Store logs on SD card (CSV format)
- Query historical data via API
- Calculate usage statistics (uptime per relay)

**Log Format (logs/events.csv):**
```csv
timestamp,event_type,relay,state,temperature,trigger
2025-11-18 14:30:00,relay_change,living_room,ON,25,manual
2025-11-18 14:30:05,temp_reading,,,,25
2025-11-18 15:00:00,relay_change,guest_room,ON,28,automation
2025-11-18 15:00:00,automation_trigger,guest_room,ON,28,temp_high
```

**API:**
```
GET /api/logs?limit=100
GET /api/logs?relay=1&from=2025-11-18
GET /api/stats
```

---

### 5. **Energy & Usage Statistics** (Insights)
**Use Case:** Understand device usage patterns

**Metrics:**
- Total ON time per relay (today/week/month)
- Most used devices
- Temperature trends (min/max/average)
- Automation trigger count
- Manual override count

**Example Response:**
```json
{
  "stats": {
    "living_room": {
      "on_time_today": 240,      // minutes
      "on_time_week": 1680,
      "on_count_today": 5,
      "last_triggered": "2025-11-18 14:30:00"
    }
  },
  "temperature": {
    "current": 25,
    "min_today": 20,
    "max_today": 30,
    "avg_today": 25
  }
}
```

---

### 6. **Enhanced REST API** (Better Structure)
**Use Case:** Modern API for web/mobile apps

**Endpoints:**

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/relays` | GET | Get all relay states |
| `/api/relay/{id}` | GET/POST | Get/Set specific relay |
| `/api/temperature` | GET | Get current temperature |
| `/api/automation` | GET/POST | Get/Set automation rules |
| `/api/schedules` | GET/POST | Manage schedules |
| `/api/scenes` | GET | List all scenes |
| `/api/scene/{name}` | POST | Trigger scene |
| `/api/logs` | GET | Query event logs |
| `/api/stats` | GET | Get usage statistics |
| `/api/config` | GET/POST | System configuration |

---

### 7. **Manual Override System**
**Use Case:** Temporarily disable automation

**Features:**
- "Override" button on web interface
- Disables automation for specific relay
- Auto-resume after timeout or manual re-enable
- Visual indicator showing override status

---

### 8. **Real-Time Updates** (AJAX Polling)
**Use Case:** Web interface updates without refresh

**Features:**
- Poll server every 2 seconds for state changes
- Update temperature display in real-time
- Show countdown timers
- Highlight recently changed relays

---

### 9. **System Configuration** (Persistent Settings)
**Use Case:** Save user preferences

**Settings (stored in config.txt on SD card):**
```ini
[temperature]
unit=celsius
read_interval=60

[automation]
enabled=true
hysteresis=2

[logging]
enabled=true
interval=300

[network]
dhcp=false
ip=192.168.0.120
```

---

### 10. **Status Dashboard** (System Health)
**Use Case:** Monitor system health

**Metrics:**
- System uptime
- Free SRAM
- SD card status
- Network connectivity
- Active automation rules
- Active timers/schedules

---

## 🏗️ ARCHITECTURE DESIGN

### Memory Budget (Arduino Uno: 2048 bytes SRAM)

| Component | SRAM Usage | Notes |
|-----------|------------|-------|
| Ethernet Library | ~1024 bytes | Fixed overhead |
| Relay States | 5 bytes | bool array |
| Automation Rules | 100 bytes | 5 rules × 20 bytes |
| Schedule Storage | 120 bytes | 6 schedules × 20 bytes |
| Statistics | 80 bytes | Counters |
| HTTP Buffer | 60 bytes | Existing |
| Logging Buffer | 100 bytes | CSV line buffer |
| **Total** | **~1489 bytes** | **73% usage** |
| **Free** | **~559 bytes** | **27% margin** |

**Optimization Strategy:**
- Store configuration in Flash (PROGMEM)
- Use SD card for logs (not SRAM)
- Use uint16_t/uint8_t instead of int
- Minimize global variables

---

### Class Structure

```cpp
// Automation.h
class AutomationRule {
  uint8_t relayId;
  enum TriggerType { TEMP_HIGH, TEMP_LOW, SCHEDULED, TIMER };
  uint8_t threshold;
  bool enabled;
};

class AutomationController {
  AutomationRule rules[MAX_RULES];
  void evaluate();
  void addRule();
  void removeRule();
};

// Schedule.h
class Schedule {
  uint8_t relayId;
  uint8_t onHour, onMinute;
  uint8_t offHour, offMinute;
  uint8_t daysOfWeek;  // Bitmask: Mon=0x01, Tue=0x02, etc.
  bool enabled;
};

class ScheduleController {
  Schedule schedules[MAX_SCHEDULES];
  void checkSchedules();
  void addSchedule();
};

// Scene.h
struct Scene {
  const char* name;
  bool relayStates[RELAY_COUNT];
};

class SceneController {
  void activateScene(const char* name);
  const Scene* getScene(const char* name);
};

// Logger.h
class DataLogger {
  void logEvent(const char* event);
  void logTemperature(uint8_t temp);
  void logRelayChange(uint8_t relay, bool state);
};

// Statistics.h
class StatisticsTracker {
  uint16_t relayOnTime[RELAY_COUNT];
  uint16_t relayOnCount[RELAY_COUNT];
  uint8_t tempMin, tempMax;
  void update();
  void reset();
};
```

---

## 📁 FILE STRUCTURE

```
Arduino-Home-Automation/
├── src/
│   ├── smart_home.ino               // Main sketch
│   ├── Config.h                     // Pin definitions, constants
│   ├── RelayController.h/cpp        // Relay management
│   ├── AutomationController.h/cpp   // Automation logic
│   ├── ScheduleController.h/cpp     // Scheduling system
│   ├── SceneController.h/cpp        // Scene management
│   ├── DataLogger.h/cpp             // Event logging
│   ├── StatisticsTracker.h/cpp      // Usage statistics
│   ├── APIHandler.h/cpp             // REST API routing
│   ├── TimeManager.h/cpp            // RTC integration
│   └── Utils.h/cpp                  // Helper functions
├── web/
│   ├── index.htm                    // Main dashboard
│   ├── automation.htm               // Automation config page
│   ├── schedules.htm                // Schedule management
│   ├── scenes.htm                   // Scene editor
│   ├── logs.htm                     // Event logs viewer
│   └── stats.htm                    // Statistics dashboard
├── data/
│   ├── config.txt                   // System configuration
│   └── logs/
│       ├── events.csv               // Event log
│       └── temperature.csv          // Temperature history
└── docs/
    ├── API_REFERENCE.md             // API documentation
    └── USER_GUIDE.md                // User manual
```

---

## 🔧 REQUIRED HARDWARE ADDITIONS

### Essential:
1. **Real-Time Clock (RTC) Module** - DS3231 or DS1307
   - **Why:** Scheduling requires accurate timekeeping
   - **Cost:** ~$2-5
   - **Connections:** I2C (SDA, SCL)

### Optional Enhancements:
2. **PIR Motion Sensor** - HC-SR501
   - **Use:** Automatic lights when motion detected
   - **Cost:** ~$1-2
3. **Light Sensor** - LDR or BH1750
   - **Use:** Auto lights based on ambient light
   - **Cost:** ~$0.50-3
4. **Current Sensors** - ACS712
   - **Use:** Real power consumption monitoring
   - **Cost:** ~$2-5 each
5. **Buzzer/LED Indicators**
   - **Use:** Visual/audio alerts
   - **Cost:** ~$0.50

---

## 📊 IMPLEMENTATION PHASES

### Phase 1: Foundation (Week 1)
- ✅ Add RTC module support
- ✅ Implement time management
- ✅ Create data logging infrastructure
- ✅ Build statistics tracking

### Phase 2: Automation (Week 2)
- ✅ Temperature-based automation
- ✅ Scheduling system
- ✅ Manual override functionality
- ✅ Scene management

### Phase 3: API & Interface (Week 3)
- ✅ Enhanced REST API
- ✅ Updated web interface
- ✅ Real-time AJAX updates
- ✅ Configuration management

### Phase 4: Advanced Features (Week 4)
- ✅ Motion sensor integration
- ✅ Light sensor automation
- ✅ Energy monitoring (if current sensors added)
- ✅ Mobile-responsive UI

---

## 🎨 UI ENHANCEMENTS

### Dashboard Features:
```
┌─────────────────────────────────────────────┐
│  🏠 Smart Home Dashboard      🌡️ 25°C       │
├─────────────────────────────────────────────┤
│  Living Room    [●] ON    ⚡ 2h 30m today   │
│  Master Bed     [○] OFF   🤖 Auto: Disabled │
│  Guest Room     [●] ON    ⏱️ OFF in 45min   │
│  Kitchen        [○] OFF   📅 Schedule: 18:00 │
│  Wash Room      [○] OFF                     │
├─────────────────────────────────────────────┤
│  Quick Scenes:                              │
│  [Good Morning] [Good Night] [Away] [Party] │
├─────────────────────────────────────────────┤
│  Automation Status:                         │
│  🌡️ Fan auto-ON when temp > 28°C           │
│  ⏰ 3 active schedules                      │
│  📊 View Statistics →                       │
└─────────────────────────────────────────────┘
```

---

## 🚀 QUICK START IMPLEMENTATION

I'll now implement:
1. **smart_home.ino** - Enhanced main sketch with all features
2. **AutomationController.h/cpp** - Temperature automation
3. **ScheduleController.h/cpp** - Scheduling system
4. **SceneController.h/cpp** - Scene management
5. **DataLogger.h/cpp** - Event logging
6. **APIHandler.h/cpp** - Enhanced API
7. **index.htm** - Updated dashboard UI

Let's build this smart home! 🏡
