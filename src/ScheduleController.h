/**
 * ScheduleController.h - Time-based Scheduling System
 * Schedule relay actions based on time of day
 * Requires RTC module (DS3231 or DS1307)
 */

#ifndef SCHEDULE_CONTROLLER_H
#define SCHEDULE_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

//==============================================================================
// SCHEDULE STRUCTURES
//==============================================================================

/**
 * Schedule Entry
 * Defines when to turn relay ON and OFF
 */
struct Schedule {
  uint8_t relayId;              // Which relay to control
  uint8_t onHour;               // Hour to turn ON (0-23)
  uint8_t onMinute;             // Minute to turn ON (0-59)
  uint8_t offHour;              // Hour to turn OFF (0-23)
  uint8_t offMinute;            // Minute to turn OFF (0-59)
  uint8_t daysOfWeek;           // Bitmask: Mon=0x01, Tue=0x02, etc.
  bool enabled;                 // Is this schedule active?
  bool isActive;                // Is schedule currently ON?
};

/**
 * Timer Entry
 * One-time countdown timer
 */
struct Timer {
  uint8_t relayId;              // Which relay to control
  unsigned long startTime;      // When timer started (millis())
  uint32_t duration;            // Duration in seconds
  bool turnOffWhenExpired;      // true=OFF, false=ON when timer expires
  bool enabled;                 // Is this timer active?
};

//==============================================================================
// SCHEDULE CONTROLLER CLASS
//==============================================================================

class ScheduleController {
public:
  /**
   * Constructor
   */
  ScheduleController();

  /**
   * Initialize scheduling system
   * Sets up RTC communication
   *
   * @return true if RTC initialized successfully
   */
  bool begin();

  /**
   * Check all schedules and timers
   * Call this every second in main loop
   *
   * @param relayStates Current relay states array
   * @param relayCallback Function to call to change relay state
   */
  void update(bool* relayStates,
              void (*relayCallback)(uint8_t relayId, bool state));

  /**
   * Add a new schedule
   *
   * @param relayId Relay to control
   * @param onHour Hour to turn ON (0-23)
   * @param onMin Minute to turn ON (0-59)
   * @param offHour Hour to turn OFF (0-23)
   * @param offMin Minute to turn OFF (0-59)
   * @param days Days of week bitmask (e.g., SCHEDULE_DAILY)
   * @return true if added successfully
   */
  bool addSchedule(uint8_t relayId,
                   uint8_t onHour, uint8_t onMin,
                   uint8_t offHour, uint8_t offMin,
                   uint8_t days = SCHEDULE_DAILY);

  /**
   * Remove schedule by index
   *
   * @param index Schedule index (0 to scheduleCount-1)
   * @return true if removed successfully
   */
  bool removeSchedule(uint8_t index);

  /**
   * Enable/disable specific schedule
   *
   * @param index Schedule index
   * @param enabled New enabled state
   */
  void setScheduleEnabled(uint8_t index, bool enabled);

  /**
   * Add a countdown timer
   *
   * @param relayId Relay to control
   * @param durationSeconds Duration in seconds
   * @param turnOffWhenExpired true=turn OFF, false=turn ON when expired
   * @return true if added successfully
   */
  bool addTimer(uint8_t relayId,
                uint32_t durationSeconds,
                bool turnOffWhenExpired = true);

  /**
   * Cancel timer for specific relay
   *
   * @param relayId Relay ID
   * @return true if timer was cancelled
   */
  bool cancelTimer(uint8_t relayId);

  /**
   * Get remaining time for relay's timer
   *
   * @param relayId Relay ID
   * @return Remaining seconds, or 0 if no active timer
   */
  uint32_t getTimerRemaining(uint8_t relayId);

  /**
   * Get schedule by index
   *
   * @param index Schedule index
   * @return Pointer to schedule, or nullptr if invalid
   */
  const Schedule* getSchedule(uint8_t index) const;

  /**
   * Get timer by relay ID
   *
   * @param relayId Relay ID
   * @return Pointer to timer, or nullptr if no timer
   */
  const Timer* getTimer(uint8_t relayId);

  /**
   * Get number of active schedules
   */
  uint8_t getScheduleCount() const { return scheduleCount; }

  /**
   * Get number of active timers
   */
  uint8_t getTimerCount() const { return timerCount; }

  /**
   * Save schedules to SD card
   */
  bool saveToSD();

  /**
   * Load schedules from SD card
   */
  bool loadFromSD();

  /**
   * Clear all schedules
   */
  void clearAllSchedules();

  /**
   * Clear all timers
   */
  void clearAllTimers();

  /**
   * Get current time from RTC
   *
   * @param hour Pointer to store hour (0-23)
   * @param minute Pointer to store minute (0-59)
   * @param second Pointer to store second (0-59)
   * @param dayOfWeek Pointer to store day of week (1=Monday, 7=Sunday)
   * @return true if time read successfully
   */
  bool getCurrentTime(uint8_t* hour, uint8_t* minute,
                      uint8_t* second = nullptr,
                      uint8_t* dayOfWeek = nullptr);

  /**
   * Set current time on RTC
   *
   * @param hour Hour (0-23)
   * @param minute Minute (0-59)
   * @param second Second (0-59)
   * @param dayOfWeek Day of week (1=Monday, 7=Sunday)
   * @return true if time set successfully
   */
  bool setCurrentTime(uint8_t hour, uint8_t minute,
                      uint8_t second = 0,
                      uint8_t dayOfWeek = 1);

  /**
   * Check if RTC is available
   */
  bool isRTCAvailable() const { return rtcAvailable; }

private:
  Schedule schedules[MAX_SCHEDULES];
  Timer timers[MAX_TIMERS];
  uint8_t scheduleCount;
  uint8_t timerCount;
  bool rtcAvailable;
  uint8_t lastMinute;           // For detecting minute changes

  /**
   * Check if schedule should be active now
   *
   * @param schedule Schedule to check
   * @param currentHour Current hour
   * @param currentMin Current minute
   * @param dayOfWeek Current day of week
   * @return true if should be ON
   */
  bool shouldScheduleBeActive(const Schedule& schedule,
                               uint8_t currentHour,
                               uint8_t currentMin,
                               uint8_t dayOfWeek);

  /**
   * Check if current day matches schedule
   *
   * @param schedule Schedule to check
   * @param dayOfWeek Current day (1=Monday, 7=Sunday)
   * @return true if day matches
   */
  bool isDayMatch(const Schedule& schedule, uint8_t dayOfWeek);

  /**
   * Check and update all timers
   */
  void updateTimers(bool* relayStates,
                    void (*relayCallback)(uint8_t, bool));

  /**
   * Find timer index for relay
   *
   * @param relayId Relay ID
   * @return Timer index, or -1 if not found
   */
  int8_t findTimerIndex(uint8_t relayId);
};

#endif // SCHEDULE_CONTROLLER_H
