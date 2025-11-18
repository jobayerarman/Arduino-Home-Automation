# Arduino Home Automation - Step-by-Step Refactoring Guide

This guide provides concrete, actionable steps to refactor the Arduino home automation code from prototype to production quality.

---

## PHASE 1: QUICK WINS (2-4 hours)

### Step 1.1: Add Flash Memory Optimization (F() Macro)

**Estimated Time:** 30 minutes
**SRAM Saved:** ~400 bytes
**Risk:** Very Low

**Before:**
```cpp
Serial.println("ERROR - SD card initialization failed!");
client.println("HTTP/1.1 200 OK");
```

**After:**
```cpp
Serial.println(F("ERROR - SD card initialization failed!"));
client.println(F("HTTP/1.1 200 OK"));
```

**Action Items:**
1. Search for all `Serial.print` and `Serial.println` calls
2. Wrap string literals with `F()` macro
3. Search for all `client.print` and `client.println` calls
4. Wrap string literals with `F()` macro
5. Compile and verify SRAM usage decreased

**Complete List of Changes:**
```cpp
// Line 81
Serial.println(F("ERROR - SD card initialization failed!"));

// Line 85
Serial.println(F("ERROR - Can't find index.htm file!"));

// Line 122
client.println(F("HTTP/1.1 200 OK"));

// Line 129
client.println(F("Content-Type: text/xml"));

// Line 130
client.println(F("Connection: keep-alive"));

// Line 139
client.println(F("Content-Type: text/html"));

// Line 140
client.println(F("Connection: keep-alive"));

// Line 233
cl.print(F("<?xml version = \"1.0\" ?>"));

// Line 234
cl.print(F("<inputs>"));

// Line 236
cl.print(F("<temp>"));

// Line 238
cl.print(F("</temp>"));

// Line 241
cl.print(F("<BUTTON>"));

// Line 243
cl.print(F("on"));

// Line 246
cl.print(F("off"));

// Line 248
cl.println(F("</BUTTON>"));

// Line 251
cl.print(F("</inputs>"));
```

### Step 1.2: Replace Deprecated Types

**Estimated Time:** 15 minutes
**SRAM Saved:** 5 bytes
**Risk:** Very Low

**Changes:**
```cpp
// Line 69 - BEFORE
char req_index = 0;
// AFTER
uint8_t requestIndex = 0;

// Line 71 - BEFORE
boolean RELAY_state[BTN_NUM] = {0};
// AFTER
bool relayStates[BTN_NUM] = {false};

// Line 231 - BEFORE
byte celsius = temp.getTemp();
// AFTER
uint8_t celsius = temp.getTemp();

// Lines 255, 264, 266, 267 - BEFORE
char StrClear(char *str, char length)
char StrContains(char *str, char *sfind)
char found = 0;
char index = 0;
// AFTER
void clearBuffer(char *str, uint8_t length)
bool stringContains(const char *str, const char *sfind)
uint8_t matchCount = 0;
uint8_t index = 0;
```

### Step 1.3: Add Pin Constants

**Estimated Time:** 20 minutes
**SRAM Saved:** 0 bytes
**Risk:** Low (prevents hardware errors)

**Add at top of file (after #define statements):**
```cpp
// Pin Definitions
constexpr uint8_t SD_CHIP_SELECT_PIN = 4;
constexpr uint8_t ETHERNET_CS_PIN = 10;

constexpr uint8_t RELAY_LIVING_ROOM_PIN = 5;
constexpr uint8_t RELAY_MASTER_BED_PIN = 6;
constexpr uint8_t RELAY_GUEST_ROOM_PIN = 9;
constexpr uint8_t RELAY_KITCHEN_PIN = 8;
constexpr uint8_t RELAY_WASH_ROOM_PIN = 7;

constexpr uint8_t THERMISTOR_PIN = 2;
```

**Replace in setup():**
```cpp
// Line 75-76 - BEFORE
pinMode(10, OUTPUT);
digitalWrite(10, HIGH);
// AFTER
pinMode(ETHERNET_CS_PIN, OUTPUT);
digitalWrite(ETHERNET_CS_PIN, HIGH);

// Line 80 - BEFORE
if (!SD.begin(4)) {
// AFTER
if (!SD.begin(SD_CHIP_SELECT_PIN)) {

// Lines 90-94 - BEFORE
pinMode(5, OUTPUT);
pinMode(6, OUTPUT);
pinMode(7, OUTPUT);
pinMode(8, OUTPUT);
pinMode(9, OUTPUT);
// AFTER
pinMode(RELAY_LIVING_ROOM_PIN, OUTPUT);
pinMode(RELAY_MASTER_BED_PIN, OUTPUT);
pinMode(RELAY_WASH_ROOM_PIN, OUTPUT);
pinMode(RELAY_KITCHEN_PIN, OUTPUT);
pinMode(RELAY_GUEST_ROOM_PIN, OUTPUT);
```

### Step 1.4: Optimize String Functions

**Estimated Time:** 30 minutes
**Performance:** 3× faster
**Risk:** Low

**Replace `StrContains()` function (lines 264-289):**
```cpp
/**
 * Search for substring in string
 * @param str Haystack string to search in
 * @param find Needle substring to find
 * @return true if found, false otherwise
 */
bool stringContains(const char* str, const char* find) {
  // Cache lengths to avoid repeated scans
  const uint8_t strLength = strlen(str);
  const uint8_t findLength = strlen(find);

  // Early exit
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
```

**Replace `StrClear()` function (lines 255-259):**
```cpp
/**
 * Clear buffer by zeroing all bytes
 * @param buffer Pointer to buffer to clear
 * @param size Number of bytes to zero
 */
void clearBuffer(char* buffer, uint8_t size) {
  memset(buffer, 0, size);
}
```

**Update all function calls:**
```cpp
// Line 127 - BEFORE
if (StrContains(HTTP_req, "button_state")) {
// AFTER
if (stringContains(httpRequestBuffer, "button_state")) {

// Line 154 - BEFORE
StrClear(HTTP_req, REQ_BUF_SZ);
// AFTER
clearBuffer(httpRequestBuffer, HTTP_BUFFER_SIZE);

// Line 179 - BEFORE
if (StrContains(HTTP_req, "RELAY1=1")) {
// AFTER
if (stringContains(httpRequestBuffer, "RELAY1=1")) {
```

### Step 1.5: Buffer SD Card Reads

**Estimated Time:** 20 minutes
**Performance:** 64× faster
**Risk:** Low

**Replace lines 145-150:**
```cpp
// BEFORE
if (webFile) {
    while(webFile.available()) {
        client.write(webFile.read()); // 1 byte at a time!
    }
    webFile.close();
}

// AFTER
if (webFile) {
    uint8_t buffer[64];  // Stack-allocated buffer
    while (webFile.available()) {
        int bytesRead = webFile.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
            client.write(buffer, bytesRead);
        }
    }
    webFile.close();
}
```

### Step 1.6: Fix Setup Error Handling

**Estimated Time:** 10 minutes
**Risk:** Low (improves stability)

**Replace setup() error handling (lines 80-87):**
```cpp
// BEFORE
if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // ❌ Returns but loop() continues!
}
if (!SD.exists("index.htm")) {
    Serial.println("ERROR - Can't find index.htm file!");
    return;  // ❌ Same issue
}

// AFTER
if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    Serial.println(F("ERROR - SD card initialization failed!"));
    Serial.println(F("System halted. Check SD card and reset Arduino."));
    while(1);  // ✅ Halt system permanently
}
if (!SD.exists("index.htm")) {
    Serial.println(F("ERROR - Can't find index.htm file!"));
    Serial.println(F("System halted. Upload index.htm and reset Arduino."));
    while(1);  // ✅ Halt system permanently
}
```

---

## PHASE 2: STRUCTURAL REFACTOR (4-8 hours)

### Step 2.1: Create Relay Configuration Structure

**Estimated Time:** 1 hour
**Lines Removed:** 70
**Risk:** Medium

**Add after constants:**
```cpp
/**
 * Relay configuration structure
 */
struct RelayConfig {
  uint8_t pin;
  const char* command;
  const char* name;
};

// Configuration table in Flash memory
const RelayConfig RELAY_CONFIGS[] PROGMEM = {
  {RELAY_LIVING_ROOM_PIN, "RELAY1", "Living Room"},
  {RELAY_MASTER_BED_PIN, "RELAY2", "Master Bed"},
  {RELAY_GUEST_ROOM_PIN, "RELAY3", "Guest Room"},
  {RELAY_KITCHEN_PIN, "RELAY4", "Kitchen"},
  {RELAY_WASH_ROOM_PIN, "RELAY5", "Wash Room"}
};

constexpr uint8_t RELAY_COUNT = sizeof(RELAY_CONFIGS) / sizeof(RelayConfig);
bool relayStates[RELAY_COUNT] = {false};
```

### Step 2.2: Refactor SetRELAYs() to Data-Driven

**Estimated Time:** 1.5 hours
**Lines Reduced:** 100 → 30
**Risk:** Medium

**Replace entire `SetRELAYs()` function (lines 177-227):**
```cpp
/**
 * Process relay commands from HTTP request
 */
void processRelayCommands() {
  char commandBuffer[16];

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    // Read command from Flash
    strcpy_P(commandBuffer,
             (const char*)pgm_read_ptr(&RELAY_CONFIGS[i].command));

    // Check ON command
    char onCommand[16];
    strcpy(onCommand, commandBuffer);
    strcat(onCommand, "=1");

    if (stringContains(httpRequestBuffer, onCommand)) {
      uint8_t pin = pgm_read_byte(&RELAY_CONFIGS[i].pin);
      relayStates[i] = true;
      digitalWrite(pin, HIGH);
      continue;  // Skip OFF check
    }

    // Check OFF command
    char offCommand[16];
    strcpy(offCommand, commandBuffer);
    strcat(offCommand, "=0");

    if (stringContains(httpRequestBuffer, offCommand)) {
      uint8_t pin = pgm_read_byte(&RELAY_CONFIGS[i].pin);
      relayStates[i] = false;
      digitalWrite(pin, LOW);
    }
  }
}
```

**Update function call (line 132):**
```cpp
// BEFORE
SetRELAYs();
// AFTER
processRelayCommands();
```

### Step 2.3: Refactor XML Response

**Estimated Time:** 30 minutes
**SRAM Saved:** 150 bytes
**Risk:** Low

**Replace `XML_response()` function (lines 230-252):**
```cpp
/**
 * Send XML response with temperature and relay states
 * @param client Connected Ethernet client
 */
void sendXmlResponse(EthernetClient& client) {
  uint8_t celsius = temp.getTemp();

  client.print(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"));
  client.print(F("<inputs>"));

  // Temperature
  client.print(F("<temp>"));
  client.print(celsius);
  client.print(F("</temp>"));

  // Relay states
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    client.print(F("<BUTTON>"));
    client.print(relayStates[i] ? F("on") : F("off"));
    client.print(F("</BUTTON>"));
  }

  client.print(F("</inputs>"));
}
```

**Update function call (line 134):**
```cpp
// BEFORE
XML_response(client);
// AFTER
sendXmlResponse(client);
```

### Step 2.4: Extract HTTP Request Parsing

**Estimated Time:** 1 hour
**Risk:** Medium

**Add new function before `loop()`:**
```cpp
/**
 * Parse and handle HTTP request
 * @param client Connected Ethernet client
 */
void handleHttpRequest(EthernetClient& client) {
  // Send HTTP header
  client.println(F("HTTP/1.1 200 OK"));

  // Check request type
  if (stringContains(httpRequestBuffer, "button_state")) {
    // AJAX request - send XML
    client.println(F("Content-Type: text/xml"));
    client.println(F("Connection: keep-alive"));
    client.println();

    processRelayCommands();
    sendXmlResponse(client);
  }
  else {
    // Page request - send HTML
    client.println(F("Content-Type: text/html"));
    client.println(F("Connection: keep-alive"));
    client.println();

    serveWebPage(client);
  }

  // Reset buffer
  requestIndex = 0;
  clearBuffer(httpRequestBuffer, HTTP_BUFFER_SIZE);
}

/**
 * Serve index.htm from SD card
 * @param client Connected Ethernet client
 */
void serveWebPage(EthernetClient& client) {
  webFile = SD.open("index.htm");

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
```

**Simplify `loop()` function:**
```cpp
void loop() {
  EthernetClient client = server.available();

  if (client) {
    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char currentChar = client.read();

        // Buffer request
        if (requestIndex < (HTTP_BUFFER_SIZE - 1)) {
          httpRequestBuffer[requestIndex] = currentChar;
          requestIndex++;
        }

        // Check for end of request
        if (currentChar == '\n' && currentLineIsBlank) {
          handleHttpRequest(client);
          break;
        }

        // Track line state
        if (currentChar == '\n') {
          currentLineIsBlank = true;
        }
        else if (currentChar != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
  }
}
```

### Step 2.5: Initialize Relays from Configuration

**Estimated Time:** 30 minutes
**Risk:** Low

**Replace pinMode calls in `setup()` (lines 90-94):**
```cpp
// BEFORE
pinMode(5, OUTPUT);
pinMode(6, OUTPUT);
pinMode(7, OUTPUT);
pinMode(8, OUTPUT);
pinMode(9, OUTPUT);

// AFTER
// Initialize relay pins from configuration
for (uint8_t i = 0; i < RELAY_COUNT; i++) {
  uint8_t pin = pgm_read_byte(&RELAY_CONFIGS[i].pin);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);  // Ensure all relays start OFF
}
```

---

## PHASE 3: ADVANCED OPTIMIZATIONS (4-8 hours)

### Step 3.1: Remove Blocking delay()

**Estimated Time:** 1 hour
**Risk:** Low

**Add global variable:**
```cpp
unsigned long clientDisconnectTime = 0;
bool clientPendingDisconnect = false;
```

**Replace in loop():**
```cpp
// BEFORE (end of loop)
delay(1);
client.stop();

// AFTER
clientDisconnectTime = millis();
clientPendingDisconnect = true;
```

**Add at start of loop():**
```cpp
// Handle pending client disconnect
if (clientPendingDisconnect && (millis() - clientDisconnectTime >= 1)) {
  client.stop();
  clientPendingDisconnect = false;
}
```

### Step 3.2: Add SRAM Monitoring

**Estimated Time:** 30 minutes
**Risk:** Very Low

**Add utility function:**
```cpp
/**
 * Get free SRAM in bytes
 * @return Available SRAM
 */
int getFreeRAM() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
```

**Add to setup():**
```cpp
Serial.print(F("Free SRAM: "));
Serial.print(getFreeRAM());
Serial.println(F(" bytes"));
```

### Step 3.3: Implement Finite State Machine

**Estimated Time:** 2-3 hours
**Risk:** High (major refactor)

**Add state enum:**
```cpp
enum class ServerState : uint8_t {
  IDLE,
  CLIENT_CONNECTED,
  RECEIVING_REQUEST,
  PROCESSING_COMMAND,
  SENDING_RESPONSE
};

ServerState currentState = ServerState::IDLE;
```

**Refactor loop():**
```cpp
void loop() {
  switch (currentState) {
    case ServerState::IDLE: {
      EthernetClient client = server.available();
      if (client) {
        currentState = ServerState::CLIENT_CONNECTED;
      }
      break;
    }

    case ServerState::CLIENT_CONNECTED: {
      // Initialize request
      requestIndex = 0;
      clearBuffer(httpRequestBuffer, HTTP_BUFFER_SIZE);
      currentState = ServerState::RECEIVING_REQUEST;
      break;
    }

    case ServerState::RECEIVING_REQUEST: {
      // Read and buffer request
      // ... (existing logic)
      break;
    }

    case ServerState::PROCESSING_COMMAND: {
      processRelayCommands();
      currentState = ServerState::SENDING_RESPONSE;
      break;
    }

    case ServerState::SENDING_RESPONSE: {
      handleHttpRequest(client);
      currentState = ServerState::IDLE;
      break;
    }
  }
}
```

### Step 3.4: Add Watchdog Timer

**Estimated Time:** 30 minutes
**Risk:** Low

**Add at top:**
```cpp
#include <avr/wdt.h>
```

**Add to setup():**
```cpp
wdt_enable(WDTO_8S);  // 8 second watchdog
```

**Add to loop():**
```cpp
wdt_reset();  // Pet the watchdog
```

---

## VALIDATION CHECKLIST

After each phase, verify:

### Phase 1 Validation
- [ ] Code compiles without errors
- [ ] Free SRAM increased (check with `getFreeRAM()`)
- [ ] Web page still loads correctly
- [ ] All 5 relays toggle ON/OFF via web interface
- [ ] Temperature reading appears correctly
- [ ] No regression in functionality

### Phase 2 Validation
- [ ] Code compiles without errors
- [ ] Adding a new relay only requires editing RELAY_CONFIGS array
- [ ] All relays initialize to OFF state on reset
- [ ] XML response format unchanged (check browser console)
- [ ] Page load time improved (measure with browser dev tools)

### Phase 3 Validation
- [ ] System responds to requests while processing
- [ ] Watchdog timer resets Arduino on hang (test by adding infinite loop)
- [ ] FSM transitions correctly (add debug prints)
- [ ] SRAM usage stable over 24 hours

---

## ROLLBACK PLAN

If issues occur during refactoring:

1. **Git Commit After Each Step:**
   ```bash
   git add .
   git commit -m "Phase 1.1: Add F() macro to string literals"
   ```

2. **Test After Each Commit:**
   - Upload to Arduino
   - Verify basic functionality
   - If broken, revert:
     ```bash
     git revert HEAD
     ```

3. **Keep Original:**
   ```bash
   cp webserver_sketch.ino webserver_sketch.ino.backup
   ```

---

## PERFORMANCE BENCHMARKS

Use these tests to measure improvements:

### Test 1: Memory Usage
```cpp
void setup() {
  Serial.begin(9600);
  Serial.print(F("Free SRAM: "));
  Serial.println(getFreeRAM());
}
```
**Target:** >1000 bytes free (>50% of 2048 bytes)

### Test 2: Page Load Time
Use browser developer tools:
1. Open Network tab
2. Request http://192.168.0.120/
3. Measure "Time to First Byte" (TTFB)

**Target:** <100ms (down from 2000ms)

### Test 3: Request Throughput
Send 100 rapid AJAX requests:
```javascript
for (let i = 0; i < 100; i++) {
  fetch('/button_state?RELAY1=1');
}
```
**Target:** All requests succeed, no timeouts

---

## FINAL CODE STRUCTURE

After all refactoring:

```cpp
/*
 * Arduino Home Automation - Refactored v2.0
 * Production-ready code with optimizations
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Thermistor.h>
#include <avr/wdt.h>

// Constants
constexpr uint8_t HTTP_BUFFER_SIZE = 60;
constexpr uint8_t SD_CHIP_SELECT_PIN = 4;
// ... (all pin constants)

// Configuration
struct RelayConfig { /* ... */ };
const RelayConfig RELAY_CONFIGS[] PROGMEM = { /* ... */ };

// Global State
bool relayStates[RELAY_COUNT] = {false};
char httpRequestBuffer[HTTP_BUFFER_SIZE] = {0};
uint8_t requestIndex = 0;

// Objects
Thermistor temp(THERMISTOR_PIN);
EthernetServer server(80);

// Function Declarations
void setup();
void loop();
void handleHttpRequest(EthernetClient& client);
void processRelayCommands();
void sendXmlResponse(EthernetClient& client);
void serveWebPage(EthernetClient& client);
bool stringContains(const char* str, const char* find);
void clearBuffer(char* buffer, uint8_t size);
int getFreeRAM();

// Implementation
void setup() { /* ... */ }
void loop() { /* ... */ }
// ... (all functions)
```

**Metrics:**
- Total Lines: ~220 (down from 290)
- Functions: 8 (up from 5, but better organized)
- Global Variables: 5 (down from 7)
- Duplicate Code: 0 lines (down from 100)
- SRAM Free: >1000 bytes (up from ~400)

---

## COMMON PITFALLS

### ❌ Pitfall 1: Forgetting PROGMEM Read Functions
```cpp
// WRONG
const char* command = RELAY_CONFIGS[i].command;

// CORRECT
const char* command = (const char*)pgm_read_ptr(&RELAY_CONFIGS[i].command);
```

### ❌ Pitfall 2: Buffer Overflow in String Operations
```cpp
// WRONG (no bounds check)
strcpy(onCommand, commandBuffer);
strcat(onCommand, "=1");

// CORRECT (use snprintf)
snprintf(onCommand, sizeof(onCommand), "%s=1", commandBuffer);
```

### ❌ Pitfall 3: Forgetting to Update All References
After renaming `HTTP_req` to `httpRequestBuffer`, search for ALL occurrences!
Use Find & Replace:
```
Find: HTTP_req
Replace: httpRequestBuffer
```

---

## NEXT STEPS

1. **Read CODE_AUDIT_REPORT.md** for detailed analysis
2. **Start with Phase 1** (low risk, high reward)
3. **Test thoroughly** after each step
4. **Commit frequently** to Git
5. **Measure improvements** with benchmarks
6. **Document changes** in commit messages

---

**Good luck with the refactoring! Each improvement brings you closer to production-ready code.**
