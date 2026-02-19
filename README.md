# SON

## A PAS OUBLIER A METTRE DANS LE README

pour l'affichage, il faut installer processing via : https://processing.org/download (Windows)
code FAUST 

import("stdfaust.lib");
fmin = 80;

fmax = 1000;

analyze(input) = input : fi.lowpass(2,1200) : an.pitchTracker(fmin, fmax);

autotune_logic(p) = correction
    with {
    // change une freq en numéro MIDI et renvoie la correction (pour transpose)
    freq = (ma.SR / max(1, p)) : si.smooth(0.99);
    current_note = 12 * ma.log2(freq / 440.0) + 69;
    target_note = ba.hz2midikey(freq);
    correction = (target_note - current_note) : si.smooth(0.999);
    };
	
transpose(w, x, s, sig) =
	de.fdelay(maxDelay, d, sig)*ma.fmin(d/x, 1) +
	de.fdelay(maxDelay, d+w, sig)*(1-ma.fmin(d/x, 1))
with {
	maxDelay = 1024; // Modifié de 65536 à 1024 pour économiser la RAM
	i = 1 - pow(2, s/12.0);
	d = (+(i) : fmod(_,w)) ~ _;
};

process = _ <: (analyze : autotune_logic), _ : transpose(2048, 1024);
