# SON
code FAUST 

import("stdfaust.lib");

fmin = 80;
fmax = 1000;

// On booste le signal X10 uniquement pour l'analyseur
// pour qu'il détecte la note sans avoir à crier
preamp(sig) = sig * 10;

analyze(input) = input : preamp : fi.lowpass(2,800) : fi.highpass(2,80) : an.pitchTracker(fmin, fmax);

autotune_logic(p) = correction
with {
    freq = ma.SR / max(1, p);
    current_note = 12 * ma.log2(freq / 440.0) + 69;
    target_note = floor(current_note + 0.5);
    // On lisse beaucoup plus pour que ce soit "doux" (0.9995)
    correction = (target_note - current_note) : si.smooth(0.9995);
};

transpose(w, x, s, sig) =
	de.fdelay(maxDelay, d, sig)*ma.fmin(d/x, 1) +
	de.fdelay(maxDelay, d+w, sig)*(1-ma.fmin(d/x, 1))
with {
	maxDelay = 1024; // Modifié de 65536 à 1024 pour économiser la RAM
	i = 1 - pow(2, s/12.0);
	d = (+(i) : ma.fmod(_,w)) ~ _;
};

process = _ <: (analyze : autotune_logic), _ : transpose(1024, 512);
