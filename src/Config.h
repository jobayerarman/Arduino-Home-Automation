/**
 * Config.h - Smart Home Configuration
 * Centralized configuration for all system constants
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==============================================================================
// SYSTEM CONFIGURATION
//==============================================================================

#define SYSTEM_VERSION "3.0"
#define SYSTEM_NAME "Arduino Smart Home"

//==============================================================================
// PIN DEFINITIONS
//==============================================================================

// SD Card & Ethernet
constexpr uint8_t SD_CHIP_SELECT_PIN = 4;
constexpr uint8_t ETHERNET_CS_PIN = 10;

// Temperature Sensor
constexpr uint8_t THERMISTOR_PIN = 2;

// Relay Pins
constexpr uint8_t RELAY_LIVING_ROOM_PIN = 5;
constexpr uint8_t RELAY_MASTER_BED_PIN = 6;
constexpr uint8_t RELAY_GUEST_ROOM_PIN = 9;
constexpr uint8_t RELAY_KITCHEN_PIN = 8;
constexpr uint8_t RELAY_WASH_ROOM_PIN = 7;

// Optional Sensors (for future expansion)
constexpr uint8_t PIR_SENSOR_PIN = 3;          // Motion sensor
constexpr uint8_t LIGHT_SENSOR_PIN = A0;       // LDR sensor
constexpr uint8_t BUZZER_PIN = A1;             // Alert buzzer

//==============================================================================
// RELAY CONFIGURATION
//==============================================================================

constexpr uint8_t RELAY_COUNT = 5;

// Relay IDs (enum for type safety)
enum RelayId : uint8_t {
  RELAY_LIVING_ROOM = 0,
  RELAY_MASTER_BED = 1,
  RELAY_GUEST_ROOM = 2,
  RELAY_KITCHEN = 3,
  RELAY_WASH_ROOM = 4
};

// Relay names (stored in Flash memory)
const char RELAY_NAME_0[] PROGMEM = "Living Room";
const char RELAY_NAME_1[] PROGMEM = "Master Bed";
const char RELAY_NAME_2[] PROGMEM = "Guest Room";
const char RELAY_NAME_3[] PROGMEM = "Kitchen";
const char RELAY_NAME_4[] PROGMEM = "Wash Room";

const char* const RELAY_NAMES[RELAY_COUNT] PROGMEM = {
  RELAY_NAME_0,
  RELAY_NAME_1,
  RELAY_NAME_2,
  RELAY_NAME_3,
  RELAY_NAME_4
};

//==============================================================================
// AUTOMATION CONFIGURATION
//==============================================================================

constexpr uint8_t MAX_AUTOMATION_RULES = 5;
constexpr uint8_t TEMP_HYSTERESIS = 2;         // °C - prevent relay chattering

// Temperature thresholds (default values)
constexpr uint8_t TEMP_THRESHOLD_HIGH = 28;    // °C
constexpr uint8_t TEMP_THRESHOLD_LOW = 20;     // °C

//==============================================================================
// SCHEDULING CONFIGURATION
//==============================================================================

constexpr uint8_t MAX_SCHEDULES = 6;
constexpr uint8_t MAX_TIMERS = 5;

// Days of week bitmask
#define SCHEDULE_MONDAY     0x01
#define SCHEDULE_TUESDAY    0x02
#define SCHEDULE_WEDNESDAY  0x04
#define SCHEDULE_THURSDAY   0x08
#define SCHEDULE_FRIDAY     0x10
#define SCHEDULE_SATURDAY   0x20
#define SCHEDULE_SUNDAY     0x40
#define SCHEDULE_WEEKDAYS   0x1F  // Mon-Fri
#define SCHEDULE_WEEKEND    0x60  // Sat-Sun
#define SCHEDULE_DAILY      0x7F  // All days

//==============================================================================
// SCENE CONFIGURATION
//==============================================================================

constexpr uint8_t MAX_SCENES = 8;

// Predefined scene IDs
enum SceneId : uint8_t {
  SCENE_GOOD_MORNING = 0,
  SCENE_GOOD_NIGHT = 1,
  SCENE_AWAY = 2,
  SCENE_PARTY = 3,
  SCENE_MOVIE_NIGHT = 4,
  SCENE_ALL_ON = 5,
  SCENE_ALL_OFF = 6,
  SCENE_CUSTOM = 7
};

//==============================================================================
// LOGGING CONFIGURATION
//==============================================================================

constexpr uint16_t TEMP_LOG_INTERVAL = 300;    // seconds (5 minutes)
constexpr uint16_t MAX_LOG_FILE_SIZE = 10000;  // lines before rotation
constexpr bool ENABLE_SERIAL_DEBUG = true;

//==============================================================================
// NETWORK CONFIGURATION
//==============================================================================

constexpr uint16_t HTTP_BUFFER_SIZE = 120;     // Increased for API requests
constexpr uint16_t WEB_SERVER_PORT = 80;
constexpr uint8_t CLIENT_TIMEOUT = 1;          // milliseconds

// Default IP (can be changed via config file)
#define DEFAULT_IP_0 192
#define DEFAULT_IP_1 168
#define DEFAULT_IP_2 0
#define DEFAULT_IP_3 120

//==============================================================================
// STATISTICS CONFIGURATION
//==============================================================================

constexpr uint16_t STATS_UPDATE_INTERVAL = 60;  // seconds
constexpr uint8_t STATS_HISTORY_HOURS = 24;     // hours to keep in memory

//==============================================================================
// MEMORY OPTIMIZATION
//==============================================================================

// Use these macros to store strings in Flash memory
#define F_STR(x) F(x)

// PROGMEM string read helper
#define READ_PROGMEM_STRING(dest, src, maxLen) \
  strncpy_P(dest, (char*)pgm_read_ptr(&src), maxLen)

//==============================================================================
// TIMING CONFIGURATION
//==============================================================================

constexpr uint16_t TEMP_READ_INTERVAL = 60000;  // ms (1 minute)
constexpr uint16_t AJAX_UPDATE_INTERVAL = 2000; // ms (2 seconds)
constexpr uint16_t AUTOMATION_CHECK_INTERVAL = 5000; // ms (5 seconds)

//==============================================================================
// ERROR CODES
//==============================================================================

enum ErrorCode : uint8_t {
  ERROR_NONE = 0,
  ERROR_SD_INIT = 1,
  ERROR_SD_FILE_NOT_FOUND = 2,
  ERROR_RTC_INIT = 3,
  ERROR_NETWORK = 4,
  ERROR_MEMORY_LOW = 5,
  ERROR_INVALID_CONFIG = 6
};

//==============================================================================
// FILE PATHS (SD Card)
//==============================================================================

#define FILE_INDEX_HTML     "index.htm"
#define FILE_CONFIG         "config.txt"
#define FILE_LOG_EVENTS     "logs/events.csv"
#define FILE_LOG_TEMP       "logs/temp.csv"
#define FILE_SCHEDULES      "data/schedules.dat"
#define FILE_AUTOMATION     "data/automation.dat"

#endif // CONFIG_H
