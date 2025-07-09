/*
 * AudioManager.ino
 * 
 * Audio processing and routing for phone call recording
 * 
 * Audio Flow:
 * - Phone audio (USB) → headset output (for user to hear)
 * - Headset mic → phone output (user's voice to phone)
 * - Both streams → mixer → recording queue → SD card
 * - Playback → headset output (to hear recordings)
 */

 #include "SongbirdRecorder.h"

 // Audio connections - Phone call routing
 AudioConnection patchCord1(inputFromPhone, 0, headsetMixer, 0);    // Phone left to headset mixer
 AudioConnection patchCord2(inputFromPhone, 1, headsetMixer, 1);    // Phone right to headset mixer
 AudioConnection patchCord3(headsetMixer, 0, outputToHeadset);      // Headset mixer to headset left
 AudioConnection patchCord4(headsetMixer, 0, outputToHeadset, 1);   // Headset mixer to headset right

 // Audio connections - Microphone passthrough
 AudioConnection patchCord5(inputFromHeadset, 0, outputToPhone, 0); // Headset mic to phone

 // Audio connections - Recording (capture both sides of the conversation)
 AudioConnection patchCord6(inputFromPhone, 0, phoneMixer, 0);      // Phone left for recording
 AudioConnection patchCord7(inputFromPhone, 1, phoneMixer, 1);      // Phone right for recording
 AudioConnection patchCord8(inputFromHeadset, 0, phoneMixer, 2);    // Headset mic for recording
 AudioConnection patchCord9(phoneMixer, 0, recordQueue, 0);         // Mixed audio to recorder
 AudioConnection patchCord10(phoneMixer, 0, inputLevel, 0);         // Audio level monitoring

 // Audio connections - Playback
 AudioConnection patchCord11(playWav, 0, headsetMixer, 2);          // Playback left to headset mixer
 AudioConnection patchCord12(playWav, 1, headsetMixer, 3);          // Playback right to headset mixer

void setupAudioProcessing()
{
  Serial.println("Initializing audio processing...");

  // Allocate audio memory
  AudioMemory(AUDIO_MEMORY_BLOCKS);

  // Initialize audio shield
  audioShield.enable();
  audioShield.volume(0.5);
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.adcHighPassFilterEnable();

  // Wait for audio shield to stabilize
  delay(100);

  // Configure headset mixer (what user hears)
  headsetMixer.gain(0, PHONE_AUDIO_LEVEL);    // Phone left channel
  headsetMixer.gain(1, PHONE_AUDIO_LEVEL);    // Phone right channel
  headsetMixer.gain(2, PLAYBACK_AUDIO_LEVEL); // Playback left
  headsetMixer.gain(3, PLAYBACK_AUDIO_LEVEL); // Playback right

  // Configure phone mixer (what gets recorded)
  phoneMixer.gain(0, RECORDING_MIX_LEVEL);    // Phone left
  phoneMixer.gain(1, RECORDING_MIX_LEVEL);    // Phone right
  phoneMixer.gain(2, RECORDING_MIX_LEVEL);    // Headset mic
  phoneMixer.gain(3, 0.0);                    // Unused

  Serial.println("Audio processing initialized");
}

void startRecording()
{
  if (!sdCardReady) 
  {
    Serial.println("Cannot record: The SD card is not ready");
    return;
  }

  if (currentState != STATE_IDLE) 
  {
    Serial.println("Cannot record: Already recording or playing");
    return;
  }

  // Generate filename with current timestamp
  String filename = generateRecordingFilename();

  // Create WAV file
  createWAVFile(filename);

  // Start recording queue
  if (recordQueue.begin()) 
  {
    currentState = STATE_RECORDING;
    recordStartTime = millis();

    Serial.print("Recording started: ");
    Serial.println(filename);
  }
  else 
  {
    Serial.println("Failed to start the recording queue");

    if (recordingFile) 
    {
      recordingFile.close();
    }
  }
}

void stopRecording()
{
  if (currentState != STATE_RECORDING) 
  {
    return;
  }

  // Stop the recording queue
  recordQueue.end();

  // Finalize the WAV file
  finalizeWAVFile();

  // Update state
  currentState = STATE_IDLE;

  // Rescan for files
  scanForFiles();

  Serial.println("Recording stopped");
}

void handleRecording()
{
  // Check if we have reached max recording time
  if (millis() - recordStartTime > MAX_RECORDING_TIME_MS) 
  {
    Serial.println("Maximum recording time reached");
    stopRecording();
    return;
  }

  // Process available audio data

  // recordQueue.available() returns how many audio blocks are ready to be read
  // We want to process 2 blocks at a time
  while (recordQueue.available() >= 2) 
  {
    // Get audio data from the queue

    // Temporary buffer that can hold 2 audio blocks (256 + 256 = 512 bytes)
    byte buffer[RECORDING_BUFFER_SIZE];

    // Copy first buffer
    memcpy(buffer, recordQueue.readBuffer(), AUDIO_BLOCK_SIZE);
    recordQueue.freeBuffer();

    // Copy second audio block
    memcpy(buffer + AUDIO_BLOCK_SIZE, recordQueue.readBuffer(), AUDIO_BLOCK_SIZE);
    recordQueue.freeBuffer();

    // Write to WAV file
    writeWAVData(buffer, RECORDING_BUFFER_SIZE);
  }
}

void startPlayback()
{
  if (!sdCardReady || totalFiles == 0) 
  {
    Serial.println("Cannot play: No files available");
    return;
  }
  
  if (currentState != STATE_IDLE) 
  {
    Serial.println("Cannot play: Already recording or playing");
    return;
  }

  // Create the filepath
  String filepath = String(CALLS_DIRECTORY) + "/" + currentFilename;

  // Start playback
  if (playWav.play(filepath.c_str())) 
  {
    currentState = STATE_PLAYBACK;
    Serial.print("Playing: ");
    Serial.println(currentFilename);
  }
  else 
  {
    Serial.print("Failed to play: ");
    Serial.println(currentFilename);
  }
}

void stopPlayback() 
{
  if (currentState != STATE_PLAYBACK) 
  {
    return;
  }
  
  // Stop playback
  playWav.stop();
  currentState = STATE_IDLE;
  
  Serial.println("Playback stopped");
}

float getInputLevel()
{
  // Return RMS level of input audio (0.0 to 1.0)
  if (inputLevel.available()) 
  {
    return inputLevel.read();
  }

  return 0.0;
}

String generateRecordingFilename()
{
  // Generate filename with timestamp
  // Format: CALL_TIMESTAMP.WAV

  String filename = "CALL_";
  filename += String(millis());
  filename += ".WAV";
  return filename;
}





