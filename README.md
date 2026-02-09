# SON
code FAUST 

import("stdfaust.lib");

fmin = 80;  
fmax = 1000;

// On crée un signal filtré pour aider la détection
analyze(input) = input : fi.lowpass(2,1200) : an.pitchTracker(fmin, fmax);

autotune_logic(p) = correction
with {
    // change une freq en numéro MIDI et renvoie la correction (pour transpose)
    freq = ma.SR / max(1, p);
    current_note = 12 * ma.log2(freq / 440.0) + 69;
    target_note = floor(current_note + 0.5);
    correction = (target_note - current_note) : si.smoo; 
};

// Ta version adaptée pour la RAM de la Teensy
// w = window (ex: 512), x = crossfade (ex: 256), s = semitones, sig = entrée
transpose(w, x, s, sig) = 
    de.fdelay(maxDelay, d, sig) * ma.fmin(d/x, 1) +
    de.fdelay(maxDelay, d + w, sig) * (1 - ma.fmin(d/x, 1))
with {
    maxDelay = 1024; 
    i = 1 - pow(2, s/12.0); 
    // Le moteur qui calcule le délai variable
    d = (+(i) : _ % w) ~ _;
};

process = _ <: (analyze : autotune_logic), _ : transpose(1000, 500);
