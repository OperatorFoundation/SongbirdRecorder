/*
 * SerialManager.ino
 * 
 * Serial command interface for manual file management
 * 
 * Commands:
 * - LIST: Show all recordings
 * - DELETE filename.wav: Delete a specific recording
 * - HELP: Show available commands
 * - STATUS: Show system status
 */

 #include "SongbirdRecorder.h"

 void handleSerialCommands()
 {
  if (!Serial.available()) return;

  // Read command for serial
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  Serial.print("Command received: ");
  Serial.println(command);

  // Process commands
  if (command == "LIST")
  {
    listFiles();
  }
  else if (command == "DELETE")
  {
    String filename = command.substring(7);
    filename.trim();
    deleteFile(filename);
  }
  else if (command == "HELP")
  {
    showHelp();
  }
  else if (command == "STATUS")
  {
    showStatus();
  }
  else if (command == "SCAN")
  {
    Serial.println("Rescanning SD card...");
    scanForFiles();
  }
  else if (command == "INIT") 
  {
    Serial.println("Reinitializing SD card...");
    initializeSDCard();
  }
  else if (command == "")
  {
    // Ignore empty commands
  }
  else
  {
    Serial.print("Unknown command: ");
    Serial.println(command);
    Serial.println("Type HELP for available commands");
  }
 }

 void showHelp()
 {
  Serial.println("=== Songbird Call Recorder Commands ===");
  Serial.println("LIST                  - List all recordings");
  Serial.println("DELETE filename.wav   - Delete a recording");
  Serial.println("STATUS                - Show system status");
  Serial.println("SCAN                  - Rescan SD card for files");
  Serial.println("INIT                  - Reinitialize SD card");
  Serial.println("HELP                  - Show this help");
  Serial.println("");
  Serial.println("Button Controls:");
  Serial.println("UP    - Start/Stop recording");
  Serial.println("DOWN  - Play/Pause current recording");
  Serial.println("LEFT  - Previous recording");
  Serial.println("RIGHT - Next recording");
 }

void showStatus()
{
  Serial.println("=== System Status ===");
  
  // Current state
  Serial.print("State: ");
  switch (currentState) {
    case STATE_IDLE:
      Serial.println("IDLE");
      break;
    case STATE_RECORDING:
      Serial.print("RECORDING (");
      Serial.print((millis() - recordStartTime) / 1000);
      Serial.println(" seconds)");
      break;
    case STATE_PLAYBACK:
      Serial.println("PLAYBACK");
      break;
  }
  
  // SD card status
  Serial.print("SD Card: ");
  if (sdCardReady) {
    Serial.println("Ready");
  } else {
    Serial.println("Not Ready");
  }
  
  // File information
  Serial.print("Total Files: ");
  Serial.println(totalFiles);
  
  if (totalFiles > 0) {
    Serial.print("Current File: [");
    Serial.print(currentFileIndex + 1);
    Serial.print("/");
    Serial.print(totalFiles);
    Serial.print("] ");
    Serial.println(currentFilename);
  }
  
  // Audio level
  Serial.print("Audio Level: ");
  Serial.println(getInputLevel());
  
  // Memory usage
  Serial.print("Audio Memory: ");
  Serial.print(AudioMemoryUsage());
  Serial.print("/");
  Serial.print(AudioMemoryUsageMax());
  Serial.println(" blocks");
}
