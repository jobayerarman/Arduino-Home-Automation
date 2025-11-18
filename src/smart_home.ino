/**
 * Arduino Smart Home - Version 3.0
 *
 * Enhanced home automation system with:
 * - Temperature-based automation
 * - Time-based scheduling
 * - Scene management
 * - Real-time monitoring
 * - Enhanced REST API
 *
 * Hardware Requirements:
 * - Arduino Uno (or compatible)
 * - Ethernet Shield (W5100)
 * - SD Card (FAT16/FAT32)
 * - Thermistor (NTC 10K)
 * - 5x Relays
 * - DS3231 RTC Module (optional, for scheduling)
 *
 * Libraries Required:
 * - Ethernet (built-in)
 * - SD (built-in)
 * - SPI (built-in)
 * - Thermistor (custom)
 * - RTClib (optional, for RTC support)
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Thermistor.h>

// Include our custom modules
#include "Config.h"
#include "AutomationController.h"
#include "ScheduleController.h"
#include "SceneController.h"

//==============================================================================
// GLOBAL OBJECTS
//==============================================================================

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(DEFAULT_IP_0, DEFAULT_IP_1, DEFAULT_IP_2, DEFAULT_IP_3);

// Objects
Thermistor tempSensor(THERMISTOR_PIN);
EthernetServer server(WEB_SERVER_PORT);
File webFile;

// Controllers
AutomationController automation;
ScheduleController scheduler;
SceneController scenes;

// System State
char httpRequestBuffer[HTTP_BUFFER_SIZE] = {0};
uint16_t requestIndex = 0;
bool relayStates[RELAY_COUNT] = {false};
uint8_t currentTemperature = 0;

// Timing
unsigned long lastTempRead = 0;
unsigned long lastAutomationCheck = 0;
unsigned long lastStatsUpdate = 0;

// Statistics
uint32_t relayOnTime[RELAY_COUNT] = {0};      // Total ON time in seconds
uint16_t relayTriggerCount[RELAY_COUNT] = {0}; // Number of state changes
unsigned long relayLastChange[RELAY_COUNT] = {0};
uint8_t tempMin = 255, tempMax = 0;

//==============================================================================
// RELAY CONTROL FUNCTIONS
//==============================================================================

/**
 * Relay configuration (stored in Flash memory)
 */
struct RelayConfig {
  uint8_t pin;
  const char* command;
  const char* name;
};

const RelayConfig RELAY_CONFIGS[] PROGMEM = {
  {RELAY_LIVING_ROOM_PIN, "RELAY1", "Living Room"},
  {RELAY_MASTER_BED_PIN, "RELAY2", "Master Bed"},
  {RELAY_GUEST_ROOM_PIN, "RELAY3", "Guest Room"},
  {RELAY_KITCHEN_PIN, "RELAY4", "Kitchen"},
  {RELAY_WASH_ROOM_PIN, "RELAY5", "Wash Room"}
};

/**
 * Set relay state
 * This is the central relay control function used by all modules
 */
void setRelay(uint8_t relayId, bool state) {
  if (relayId >= RELAY_COUNT) {
    return;
  }

  // Only update if state changed
  if (relayStates[relayId] != state) {
    uint8_t pin = pgm_read_byte(&RELAY_CONFIGS[relayId].pin);
    digitalWrite(pin, state ? HIGH : LOW);
    relayStates[relayId] = state;

    // Update statistics
    relayTriggerCount[relayId]++;
    relayLastChange[relayId] = millis();

    if (ENABLE_SERIAL_DEBUG) {
      char nameBuf[20];
      strcpy_P(nameBuf, (char*)pgm_read_ptr(&RELAY_CONFIGS[relayId].name));
      Serial.print(F("Relay "));
      Serial.print(nameBuf);
      Serial.print(F(": "));
      Serial.println(state ? F("ON") : F("OFF"));
    }
  }
}

/**
 * Initialize all relay pins
 */
void initializeRelays() {
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    uint8_t pin = pgm_read_byte(&RELAY_CONFIGS[i].pin);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);  // Start with all relays OFF
    relayStates[i] = false;
  }

  Serial.println(F("Relays initialized"));
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

/**
 * Search for substring in string
 */
bool stringContains(const char* str, const char* find) {
  const uint8_t strLength = strlen(str);
  const uint8_t findLength = strlen(find);

  if (findLength > strLength || findLength == 0) {
    return false;
  }

  uint8_t matchCount = 0;

  for (uint8_t i = 0; i < strLength; i++) {
    if (str[i] == find[matchCount]) {
      matchCount++;
      if (matchCount == findLength) {
        return true;
      }
    } else {
      matchCount = 0;
    }
  }

  return false;
}

/**
 * Clear buffer
 */
void clearBuffer(char* buffer, uint16_t size) {
  memset(buffer, 0, size);
}

/**
 * Get free SRAM
 */
int getFreeRAM() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

/**
 * Extract parameter value from URL
 * Example: getURLParameter("?relay=3&state=1", "relay", buffer)
 */
bool getURLParameter(const char* url, const char* param, char* value, uint8_t maxLen) {
  char searchStr[32];
  snprintf(searchStr, sizeof(searchStr), "%s=", param);

  const char* start = strstr(url, searchStr);
  if (start == nullptr) {
    return false;
  }

  start += strlen(searchStr);
  const char* end = strchr(start, '&');

  uint8_t len;
  if (end == nullptr) {
    len = strlen(start);
  } else {
    len = end - start;
  }

  if (len >= maxLen) {
    len = maxLen - 1;
  }

  strncpy(value, start, len);
  value[len] = '\0';

  return true;
}

//==============================================================================
// API HANDLERS
//==============================================================================

/**
 * Handle relay control API
 * Endpoint: /api/relay?id=0&state=1
 */
void handleAPIRelay(EthernetClient& client) {
  char idStr[4], stateStr[4];

  if (getURLParameter(httpRequestBuffer, "id", idStr, sizeof(idStr)) &&
      getURLParameter(httpRequestBuffer, "state", stateStr, sizeof(stateStr))) {

    uint8_t relayId = atoi(idStr);
    bool state = (atoi(stateStr) != 0);

    if (relayId < RELAY_COUNT) {
      setRelay(relayId, state);

      // Send success response
      client.println(F("{\"status\":\"ok\",\"relay\":"));
      client.print(relayId);
      client.print(F(",\"state\":"));
      client.print(state ? F("true") : F("false"));
      client.println(F("}"));
    } else {
      client.println(F("{\"status\":\"error\",\"message\":\"Invalid relay ID\"}"));
    }
  } else {
    client.println(F("{\"status\":\"error\",\"message\":\"Missing parameters\"}"));
  }
}

/**
 * Handle scene activation API
 * Endpoint: /api/scene?name=good_night
 */
void handleAPIScene(EthernetClient& client) {
  char sceneName[20];

  if (getURLParameter(httpRequestBuffer, "name", sceneName, sizeof(sceneName))) {
    if (scenes.activateScene(sceneName, setRelay)) {
      client.print(F("{\"status\":\"ok\",\"scene\":\""));
      client.print(sceneName);
      client.println(F("\"}"));
    } else {
      client.println(F("{\"status\":\"error\",\"message\":\"Scene not found\"}"));
    }
  } else {
    // List all scenes
    char sceneList[256];
    scenes.listScenesJSON(sceneList, sizeof(sceneList));
    client.print(F("{\"status\":\"ok\",\"scenes\":"));
    client.print(sceneList);
    client.println(F("}"));
  }
}

/**
 * Handle timer API
 * Endpoint: /api/timer?relay=0&duration=3600
 */
void handleAPITimer(EthernetClient& client) {
  char relayStr[4], durationStr[8];

  if (getURLParameter(httpRequestBuffer, "relay", relayStr, sizeof(relayStr)) &&
      getURLParameter(httpRequestBuffer, "duration", durationStr, sizeof(durationStr))) {

    uint8_t relayId = atoi(relayStr);
    uint32_t duration = atol(durationStr);

    if (scheduler.addTimer(relayId, duration, true)) {
      client.print(F("{\"status\":\"ok\",\"relay\":"));
      client.print(relayId);
      client.print(F(",\"duration\":"));
      client.print(duration);
      client.println(F("}"));
    } else {
      client.println(F("{\"status\":\"error\",\"message\":\"Failed to add timer\"}"));
    }
  } else {
    client.println(F("{\"status\":\"error\",\"message\":\"Missing parameters\"}"));
  }
}

/**
 * Handle status API (current state of everything)
 * Endpoint: /api/status
 */
void handleAPIStatus(EthernetClient& client) {
  client.println(F("{"));

  // Temperature
  client.print(F("\"temperature\":"));
  client.print(currentTemperature);
  client.println(F(","));

  // Relays
  client.print(F("\"relays\":["));
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    if (i > 0) client.print(F(","));

    char nameBuf[20];
    strcpy_P(nameBuf, (char*)pgm_read_ptr(&RELAY_CONFIGS[i].name));

    client.print(F("{\"id\":"));
    client.print(i);
    client.print(F(",\"name\":\""));
    client.print(nameBuf);
    client.print(F("\",\"state\":"));
    client.print(relayStates[i] ? F("true") : F("false"));

    // Add timer info if exists
    uint32_t remaining = scheduler.getTimerRemaining(i);
    if (remaining > 0) {
      client.print(F(",\"timer\":"));
      client.print(remaining);
    }

    client.print(F("}"));
  }
  client.println(F("],"));

  // System info
  client.print(F("\"system\":{"));
  client.print(F("\"uptime\":"));
  client.print(millis() / 1000);
  client.print(F(",\"free_ram\":"));
  client.print(getFreeRAM());
  client.print(F(",\"automation_enabled\":"));
  client.print(automation.isEnabled() ? F("true") : F("false"));
  client.println(F("}"));

  client.println(F("}"));
}

/**
 * Handle statistics API
 * Endpoint: /api/stats
 */
void handleAPIStats(EthernetClient& client) {
  client.println(F("{\"stats\":["));

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    if (i > 0) client.print(F(","));

    char nameBuf[20];
    strcpy_P(nameBuf, (char*)pgm_read_ptr(&RELAY_CONFIGS[i].name));

    client.print(F("{\"name\":\""));
    client.print(nameBuf);
    client.print(F("\",\"on_time\":"));
    client.print(relayOnTime[i]);
    client.print(F(",\"trigger_count\":"));
    client.print(relayTriggerCount[i]);
    client.print(F("}"));
  }

  client.print(F("],\"temperature\":{\"current\":"));
  client.print(currentTemperature);
  client.print(F(",\"min\":"));
  client.print(tempMin);
  client.print(F(",\"max\":"));
  client.print(tempMax);
  client.println(F("}}"));
}

//==============================================================================
// HTTP REQUEST HANDLER
//==============================================================================

/**
 * Process HTTP request and route to appropriate handler
 */
void handleHTTPRequest(EthernetClient& client) {
  // Send HTTP header
  client.println(F("HTTP/1.1 200 OK"));

  // Route API requests
  if (stringContains(httpRequestBuffer, "GET /api/status")) {
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    handleAPIStatus(client);
  }
  else if (stringContains(httpRequestBuffer, "GET /api/relay")) {
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    handleAPIRelay(client);
  }
  else if (stringContains(httpRequestBuffer, "GET /api/scene")) {
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    handleAPIScene(client);
  }
  else if (stringContains(httpRequestBuffer, "GET /api/timer")) {
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    handleAPITimer(client);
  }
  else if (stringContains(httpRequestBuffer, "GET /api/stats")) {
    client.println(F("Content-Type: application/json"));
    client.println(F("Connection: close"));
    client.println();
    handleAPIStats(client);
  }
  else {
    // Serve HTML page
    client.println(F("Content-Type: text/html"));
    client.println(F("Connection: close"));
    client.println();

    webFile = SD.open(FILE_INDEX_HTML);
    if (webFile) {
      uint8_t buffer[64];
      while (webFile.available()) {
        int bytesRead = webFile.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
          client.write(buffer, bytesRead);
        }
      }
      webFile.close();
    }
  }

  // Reset buffer
  requestIndex = 0;
  clearBuffer(httpRequestBuffer, HTTP_BUFFER_SIZE);
}

//==============================================================================
// UPDATE FUNCTIONS
//==============================================================================

/**
 * Update statistics
 */
void updateStatistics() {
  unsigned long now = millis();

  // Update ON time for active relays
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    if (relayStates[i]) {
      relayOnTime[i]++;  // Increment seconds
    }
  }

  // Track temperature min/max
  if (currentTemperature < tempMin) tempMin = currentTemperature;
  if (currentTemperature > tempMax) tempMax = currentTemperature;
}

//==============================================================================
// SETUP
//==============================================================================

void setup() {
  // Disable Ethernet chip during initialization
  pinMode(ETHERNET_CS_PIN, OUTPUT);
  digitalWrite(ETHERNET_CS_PIN, HIGH);

  // Initialize serial
  Serial.begin(9600);
  Serial.println(F("====================================="));
  Serial.println(F("Arduino Smart Home v3.0"));
  Serial.println(F("====================================="));

  // Display free SRAM
  Serial.print(F("Free SRAM: "));
  Serial.print(getFreeRAM());
  Serial.println(F(" bytes"));

  // Initialize SD card
  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    Serial.println(F("ERROR: SD card initialization failed!"));
    Serial.println(F("System halted. Check SD card and reset."));
    while(1);
  }
  Serial.println(F("SD card OK"));

  // Verify index.htm exists
  if (!SD.exists(FILE_INDEX_HTML)) {
    Serial.println(F("ERROR: index.htm not found on SD card!"));
    Serial.println(F("System halted. Upload index.htm and reset."));
    while(1);
  }

  // Create directories if needed
  if (!SD.exists("data")) SD.mkdir("data");
  if (!SD.exists("logs")) SD.mkdir("logs");

  // Initialize relays
  initializeRelays();

  // Initialize controllers
  automation.begin();
  scheduler.begin();
  scenes.begin();

  // Start Ethernet server
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print(F("Server IP: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("System ready!"));
  Serial.println(F("====================================="));
}

//==============================================================================
// MAIN LOOP
//==============================================================================

void loop() {
  unsigned long now = millis();

  // Read temperature periodically
  if (now - lastTempRead >= TEMP_READ_INTERVAL) {
    currentTemperature = tempSensor.getTemp();
    lastTempRead = now;
  }

  // Check automation rules
  if (now - lastAutomationCheck >= AUTOMATION_CHECK_INTERVAL) {
    automation.evaluate(currentTemperature, relayStates, setRelay);
    lastAutomationCheck = now;
  }

  // Update schedules and timers
  scheduler.update(relayStates, setRelay);

  // Update statistics
  if (now - lastStatsUpdate >= STATS_UPDATE_INTERVAL * 1000) {
    updateStatistics();
    lastStatsUpdate = now;
  }

  // Handle web requests
  EthernetClient client = server.available();

  if (client) {
    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // Buffer HTTP request
        if (requestIndex < (HTTP_BUFFER_SIZE - 1)) {
          httpRequestBuffer[requestIndex] = c;
          requestIndex++;
        }

        // Check for end of request
        if (c == '\n' && currentLineIsBlank) {
          handleHTTPRequest(client);
          break;
        }

        // Track line state
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
  }
}
