/**
 * SceneController.h - Scene Management System
 * Preset configurations for common scenarios
 */

#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

//==============================================================================
// SCENE STRUCTURE
//==============================================================================

/**
 * Scene Definition
 * Stores desired state for all relays
 */
struct Scene {
  const char* name;             // Scene name (stored in PROGMEM)
  bool relayStates[RELAY_COUNT]; // Desired state for each relay
  bool enabled;                 // Is this scene available?
};

//==============================================================================
// SCENE CONTROLLER CLASS
//==============================================================================

class SceneController {
public:
  /**
   * Constructor
   */
  SceneController();

  /**
   * Initialize scene system
   */
  void begin();

  /**
   * Activate a scene by name
   *
   * @param sceneName Scene name to activate
   * @param relayCallback Function to call to change relay states
   * @return true if scene found and activated
   */
  bool activateScene(const char* sceneName,
                     void (*relayCallback)(uint8_t relayId, bool state));

  /**
   * Activate a scene by ID
   *
   * @param sceneId Scene ID
   * @param relayCallback Function to call to change relay states
   * @return true if scene activated
   */
  bool activateSceneById(uint8_t sceneId,
                         void (*relayCallback)(uint8_t relayId, bool state));

  /**
   * Get scene by name
   *
   * @param sceneName Scene name
   * @return Pointer to scene, or nullptr if not found
   */
  const Scene* getScene(const char* sceneName) const;

  /**
   * Get scene by index
   *
   * @param index Scene index (0 to sceneCount-1)
   * @return Pointer to scene, or nullptr if invalid
   */
  const Scene* getSceneByIndex(uint8_t index) const;

  /**
   * Get number of available scenes
   */
  uint8_t getSceneCount() const { return sceneCount; }

  /**
   * Create custom scene from current relay states
   *
   * @param name Custom scene name (max 15 chars)
   * @param currentStates Current relay states to save
   * @return true if created successfully
   */
  bool createCustomScene(const char* name, const bool* currentStates);

  /**
   * List all available scenes (for web interface)
   *
   * @param buffer Buffer to store JSON array
   * @param maxLen Maximum buffer length
   */
  void listScenesJSON(char* buffer, uint16_t maxLen);

private:
  static const uint8_t MAX_CUSTOM_SCENES = 3;
  Scene scenes[MAX_SCENES];
  uint8_t sceneCount;

  /**
   * Initialize predefined scenes
   */
  void initializePredefinedScenes();

  /**
   * Compare strings (case-insensitive)
   */
  bool compareSceneName(const char* name1, const char* name2) const;
};

#endif // SCENE_CONTROLLER_H
