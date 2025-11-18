/**
 * SceneController.cpp - Scene Management Implementation
 */

#include "SceneController.h"
#include <string.h>

// Scene names stored in Flash memory
const char SCENE_NAME_GOOD_MORNING[] PROGMEM = "good_morning";
const char SCENE_NAME_GOOD_NIGHT[] PROGMEM = "good_night";
const char SCENE_NAME_AWAY[] PROGMEM = "away";
const char SCENE_NAME_PARTY[] PROGMEM = "party";
const char SCENE_NAME_MOVIE_NIGHT[] PROGMEM = "movie_night";
const char SCENE_NAME_ALL_ON[] PROGMEM = "all_on";
const char SCENE_NAME_ALL_OFF[] PROGMEM = "all_off";

//==============================================================================
// CONSTRUCTOR
//==============================================================================

SceneController::SceneController() : sceneCount(0) {
}

//==============================================================================
// INITIALIZATION
//==============================================================================

void SceneController::begin() {
  initializePredefinedScenes();

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Scene controller initialized with "));
    Serial.print(sceneCount);
    Serial.println(F(" scenes"));
  }
}

void SceneController::initializePredefinedScenes() {
  sceneCount = 0;

  // Scene 1: Good Morning
  // Living Room ON, Kitchen ON, bedrooms OFF
  Scene& goodMorning = scenes[sceneCount++];
  goodMorning.name = SCENE_NAME_GOOD_MORNING;
  goodMorning.relayStates[RELAY_LIVING_ROOM] = true;
  goodMorning.relayStates[RELAY_MASTER_BED] = false;
  goodMorning.relayStates[RELAY_GUEST_ROOM] = false;
  goodMorning.relayStates[RELAY_KITCHEN] = true;
  goodMorning.relayStates[RELAY_WASH_ROOM] = false;
  goodMorning.enabled = true;

  // Scene 2: Good Night
  // All main lights OFF, Wash Room ON (night light)
  Scene& goodNight = scenes[sceneCount++];
  goodNight.name = SCENE_NAME_GOOD_NIGHT;
  goodNight.relayStates[RELAY_LIVING_ROOM] = false;
  goodNight.relayStates[RELAY_MASTER_BED] = false;
  goodNight.relayStates[RELAY_GUEST_ROOM] = false;
  goodNight.relayStates[RELAY_KITCHEN] = false;
  goodNight.relayStates[RELAY_WASH_ROOM] = true;
  goodNight.enabled = true;

  // Scene 3: Away (Security)
  // All lights OFF to save energy
  Scene& away = scenes[sceneCount++];
  away.name = SCENE_NAME_AWAY;
  away.relayStates[RELAY_LIVING_ROOM] = false;
  away.relayStates[RELAY_MASTER_BED] = false;
  away.relayStates[RELAY_GUEST_ROOM] = false;
  away.relayStates[RELAY_KITCHEN] = false;
  away.relayStates[RELAY_WASH_ROOM] = false;
  away.enabled = true;

  // Scene 4: Party
  // All lights ON
  Scene& party = scenes[sceneCount++];
  party.name = SCENE_NAME_PARTY;
  party.relayStates[RELAY_LIVING_ROOM] = true;
  party.relayStates[RELAY_MASTER_BED] = true;
  party.relayStates[RELAY_GUEST_ROOM] = true;
  party.relayStates[RELAY_KITCHEN] = true;
  party.relayStates[RELAY_WASH_ROOM] = true;
  party.enabled = true;

  // Scene 5: Movie Night
  // Living Room ON (dimmed, if supported), others OFF
  Scene& movieNight = scenes[sceneCount++];
  movieNight.name = SCENE_NAME_MOVIE_NIGHT;
  movieNight.relayStates[RELAY_LIVING_ROOM] = true;
  movieNight.relayStates[RELAY_MASTER_BED] = false;
  movieNight.relayStates[RELAY_GUEST_ROOM] = false;
  movieNight.relayStates[RELAY_KITCHEN] = false;
  movieNight.relayStates[RELAY_WASH_ROOM] = false;
  movieNight.enabled = true;

  // Scene 6: All ON
  Scene& allOn = scenes[sceneCount++];
  allOn.name = SCENE_NAME_ALL_ON;
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    allOn.relayStates[i] = true;
  }
  allOn.enabled = true;

  // Scene 7: All OFF
  Scene& allOff = scenes[sceneCount++];
  allOff.name = SCENE_NAME_ALL_OFF;
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    allOff.relayStates[i] = false;
  }
  allOff.enabled = true;
}

//==============================================================================
// SCENE ACTIVATION
//==============================================================================

bool SceneController::activateScene(const char* sceneName,
                                     void (*relayCallback)(uint8_t, bool)) {
  if (sceneName == nullptr || relayCallback == nullptr) {
    return false;
  }

  // Find scene by name
  for (uint8_t i = 0; i < sceneCount; i++) {
    if (scenes[i].enabled && compareSceneName(sceneName, scenes[i].name)) {
      return activateSceneById(i, relayCallback);
    }
  }

  return false;
}

bool SceneController::activateSceneById(uint8_t sceneId,
                                         void (*relayCallback)(uint8_t, bool)) {
  if (sceneId >= sceneCount || !scenes[sceneId].enabled) {
    return false;
  }

  if (relayCallback == nullptr) {
    return false;
  }

  const Scene& scene = scenes[sceneId];

  if (ENABLE_SERIAL_DEBUG) {
    char nameBuf[20];
    strncpy_P(nameBuf, scene.name, sizeof(nameBuf));
    Serial.print(F("Activating scene: "));
    Serial.println(nameBuf);
  }

  // Apply all relay states
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    relayCallback(i, scene.relayStates[i]);
  }

  return true;
}

//==============================================================================
// SCENE QUERIES
//==============================================================================

const Scene* SceneController::getScene(const char* sceneName) const {
  if (sceneName == nullptr) {
    return nullptr;
  }

  for (uint8_t i = 0; i < sceneCount; i++) {
    if (scenes[i].enabled && compareSceneName(sceneName, scenes[i].name)) {
      return &scenes[i];
    }
  }

  return nullptr;
}

const Scene* SceneController::getSceneByIndex(uint8_t index) const {
  if (index >= sceneCount) {
    return nullptr;
  }
  return &scenes[index];
}

//==============================================================================
// CUSTOM SCENES
//==============================================================================

bool SceneController::createCustomScene(const char* name,
                                         const bool* currentStates) {
  // Check if we have space (reserve last slot for custom)
  if (sceneCount >= MAX_SCENES) {
    return false;
  }

  // Validate name
  if (name == nullptr || strlen(name) == 0 || strlen(name) > 15) {
    return false;
  }

  // Check if scene name already exists
  if (getScene(name) != nullptr) {
    return false;  // Name collision
  }

  // Create new scene
  Scene& newScene = scenes[sceneCount];

  // Copy name (note: this should be allocated separately for custom scenes)
  // For simplicity, we're using stack/global storage
  newScene.name = name;  // WARNING: name must persist!

  // Copy current states
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    newScene.relayStates[i] = currentStates[i];
  }

  newScene.enabled = true;
  sceneCount++;

  if (ENABLE_SERIAL_DEBUG) {
    Serial.print(F("Created custom scene: "));
    Serial.println(name);
  }

  return true;
}

//==============================================================================
// JSON EXPORT
//==============================================================================

void SceneController::listScenesJSON(char* buffer, uint16_t maxLen) {
  uint16_t pos = 0;

  // Start JSON array
  pos += snprintf(buffer + pos, maxLen - pos, "[");

  for (uint8_t i = 0; i < sceneCount; i++) {
    if (!scenes[i].enabled) {
      continue;
    }

    if (i > 0) {
      pos += snprintf(buffer + pos, maxLen - pos, ",");
    }

    // Read scene name from PROGMEM
    char nameBuf[20];
    strncpy_P(nameBuf, scenes[i].name, sizeof(nameBuf));

    pos += snprintf(buffer + pos, maxLen - pos,
                    "{\"id\":%d,\"name\":\"%s\"}", i, nameBuf);

    if (pos >= maxLen - 50) break;  // Prevent buffer overflow
  }

  pos += snprintf(buffer + pos, maxLen - pos, "]");
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

bool SceneController::compareSceneName(const char* name1,
                                        const char* name2) const {
  // Compare with PROGMEM string
  char buf[20];
  strncpy_P(buf, name2, sizeof(buf));

  // Case-insensitive comparison
  return strcasecmp(name1, buf) == 0;
}
