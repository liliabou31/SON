// on entend bien la voix 

import("stdfaust.lib");
fmin = 80;

fmax = 1000;

analyze(input) = input : fi.lowpass(2,1200) : an.pitchTracker(fmin, fmax);

autotune_logic(p) = correction
    with {
    // change une freq en numéro MIDI et renvoie la correction (pour transpose)
    freq = (ma.SR / max(1, p));  //: si.smooth(0.99);
    current_note = ba.if(freq > 50, 12 * ma.log2(freq / 440.0) + 69, 69); // on ajoute un ba.if pour éviter le silence à 0Hz)
    target_note = floor(current_note + 0.5);
    correction = (target_note - current_note) : si.smooth(0.5);
    };
	
transpose(w, x, s, sig) =
	de.fdelay(maxDelay, d, sig)*ma.fmin(d/x, 1) +
	de.fdelay(maxDelay, d+w, sig)*(1-ma.fmin(d/x, 1))
with {
	maxDelay = 1024; // Modifié de 65536 à 1024 pour économiser la RAM
	i = 1 - pow(2, s/12.0);
	d = (+(i) : fmod(_, 1024)) ~ _;
};

process = _ : transpose(1024, 512, 7) : *(0.8);
