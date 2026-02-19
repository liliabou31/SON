import processing.serial.*;

Serial myPort;
float freqChantee = 0;
float freqCible = 0;
float angle = 0;

void setup() {
  size(800, 800);
  printArray(Serial.list());
  String portName = Serial.list()[4]; 
  myPort = new Serial(this, portName, 115200);
  println(Serial.list()); // Ceci va afficher la liste des ports dans la console noire
}

void draw() {
  background(10, 15, 25);
  
  // Lecture des données du Teensy 
  while (myPort.available() > 0) {
    String in = myPort.readStringUntil('\n');
    if (in != null) {
      float[] data = float(split(trim(in), ','));
      if (data.length >= 2) { 
        freqChantee = data[0];
        freqCible = data[1]; 
      }
    }
  }
  
  float erreur = freqChantee - freqCible;
  
  if (freqChantee > 20) {
    angle += erreur * 0.005; 
  }
  
  // Dessin
  
  translate(width/2, height/2);
  
  noFill();
  stroke(50);
  ellipse(0, 0, 400, 400);
  
// La roue qui tourne (Strobe)
  pushMatrix();
  rotate(angle);
  strokeWeight(3);
  // Si on est proche de la note, la roue devient verte
  if (abs(erreur) < 1.0) stroke(0, 255, 100); 
  else stroke(0, 200, 255);
  
  for (int i = 0; i < 24; i++) {
    rotate(TWO_PI / 24);
    line(160, 0, 200, 0);
  }
  popMatrix();

  // Affichage des fréquences
  fill(255);
  textAlign(CENTER);
  textSize(50);
  text(nf(freqChantee, 0, 1) + " Hz", 0, 10);
  
  textSize(20);
  fill(150);
  text("CIBLE: " + nf(freqCible, 0, 1) + " Hz", 0, 50);
}
  
