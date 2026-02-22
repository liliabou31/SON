#include <Audio.h> // traitement du son
#include <Wire.h> // protocoles de communication avec le teensy
#include <SPI.h> // ``                ``
// #include <SD.h> // traitement de fichier audio
// #include <SerialFlash.h> // ``         ``
#include "midi_note.h"

#define FREQ_THRESHOLD 0.8

// Systeme audio 

AudioInputI2S            i2s1; // entrée micro
AudioMixer4              mixer1; // mélange les canaux
AudioEffectGranular      granular1; // change la hauteur
AudioAnalyzeNoteFrequency notefreq1; // detecte la frequence dominante
AudioAmplifier           amp1; // ajuste le volume 
AudioOutputI2S           i2s2; // sortie audio 

AudioConnection          patchCord1(i2s1, 0, mixer1, 0); // PC → Teensy (L)
AudioConnection          patchCord2(i2s1, 1, mixer1, 1); // PC → Teensy (R)
AudioConnection          patchCord3(mixer1, granular1); // vers pitch shift
AudioConnection          patchCord4(mixer1, notefreq1); // vers analyse pitch
AudioConnection          patchCord5(granular1, amp1);
AudioConnection          patchCord6(amp1, 0, i2s2, 0); // Teensy → PC (L)
AudioConnection          patchCord7(amp1, 0, i2s2, 1); // Teensy → PC (R)

AudioControlSGTL5000     sgtl5000_1; // "pont de communication" entre le code et la puce physique

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
  // Fréquence mesurée depuis le micro
  float f_mes = getPeakFreq();

  if (f_mes <= 0.0)
    return;   // = si aucune fréquence fiable 

  // Trouver la note MIDI la plus proche
  float f_des = findClosestMIDINote(f_mes);

  float f_ratio = f_des / f_mes;

  // Sécurité : empeche des deformations extremes → peut etre le commenter pour vois ce que ça fait? 
  if (f_ratio < 0.25) f_ratio = 0.25;
  if (f_ratio > 4.0)  f_ratio = 4.0;

  granular1.setSpeed(f_ratio); // application de l'autotune

  // Debug
  //Serial.print("Mesurée: ");
  //Serial.print(f_mes);
  //Serial.print(" | Cible: ");
  //Serial.print(f_des);
  //Serial.print(" | Ratio: ");
  //Serial.println(f_ratio);

  Serial.print(f_mes);
  Serial.print(",");
  Serial.println(f_des);

  delay(10);
}
