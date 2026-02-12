#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "midi_note.h"

#define FREQ_THRESHOLD 0.5

// ---------------- AUDIO SYSTEM ----------------
AudioInputI2S            i2s1;
AudioMixer4              mixer1;
AudioEffectGranular      granular1;
AudioAnalyzeNoteFrequency notefreq1;
AudioAmplifier           amp1;
AudioOutputI2S           i2s2;

AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, granular1);
AudioConnection          patchCord4(mixer1, notefreq1);
AudioConnection          patchCord5(granular1, amp1);
AudioConnection          patchCord6(amp1, 0, i2s2, 0);
AudioConnection          patchCord7(amp1, 0, i2s2, 1);

AudioControlSGTL5000     sgtl5000_1;

// ---------------- GRANULAR MEMORY ----------------
#define GRANULAR_MEMORY_SIZE 12800
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// ---------------- GLOBALS ----------------
float freq_out = 440.0;   // fréquence cible par défaut (La 440)
float last_valid_freq = -1.0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  AudioMemory(128);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);  // <-- Activation du micro
  sgtl5000_1.micGain(40);                   // <-- Ajuste le gain du micro (0-63)
  sgtl5000_1.volume(0.5);

  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
  notefreq1.begin(FREQ_THRESHOLD);

  Serial.println("System ready.");
}

// ---------------- FREQUENCY DETECTION ----------------
float getPeakFreq()
{
  if (notefreq1.available())
  {
    float prob = notefreq1.probability();
    float freq = notefreq1.read();

    if (prob > FREQ_THRESHOLD && freq > 0.0)
    {
      last_valid_freq = freq;
      return freq;
    }
  }
  return -1.0; // fréquence invalide
}

// ---------------- MIDI ----------------
float findClosestMIDINote(float freq) {
  float minDiff = 1e6;
  float targetFreq = 440.0; // La par défaut

  for (int i = 0; i < NOTE_MAX; i++) {
    float diff = abs(freq - NOTES[i]);
    if (diff < minDiff) {
      minDiff = diff;
      targetFreq = NOTES[i];
    }
  }

  return targetFreq;
}


// ---------------- MAIN LOOP ----------------
void loop() {
  // Fréquence mesurée depuis le micro
  float f_mes = getPeakFreq();

  if (f_mes <= 0.0)
    return;

  // Trouver la note MIDI la plus proche
  float f_des = findClosestMIDINote(f_mes);

  float f_ratio = f_des / f_mes;

  // Sécurité
  if (f_ratio < 0.25) f_ratio = 0.25;
  if (f_ratio > 4.0)  f_ratio = 4.0;

  granular1.setSpeed(f_ratio);

  // Debug
  Serial.print("Measured: ");
  Serial.print(f_mes);
  Serial.print(" | Target: ");
  Serial.print(f_des);
  Serial.print(" | Ratio: ");
  Serial.println(f_ratio);

  delay(10);
}


// ---------------- MIDI PROCESSING ----------------
int processMIDI(byte * ret_channel, byte * ret_note) {
    byte type = usbMIDI.getType();
    byte channel = usbMIDI.getChannel();
    byte data1 = usbMIDI.getData1();
    byte data2 = usbMIDI.getData2();

    if (type == usbMIDI.NoteOn && data2 > 0) {
        *ret_channel = channel;
        *ret_note = data1;
        return 1;
    }
    return 0;
}
