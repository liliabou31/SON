#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "midi_note.h"
#include "Autotune_final.h" // Ton export Faust

#define FREQ_THRESHOLD 0.8

// Système audio 
AudioInputI2S            i2s1;
AudioMixer4              mixer1;
Autotune_final           Autotune;   // Ton effet Faust
AudioAnalyzeNoteFrequency notefreq1; 
AudioAmplifier           amp1;
AudioOutputI2S           i2s2;

// Connexions corrigées
AudioConnection          patch1(i2s1, 0, mixer1, 0);
AudioConnection          patch2(i2s1, 1, mixer1, 1);

// Le son part vers l'analyseur ET vers l'effet Faust
AudioConnection          patch3(mixer1, 0, notefreq1, 0); 
AudioConnection          patch4(mixer1, 0, Autotune, 0); 

// La sortie de l'effet Faust va vers l'ampli, puis vers les HP
AudioConnection          patch5(Autotune, 0, amp1, 0);
AudioConnection          patch6(amp1, 0, i2s2, 0);
AudioConnection          patch7(amp1, 0, i2s2, 1);

AudioControlSGTL5000     sgtl5000_1;

// Granular memory

#define GRANULAR_MEMORY_SIZE 12800
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// variables globales 

float freq_out = 440.0;   // fréquence cible par défaut (La 440)
float last_valid_freq = -1.0;

// setup 

void setup() {
  Serial.begin(115200);
  AudioMemory(128);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);  // Activation du micro
  sgtl5000_1.micGain(40);                   // Ajuste le gain du micro (0-63)
  sgtl5000_1.volume(0.5);

  mixer1.gain(0, 0.5);   // mélange des deux canaux
  mixer1.gain(1, 0.5);

  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
  granular1.beginPitchShift(120); // rendu + ou - robotique
  notefreq1.begin(FREQ_THRESHOLD);

  Serial.println("Le système est prêt");
}

// détection de fréquence 

float getPeakFreq()
{
  if (notefreq1.available())
  {
    float prob = notefreq1.probability(); // score interne de fiabilité de la fréquence
    float freq = notefreq1.read();

    if (prob > FREQ_THRESHOLD && freq > 0.0) 
    {
      last_valid_freq = freq;
      return freq;
    }
  }
  return -1.0; // fréquence invalide
}

// MIDI

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

// main loop

void loop() {
  float f_mes = getPeakFreq();
  if (f_mes <= 0.0) return;

  float f_des = findClosestMIDINote(f_mes);
  float f_ratio = f_des / f_mes;

  // C'est ici que tu envoies le ratio à Faust !
  // "ratio" correspond au nom que tu as donné dans nentry("ratio", ...)
  myAutotune.setParamValue("ratio", f_ratio);

  delay(10);
}
