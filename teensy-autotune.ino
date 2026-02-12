#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "midi_note.h"

#define MIDI_CHANNEL 15
#define FREQ_THRESHOLD 0.4   // un peu plus sensible

// ---------------- AUDIO SYSTEM ----------------

AudioInputI2S             i2s1;
AudioMixer4               mixer1;
AudioEffectGranular       granular1;
AudioAnalyzeNoteFrequency notefreq1;
AudioAmplifier            amp1;
AudioOutputI2S            i2s2;
AudioControlSGTL5000      sgtl5000_1;

AudioConnection patchCord1(i2s1, 0, mixer1, 0);
AudioConnection patchCord2(i2s1, 1, mixer1, 1);
AudioConnection patchCord3(mixer1, granular1);
AudioConnection patchCord4(mixer1, notefreq1);
AudioConnection patchCord5(granular1, amp1);
AudioConnection patchCord6(amp1, 0, i2s2, 0);
AudioConnection patchCord7(amp1, 0, i2s2, 1);

// ---------------- GRANULAR MEMORY ----------------

#define GRANULAR_MEMORY_SIZE 12800
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// ---------------- GLOBALS ----------------

float freq_out = 440.0;
float last_valid_freq = 440.0;   // évite division par zéro

// ---------------- SETUP ----------------

void setup() {

  Serial.begin(115200);
  AudioMemory(128);

  // Activer le shield
  sgtl5000_1.enable();

  // 🔥 IMPORTANT : sélectionner le micro
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);

  // 🔥 IMPORTANT : gain micro (monte si freq bloquée)
  sgtl5000_1.micGain(45);   // max 63

  sgtl5000_1.volume(0.6);

  mixer1.gain(0, 0.7);
  mixer1.gain(1, 0.7);

  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

  notefreq1.begin(FREQ_THRESHOLD);

  Serial.println("System ready");
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
    }
  }

  return last_valid_freq;  // on garde la dernière fréquence valide
}

// ---------------- MIDI ----------------

float getDesFreq()
{
  byte channel;
  byte note;

  while (usbMIDI.read())   // lire tout le buffer
  {
    if (processMIDI(&channel, &note) == 1)
    {
      if (channel == MIDI_CHANNEL && note < NOTE_MAX)
      {
        freq_out = NOTES[note];
      }
    }
  }

  return freq_out;
}

// ---------------- MAIN LOOP ----------------

void loop() {

  float f_mes = getPeakFreq();
  float f_des = getDesFreq();

  if (f_mes <= 0.0) return;

  float f_ratio = f_des / f_mes;

  if (f_ratio < 0.5) f_ratio = 0.5;
  if (f_ratio > 2.0) f_ratio = 2.0;

  granular1.setSpeed(f_ratio);

  Serial.print("Measured: ");
  Serial.print(f_mes);
  Serial.print(" | Target: ");
  Serial.print(f_des);
  Serial.print(" | Ratio: ");
  Serial.println(f_ratio);

  delay(20);
}

// ---------------- MIDI PROCESSING ----------------

int processMIDI(byte * ret_channel, byte * ret_note)
{
  byte type = usbMIDI.getType();
  byte channel = usbMIDI.getChannel();
  byte data1 = usbMIDI.getData1();
  byte data2 = usbMIDI.getData2();

  if (type == usbMIDI.NoteOn && data2 > 0)
  {
    *ret_channel = channel;
    *ret_note = data1;
    return 1;
  }

  return 0;
}
