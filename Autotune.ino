#include <Audio.h>
#include "Autotune.h"

AudioInputI2S input;
Autotune Autotune;
AudioOutputI2S output;
AudioControlSGTL5000 audioShield;

AudioConnection patchCord1(input, 0, Autotune, 0);
AudioConnection patchCord2(Autotune, 0, output, 0);
AudioConnection patchCord3(Autotune, 0, output, 1);

void setup() {
    AudioMemory(20);
    audioShield.enable(); 
    audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioShield.volume(0.4);
}

void loop() {
    // Ici tu peux par exemple ajuster les paramètres Faust en temps réel
}
