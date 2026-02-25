# SON

## A PAS OUBLIER A METTRE DANS LE README

pour l'affichage, il faut installer processing via : https://processing.org/download (Windows)
code FAUST 

import("stdfaust.lib");

ratio = hslider("ratio", 1.0, 0.5, 2.0, 0.001);

demitons = 12 * (log(ratio) / log(2));

transpose(w, x, s, sig) = de.fdelay(maxDelay,d,sig)*ma.fmin(d/x,1) +
                           de.fdelay(maxDelay,d+w,sig)*(1-ma.fmin(d/x,1))
with {
    maxDelay = 2048;
    i = 1 - pow(2, s/12); 
    d = i : (+ : +(w) : fmod(_,w)) ~ _;
};

process = transpose(2048, 10, demitons);
