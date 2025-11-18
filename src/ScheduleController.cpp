/**
 * ScheduleController.cpp - Implementation of scheduling system
 * Note: Requires RTClib library (install via Arduino Library Manager)
 *       For simulation/testing without RTC, system will use millis()
 */

#include "ScheduleController.h"
#include <SD.h>

// Uncomment if you have an RTC module installed
// #include <RTClib.h>
// RTC_DS3231 rtc;  // or RTC_DS1307 for DS1307 module

//==============================================================================
// CONSTRUCTOR
//==============================================================================

ScheduleController::ScheduleController()
  : scheduleCount(0), timerCount(0), rtcAvailable(false), lastMinute(0) {
  // Initialize all schedules to disabled
  for (uint8_t i = 0; i < MAX_SCHEDULES; i++) {
    schedules[i].enabled = false;
    schedules[i].isActive = false;
  }

  // Initialize all timers to disabled
  for (uint8_t i = 0; i < MAX_TIMERS; i++) {
    timers[i].enabled = false;
  }
}

//==============================================================================
// INITIALIZATION
//==============================================================================

bool ScheduleController::begin() {
  // Try to initialize RTC
  // Uncomment if you have RTC module:
  /*
  if (rtc.begin()) {
    rtcAvailable = true;

    // Check if RTC lost power and if so, set the time
    if (rtc.lostPower()) {
      Serial.println(F("RTC lost power, setting time!"));
      // Set to compile time
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println(F("RTC initialized"));
  } else {
    Serial.println(F("RTC not found, scheduling disabled"));
    rtcAvailable = false;
  }
  */

  // For testing without RTC:
  rtcAvailable = false;
  if (ENABLE_SERIAL_DEBUG) {
    Serial.println(F("RTC not available - install RTClib and uncomment code"));
    Serial.println(F("Timers will work, but schedules require RTC"));
  }

  // Load saved schedules
  loadFromSD();

  return rtcAvailable;
}

//==============================================================================
// UPDATE LOOP
//==============================================================================

void ScheduleController::update(bool* relayStates,
                                  void (*relayCallback)(uint8_t, bool)) {
  // Always update timers (they use millis(), not RTC)
  updateTimers(relayStates, relayCallback);

  // Only check schedules if RTC is available
  if (!rtcAvailable) {
    return;
  }

  uint8_t currentHour, currentMin, currentSec, dayOfWeek;
  if (!getCurrentTime(&currentHour, &currentMin, &currentSec, &dayOfWeek)) {
    return;
  }

  // Only check schedules when minute changes (avoid repeated triggers)
  if (currentMin == lastMinute) {
    return;
  }
  lastMinute = currentMin;

  // Check each schedule
  for (uint8_t i = 0; i < scheduleCount; i++) {
    Schedule& sched = schedules[i];

    if (!sched.enabled) {
      continue;
    }

    bool shouldBeActive = shouldScheduleBeActive(sched, currentHour,
                                                  currentMin, dayOfWeek);

    // State changed?
    if (shouldBeActive && !sched.isActive) {
      // Turn ON
      if (relayCallback) {
        relayCallback(sched.relayId, true);
      }
      sched.isActive = true;

      if (ENABLE_SERIAL_DEBUG) {
        Serial.print(F("Schedule ON: Relay "));
        Serial.println(sched.relayId);
      }
    }
    else if (!shouldBeActive && sched.isActive) {
      // Turn OFF
      if (relayCallback) {
        relayCallback(sched.relayId, false);
      }
      sched.isActive = false;

      if (ENABLE_SERIAL_DEBUG) {
        Serial.print(F("Schedule OFF: Relay "));
        Serial.println(sched.relayId);
      }
    }
  }
}

void ScheduleController::updateTimers(bool* relayStates,
                                       void (*relayCallback)(uint8_t, bool)) {
  unsigned long currentTime = millis();

  for (uint8_t i = 0; i < timerCount; i++) {
    Timer& timer = timers[i];

    if (!timer.enabled) {
      continue;
    }

    // Calculate elapsed time (handle millis() overflow)
    unsigned long elapsed = currentTime - timer.startTime;
    uint32_t elapsedSeconds = elapsed / 1000;

    // Timer expired?
    if (elapsedSeconds >= timer.duration) {
      // Execute timer action
      if (relayCallback) {
        relayCallback(timer.relayId, !timer.turnOffWhenExpired);
      }

      // Disable timer (one-time only)
      timer.enabled = false;

      if (ENABLE_SERIAL_DEBUG) {
        Serial.print(F("Timer expired: Relay "));
        Serial.print(timer.relayId);
        Serial.print(F(" -> "));
        Serial.println(timer.turnOffWhenExpired ? F("OFF") : F("ON"));
      }
    }
  }

  // Compact timer array (remove disabled timers)
  uint8_t writeIndex = 0;
  for (uint8_t readIndex = 0; readIndex < timerCount; readIndex++) {
    if (timers[readIndex].enabled) {
      if (writeIndex != readIndex) {
        timers[writeIndex] = timers[readIndex];
      }
      writeIndex++;
    }
  }
  timerCount = writeIndex;
}

//==============================================================================
// SCHEDULE LOGIC
//==============================================================================

bool ScheduleController::shouldScheduleBeActive(const Schedule& sched,
                                                 uint8_t currentHour,
                                                 uint8_t currentMin,
                                                 uint8_t dayOfWeek) {
  // Check if today is a scheduled day
  if (!isDayMatch(sched, dayOfWeek)) {
    return false;
  }

  // Convert times to minutes since midnight for easier comparison
  uint16_t currentMinutes = currentHour * 60 + currentMin;
  uint16_t onMinutes = sched.onHour * 60 + sched.onMinute;
  uint16_t offMinutes = sched.offHour * 60 + sched.offMinute;

  // Handle schedules that cross midnight
  if (offMinutes < onMinutes) {
    // e.g., ON at 22:00, OFF at 06:00
    return (currentMinutes >= onMinutes) || (currentMinutes < offMinutes);
  } else {
    // Normal schedule within same day
    return (currentMinutes >= onMinutes) && (currentMinutes < offMinutes);
  }
}

bool ScheduleController::isDayMatch(const Schedule& sched, uint8_t dayOfWeek) {
  // dayOfWeek: 1=Monday, 2=Tuesday, ..., 7=Sunday
  // Convert to bitmask
  uint8_t dayBit = 1 << (dayOfWeek - 1);
  return (sched.daysOfWeek & dayBit) != 0;
}

//==============================================================================
// SCHEDULE MANAGEMENT
//==============================================================================

bool ScheduleController::addSchedule(uint8_t relayId,
                                      uint8_t onHour, uint8_t onMin,
                                      uint8_t offHour, uint8_t offMin,
                                      uint8_t days) {
  if (scheduleCount >= MAX_SCHEDULES) {
    return false;
  }

  // Validate parameters
  if (relayId >= RELAY_COUNT || onHour > 23 || offHour > 23 ||
      onMin > 59 || offMin > 59) {
    return false;
  }

  Schedule& sched = schedules[scheduleCount];
  sched.relayId = relayId;
  sched.onHour = onHour;
  sched.onMinute = onMin;
  sched.offHour = offHour;
  sched.offMinute = offMin;
  sched.daysOfWeek = days;
  sched.enabled = true;
  sched.isActive = false;

  scheduleCount++;

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Added schedule #"));
    Serial.print(scheduleCount);
    Serial.print(F(": Relay "));
    Serial.print(relayId);
    Serial.print(F(" ON="));
    Serial.print(onHour);
    Serial.print(F(":"));
    Serial.print(onMin);
    Serial.print(F(" OFF="));
    Serial.print(offHour);
    Serial.print(F(":"));
    Serial.println(offMin);
  }

  return true;
}

bool ScheduleController::removeSchedule(uint8_t index) {
  if (index >= scheduleCount) {
    return false;
  }

  // Shift schedules down
  for (uint8_t i = index; i < scheduleCount - 1; i++) {
    schedules[i] = schedules[i + 1];
  }

  scheduleCount--;
  return true;
}

void ScheduleController::setScheduleEnabled(uint8_t index, bool enabled) {
  if (index < scheduleCount) {
    schedules[index].enabled = enabled;
  }
}

const Schedule* ScheduleController::getSchedule(uint8_t index) const {
  if (index >= scheduleCount) {
    return nullptr;
  }
  return &schedules[index];
}

void ScheduleController::clearAllSchedules() {
  scheduleCount = 0;
}

//==============================================================================
// TIMER MANAGEMENT
//==============================================================================

bool ScheduleController::addTimer(uint8_t relayId,
                                   uint32_t durationSeconds,
                                   bool turnOffWhenExpired) {
  if (timerCount >= MAX_TIMERS) {
    return false;
  }

  if (relayId >= RELAY_COUNT) {
    return false;
  }

  // Cancel existing timer for this relay
  cancelTimer(relayId);

  Timer& timer = timers[timerCount];
  timer.relayId = relayId;
  timer.startTime = millis();
  timer.duration = durationSeconds;
  timer.turnOffWhenExpired = turnOffWhenExpired;
  timer.enabled = true;

  timerCount++;

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Added timer: Relay "));
    Serial.print(relayId);
    Serial.print(F(" -> "));
    Serial.print(turnOffWhenExpired ? F("OFF") : F("ON"));
    Serial.print(F(" in "));
    Serial.print(durationSeconds);
    Serial.println(F(" seconds"));
  }

  return true;
}

bool ScheduleController::cancelTimer(uint8_t relayId) {
  int8_t index = findTimerIndex(relayId);
  if (index >= 0) {
    timers[index].enabled = false;
    return true;
  }
  return false;
}

uint32_t ScheduleController::getTimerRemaining(uint8_t relayId) {
  int8_t index = findTimerIndex(relayId);
  if (index < 0) {
    return 0;
  }

  const Timer& timer = timers[index];
  unsigned long elapsed = millis() - timer.startTime;
  uint32_t elapsedSeconds = elapsed / 1000;

  if (elapsedSeconds >= timer.duration) {
    return 0;
  }

  return timer.duration - elapsedSeconds;
}

const Timer* ScheduleController::getTimer(uint8_t relayId) {
  int8_t index = findTimerIndex(relayId);
  if (index < 0) {
    return nullptr;
  }
  return &timers[index];
}

void ScheduleController::clearAllTimers() {
  timerCount = 0;
}

int8_t ScheduleController::findTimerIndex(uint8_t relayId) {
  for (uint8_t i = 0; i < timerCount; i++) {
    if (timers[i].relayId == relayId && timers[i].enabled) {
      return i;
    }
  }
  return -1;
}

//==============================================================================
// TIME MANAGEMENT (RTC)
//==============================================================================

bool ScheduleController::getCurrentTime(uint8_t* hour, uint8_t* minute,
                                         uint8_t* second, uint8_t* dayOfWeek) {
  if (!rtcAvailable) {
    return false;
  }

  // Uncomment if RTC is available:
  /*
  DateTime now = rtc.now();
  *hour = now.hour();
  *minute = now.minute();
  if (second) *second = now.second();
  if (dayOfWeek) *dayOfWeek = now.dayOfTheWeek(); // 0=Sunday, need to adjust

  return true;
  */

  return false;
}

bool ScheduleController::setCurrentTime(uint8_t hour, uint8_t minute,
                                         uint8_t second, uint8_t dayOfWeek) {
  if (!rtcAvailable) {
    return false;
  }

  // Uncomment if RTC is available:
  /*
  // Create DateTime object for today with specified time
  DateTime now = rtc.now();
  DateTime newTime(now.year(), now.month(), now.day(),
                   hour, minute, second);
  rtc.adjust(newTime);

  Serial.println(F("RTC time updated"));
  return true;
  */

  return false;
}

//==============================================================================
// PERSISTENCE (SD CARD)
//==============================================================================

bool ScheduleController::saveToSD() {
  // Remove old file
  if (SD.exists(FILE_SCHEDULES)) {
    SD.remove(FILE_SCHEDULES);
  }

  // Create data directory if needed
  if (!SD.exists("data")) {
    SD.mkdir("data");
  }

  File file = SD.open(FILE_SCHEDULES, FILE_WRITE);
  if (!file) {
    return false;
  }

  // Write schedule count
  file.write(scheduleCount);

  // Write each schedule
  for (uint8_t i = 0; i < scheduleCount; i++) {
    file.write((uint8_t*)&schedules[i], sizeof(Schedule));
  }

  file.close();

  if (ENABLE_SERIAL_DEBUG) {
    Serial.println(F("Schedules saved to SD"));
  }

  return true;
}

bool ScheduleController::loadFromSD() {
  if (!SD.exists(FILE_SCHEDULES)) {
    return false;
  }

  File file = SD.open(FILE_SCHEDULES, FILE_READ);
  if (!file) {
    return false;
  }

  // Read schedule count
  scheduleCount = file.read();

  if (scheduleCount > MAX_SCHEDULES) {
    scheduleCount = 0;
    file.close();
    return false;
  }

  // Read each schedule
  for (uint8_t i = 0; i < scheduleCount; i++) {
    file.read((uint8_t*)&schedules[i], sizeof(Schedule));
  }

  file.close();

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Loaded "));
    Serial.print(scheduleCount);
    Serial.println(F(" schedules from SD"));
  }

  return true;
}
