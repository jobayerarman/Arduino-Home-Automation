/**
 * AutomationController.h - Temperature-based Automation System
 * Automatically controls relays based on temperature thresholds
 */

#ifndef AUTOMATION_CONTROLLER_H
#define AUTOMATION_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

//==============================================================================
// AUTOMATION RULE STRUCTURE
//==============================================================================

/**
 * Trigger types for automation rules
 */
enum class TriggerType : uint8_t {
  TEMP_HIGH,        // Trigger when temperature exceeds threshold
  TEMP_LOW,         // Trigger when temperature drops below threshold
  TEMP_RANGE,       // Trigger when temperature in range
  MANUAL_ONLY,      // Automation disabled
  MOTION_DETECTED,  // Trigger on motion (future)
  LIGHT_LOW         // Trigger when light level low (future)
};

/**
 * Action to perform when triggered
 */
enum class ActionType : uint8_t {
  TURN_ON,          // Turn relay ON
  TURN_OFF,         // Turn relay OFF
  TOGGLE            // Toggle relay state
};

/**
 * Automation Rule Structure
 * Defines conditions and actions for automated relay control
 */
struct AutomationRule {
  uint8_t relayId;              // Which relay to control
  TriggerType triggerType;      // What triggers this rule
  ActionType action;            // What to do when triggered
  uint8_t threshold;            // Temperature threshold (°C)
  uint8_t thresholdHigh;        // Upper threshold for TEMP_RANGE
  bool enabled;                 // Is this rule active?
  bool currentlyTriggered;      // Is condition currently met?
  uint8_t hysteresis;           // Temperature hysteresis (°C)
  unsigned long lastTriggerTime; // Last time rule was triggered
};

//==============================================================================
// AUTOMATION CONTROLLER CLASS
//==============================================================================

class AutomationController {
public:
  /**
   * Constructor
   */
  AutomationController();

  /**
   * Initialize automation system
   * Load rules from SD card if available
   */
  void begin();

  /**
   * Evaluate all automation rules
   * Call this periodically (e.g., every 5 seconds)
   *
   * @param currentTemp Current temperature reading
   * @param relayStates Current relay states array
   * @param relayCallback Function to call to change relay state
   */
  void evaluate(uint8_t currentTemp,
                bool* relayStates,
                void (*relayCallback)(uint8_t relayId, bool state));

  /**
   * Add a new automation rule
   *
   * @param relayId Relay to control (0-4)
   * @param triggerType Type of trigger
   * @param action Action to perform
   * @param threshold Temperature threshold
   * @return true if added successfully, false if no space
   */
  bool addRule(uint8_t relayId,
               TriggerType triggerType,
               ActionType action,
               uint8_t threshold);

  /**
   * Remove automation rule by index
   *
   * @param index Rule index (0 to ruleCount-1)
   * @return true if removed successfully
   */
  bool removeRule(uint8_t index);

  /**
   * Enable/disable specific rule
   *
   * @param index Rule index
   * @param enabled New enabled state
   */
  void setRuleEnabled(uint8_t index, bool enabled);

  /**
   * Enable/disable ALL automation
   *
   * @param enabled New enabled state
   */
  void setAutomationEnabled(bool enabled);

  /**
   * Get automation rule by index
   *
   * @param index Rule index
   * @return Pointer to rule, or nullptr if invalid
   */
  const AutomationRule* getRule(uint8_t index) const;

  /**
   * Get number of active rules
   */
  uint8_t getRuleCount() const { return ruleCount; }

  /**
   * Check if automation is enabled globally
   */
  bool isEnabled() const { return automationEnabled; }

  /**
   * Save automation rules to SD card
   *
   * @return true if saved successfully
   */
  bool saveToSD();

  /**
   * Load automation rules from SD card
   *
   * @return true if loaded successfully
   */
  bool loadFromSD();

  /**
   * Clear all automation rules
   */
  void clearAllRules();

  /**
   * Get trigger type name as string
   *
   * @param type Trigger type
   * @param buffer Buffer to store string
   * @param maxLen Maximum buffer length
   */
  static void getTriggerTypeName(TriggerType type, char* buffer, uint8_t maxLen);

private:
  AutomationRule rules[MAX_AUTOMATION_RULES];
  uint8_t ruleCount;
  bool automationEnabled;
  uint8_t lastTemperature;

  /**
   * Check if rule condition is met
   *
   * @param rule Rule to check
   * @param currentTemp Current temperature
   * @return true if condition met
   */
  bool evaluateCondition(const AutomationRule& rule, uint8_t currentTemp);

  /**
   * Execute rule action
   *
   * @param rule Rule to execute
   * @param relayStates Current relay states
   * @param relayCallback Function to call to change state
   */
  void executeAction(const AutomationRule& rule,
                     bool* relayStates,
                     void (*relayCallback)(uint8_t, bool));
};

#endif // AUTOMATION_CONTROLLER_H
