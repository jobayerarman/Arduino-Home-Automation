/**
 * AutomationController.cpp - Implementation of temperature-based automation
 */

#include "AutomationController.h"
#include <SD.h>

//==============================================================================
// CONSTRUCTOR
//==============================================================================

AutomationController::AutomationController()
  : ruleCount(0), automationEnabled(true), lastTemperature(0) {
  // Initialize all rules to disabled
  for (uint8_t i = 0; i < MAX_AUTOMATION_RULES; i++) {
    rules[i].enabled = false;
    rules[i].currentlyTriggered = false;
  }
}

//==============================================================================
// INITIALIZATION
//==============================================================================

void AutomationController::begin() {
  // Try to load rules from SD card
  if (!loadFromSD()) {
    // If no saved rules, create default rules
    if (ENABLE_SERIAL_DEBUG) {
      Serial.println(F("No saved automation rules, using defaults"));
    }

    // Default Rule 1: Turn ON Guest Room Fan when temp > 28°C
    addRule(RELAY_GUEST_ROOM, TriggerType::TEMP_HIGH,
            ActionType::TURN_ON, TEMP_THRESHOLD_HIGH);

    // Default Rule 2: Turn OFF Guest Room Fan when temp < 26°C (with hysteresis)
    addRule(RELAY_GUEST_ROOM, TriggerType::TEMP_LOW,
            ActionType::TURN_OFF, TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS);
  }
}

//==============================================================================
// RULE EVALUATION
//==============================================================================

void AutomationController::evaluate(uint8_t currentTemp,
                                     bool* relayStates,
                                     void (*relayCallback)(uint8_t, bool)) {
  // Skip if automation is disabled globally
  if (!automationEnabled) {
    return;
  }

  lastTemperature = currentTemp;

  // Evaluate each rule
  for (uint8_t i = 0; i < ruleCount; i++) {
    AutomationRule& rule = rules[i];

    // Skip disabled rules
    if (!rule.enabled) {
      continue;
    }

    // Check if condition is met
    bool conditionMet = evaluateCondition(rule, currentTemp);

    // Execute action if condition changed from false to true
    // (avoids repeated execution)
    if (conditionMet && !rule.currentlyTriggered) {
      executeAction(rule, relayStates, relayCallback);
      rule.currentlyTriggered = true;
      rule.lastTriggerTime = millis();

      if (ENABLE_SERIAL_DEBUG) {
        Serial.print(F("Automation triggered: Relay "));
        Serial.print(rule.relayId);
        Serial.print(F(" - Temp: "));
        Serial.println(currentTemp);
      }
    }
    else if (!conditionMet && rule.currentlyTriggered) {
      // Reset trigger state when condition no longer met
      rule.currentlyTriggered = false;
    }
  }
}

bool AutomationController::evaluateCondition(const AutomationRule& rule,
                                              uint8_t currentTemp) {
  switch (rule.triggerType) {
    case TriggerType::TEMP_HIGH:
      // Trigger when temperature exceeds threshold
      if (rule.currentlyTriggered) {
        // Add hysteresis: Turn OFF when temp drops below (threshold - hysteresis)
        return currentTemp >= (rule.threshold - rule.hysteresis);
      } else {
        // Turn ON when temp exceeds threshold
        return currentTemp >= rule.threshold;
      }

    case TriggerType::TEMP_LOW:
      // Trigger when temperature drops below threshold
      if (rule.currentlyTriggered) {
        // Add hysteresis: Turn OFF when temp rises above (threshold + hysteresis)
        return currentTemp <= (rule.threshold + rule.hysteresis);
      } else {
        // Turn ON when temp drops below threshold
        return currentTemp <= rule.threshold;
      }

    case TriggerType::TEMP_RANGE:
      // Trigger when temperature in range
      return (currentTemp >= rule.threshold) &&
             (currentTemp <= rule.thresholdHigh);

    case TriggerType::MANUAL_ONLY:
    default:
      return false;
  }
}

void AutomationController::executeAction(const AutomationRule& rule,
                                          bool* relayStates,
                                          void (*relayCallback)(uint8_t, bool)) {
  if (rule.relayId >= RELAY_COUNT || relayCallback == nullptr) {
    return;
  }

  bool newState;

  switch (rule.action) {
    case ActionType::TURN_ON:
      newState = true;
      break;

    case ActionType::TURN_OFF:
      newState = false;
      break;

    case ActionType::TOGGLE:
      newState = !relayStates[rule.relayId];
      break;

    default:
      return;
  }

  // Execute the relay state change via callback
  relayCallback(rule.relayId, newState);
}

//==============================================================================
// RULE MANAGEMENT
//==============================================================================

bool AutomationController::addRule(uint8_t relayId,
                                    TriggerType triggerType,
                                    ActionType action,
                                    uint8_t threshold) {
  // Check if we have space
  if (ruleCount >= MAX_AUTOMATION_RULES) {
    return false;
  }

  // Validate relay ID
  if (relayId >= RELAY_COUNT) {
    return false;
  }

  // Create new rule
  AutomationRule& rule = rules[ruleCount];
  rule.relayId = relayId;
  rule.triggerType = triggerType;
  rule.action = action;
  rule.threshold = threshold;
  rule.thresholdHigh = threshold + 10;  // Default range
  rule.enabled = true;
  rule.currentlyTriggered = false;
  rule.hysteresis = TEMP_HYSTERESIS;
  rule.lastTriggerTime = 0;

  ruleCount++;

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Added automation rule #"));
    Serial.print(ruleCount);
    Serial.print(F(" for relay "));
    Serial.println(relayId);
  }

  return true;
}

bool AutomationController::removeRule(uint8_t index) {
  if (index >= ruleCount) {
    return false;
  }

  // Shift all rules after this one down
  for (uint8_t i = index; i < ruleCount - 1; i++) {
    rules[i] = rules[i + 1];
  }

  ruleCount--;
  return true;
}

void AutomationController::setRuleEnabled(uint8_t index, bool enabled) {
  if (index < ruleCount) {
    rules[index].enabled = enabled;
  }
}

void AutomationController::setAutomationEnabled(bool enabled) {
  automationEnabled = enabled;

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Automation globally "));
    Serial.println(enabled ? F("enabled") : F("disabled"));
  }
}

const AutomationRule* AutomationController::getRule(uint8_t index) const {
  if (index >= ruleCount) {
    return nullptr;
  }
  return &rules[index];
}

void AutomationController::clearAllRules() {
  ruleCount = 0;
  for (uint8_t i = 0; i < MAX_AUTOMATION_RULES; i++) {
    rules[i].enabled = false;
  }
}

//==============================================================================
// PERSISTENCE (SD CARD)
//==============================================================================

bool AutomationController::saveToSD() {
  // Remove old file
  if (SD.exists(FILE_AUTOMATION)) {
    SD.remove(FILE_AUTOMATION);
  }

  // Create data directory if it doesn't exist
  if (!SD.exists("data")) {
    SD.mkdir("data");
  }

  File file = SD.open(FILE_AUTOMATION, FILE_WRITE);
  if (!file) {
    return false;
  }

  // Write rule count
  file.write(ruleCount);

  // Write each rule
  for (uint8_t i = 0; i < ruleCount; i++) {
    file.write((uint8_t*)&rules[i], sizeof(AutomationRule));
  }

  file.close();

  if (ENABLE_SERIAL_DEBUG) {
    Serial.println(F("Automation rules saved to SD"));
  }

  return true;
}

bool AutomationController::loadFromSD() {
  if (!SD.exists(FILE_AUTOMATION)) {
    return false;
  }

  File file = SD.open(FILE_AUTOMATION, FILE_READ);
  if (!file) {
    return false;
  }

  // Read rule count
  ruleCount = file.read();

  // Validate rule count
  if (ruleCount > MAX_AUTOMATION_RULES) {
    ruleCount = 0;
    file.close();
    return false;
  }

  // Read each rule
  for (uint8_t i = 0; i < ruleCount; i++) {
    file.read((uint8_t*)&rules[i], sizeof(AutomationRule));
  }

  file.close();

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Loaded "));
    Serial.print(ruleCount);
    Serial.println(F(" automation rules from SD"));
  }

  return true;
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

void AutomationController::getTriggerTypeName(TriggerType type,
                                               char* buffer,
                                               uint8_t maxLen) {
  switch (type) {
    case TriggerType::TEMP_HIGH:
      strncpy_P(buffer, PSTR("Temp High"), maxLen);
      break;
    case TriggerType::TEMP_LOW:
      strncpy_P(buffer, PSTR("Temp Low"), maxLen);
      break;
    case TriggerType::TEMP_RANGE:
      strncpy_P(buffer, PSTR("Temp Range"), maxLen);
      break;
    case TriggerType::MANUAL_ONLY:
      strncpy_P(buffer, PSTR("Manual Only"), maxLen);
      break;
    case TriggerType::MOTION_DETECTED:
      strncpy_P(buffer, PSTR("Motion"), maxLen);
      break;
    case TriggerType::LIGHT_LOW:
      strncpy_P(buffer, PSTR("Light Low"), maxLen);
      break;
    default:
      strncpy_P(buffer, PSTR("Unknown"), maxLen);
  }
}
