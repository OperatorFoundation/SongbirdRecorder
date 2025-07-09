/*
* ButtonManager.ino
* 
* Button input handling with debouncing
* 
* Button Functions:
* - UP: Start/Stop recording
* - DOWN: Play/Pause current recording
* - LEFT: Previous recording
* - RIGHT: Next recording
*/

#include "SongbirdRecorder.h"

// Button state tracking for debouncing
static bool lastUpState = HIGH;
static bool lastDownState = HIGH;
static bool lastLeftState = HIGH;
static bool lastRightState = HIGH;
static unsigned long lastButtonTime = 0;

void handleButtons()
{
  // Simple debouncing - ignore button presses for a short time
  if (millis() - lastButtonTime < BUTTON_DEBOUNCE_MS) return;

  // Read current button states
  bool upPressed = (digitalRead(BTN_UP_PIN) == LOW && lastUpState == HIGH);
  bool downPressed = (digitalRead(BTN_DOWN_PIN) == LOW && lastDownState == HIGH);
  bool leftPressed = (digitalRead(BTN_LEFT_PIN) == LOW && lastLeftState == HIGH);
  bool rightPressed = (digitalRead(BTN_RIGHT_PIN) == LOW && lastRightState == HIGH);
  
  // Update last button states
  lastUpState = digitalRead(BTN_UP_PIN);
  lastDownState = digitalRead(BTN_DOWN_PIN);
  lastLeftState = digitalRead(BTN_LEFT_PIN);
  lastRightState = digitalRead(BTN_RIGHT_PIN);

  // Handle button presses
  if (upPressed) 
  {
    handleUpButton();
    lastButtonTime = millis();
  }

  if (downPressed) 
  {
    handleDownButton();
    lastButtonTime = millis();
  }

  if (leftPressed) 
  {
    handleLeftButton();
    lastButtonTime = millis();
  }

  if (rightPressed) 
  {
    handleRightButton();
    lastButtonTime = millis();
  }
}

void handleUpButton()
{
  Serial.println("UP button pressed");

  switch (currentState)
  {
    case STATE_IDLE:
      // Start recording
      startRecording();
      break;

    case STATE_RECORDING:
      // Stop recording
      stopRecording();
      break;

    case STATE_PLAYBACK:
      // Stop playback
      stopPlayback();
      break;
  }
}

void handleDownButton()
{
  Serial.println("DOWN button pressed");

  switch (currentState) 
  {
    case STATE_IDLE:
      // Start playback if files available
      if (totalFiles > 0)
      {
        startPlayback();
      }
      else 
      {
        Serial.println("No files to play");
      }
      break;

    case STATE_RECORDING:
      // Cannot change to playback while recording
      Serial.println("Cannot play while recording");
      break;

    case STATE_PLAYBACK:
      // Stop playback
      stopPlayback();
      break;
  }
}

void handleLeftButton()
{
  Serial.println("Left button pressed");

  // Only navigate files when idle
  if (currentState != STATE_IDLE) 
  {
    Serial.println("Cannot navigate while recording/playing");
    return;
  }

  // Navigate to previous file
  if (totalFiles > 0)
  {
    currentFileIndex--;
    if (currentFileIndex < 0)
    {
      currentFileIndex = totalFiles - 1; // Wrap to the last file
    }

    loadCurrentFilename();
    Serial.print("Selected file: ");
    Serial.println(currentFilename);
  }
  else
  {
    Serial.println("No files available");
  }
}

void handleRightButton()
{
  Serial.println("RIGHT button pressed");

  // Only navigate files when idle
  if (currentState != STATE_IDLE) 
  {
    Serial.println("Cannot navigate while recording/playing");
    return;
  }

  // Navigate to next file
  if (totalFiles > 0)
  {
    currentFileIndex++;
    if (currentFileIndex >= totalFiles)
    {
      currentFileIndex = 0; // Wrap to the first file
    }

    loadCurrentFilename();

    Serial.print("Selected file: ");
    Serial.println(currentFilename);
  }
  else 
  {
    Serial.println("No files available");
  }
}