// TimerOne from https://github.com/PaulStoffregen/TimerOne
#include <TimerOne.h>

// a loose clone of a clone of a clone

// original: MTM Turning Machine
// clone: 2hp TM
// clone: https://github.com/eparadis/turing-machine-clone
// clone: this thing

// inputs
#define POT1_PIN    A0  // internal clock tempo
#define POT2_PIN    A1  // machine 1 probability
#define POT3_PIN    A2  // machine 1 steps
#define POT4_PIN    A3  // machine 2 probability
#define POT5_PIN    A4  // machine 2 steps
#define SWITCH1_PIN A5  // internal clock / trigger toggle
#define JACK1_PIN   A6  // trigger input

// outputs
#define JACK2_PIN   5   // PD5 machine 1 pattern a
#define JACK3_PIN   6   // PD6 machine 1 pattern b
#define JACK4_PIN   9   // PB1 machine 2 pattern a
#define JACK5_PIN   10  // PB2 machine 2 pattern b

#define STEPS_MAX   32  // max number of steps
#define NOTES_MAX   12  // number of notes in the output voltage LUT

int pot1;
int pot2;
int pot3;
int pot4;
int pot5;

int switch1;
int jack1;
int jack2;
int jack3;
int jack4;
int jack5;

bool trigger = false;

int m1_pattern_a[STEPS_MAX];
int m1_pattern_b[STEPS_MAX];
int m2_pattern_a[STEPS_MAX];
int m2_pattern_b[STEPS_MAX];
int notes[NOTES_MAX];

byte m1_pattern_index = 0;
byte m2_pattern_index = 0;

void setup() {
  pinMode(JACK2_PIN, OUTPUT);
  pinMode(JACK3_PIN, OUTPUT);
  pinMode(JACK4_PIN, OUTPUT);
  pinMode(JACK5_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Timer1.initialize(1000); // microseconds
  // Timer1.pwm(JACK2_PIN, 0);
  // Timer1.pwm(JACK3_PIN, 0);
  Timer1.pwm(JACK4_PIN, 0);
  Timer1.pwm(JACK5_PIN, 0);

  // fill the notes lookup table with half-note steps
  for (byte i = 0; i < NOTES_MAX; i++) {
    notes[i] = (i * 1000.0) / 12.0;
  }

  // fill the pattern buffer with the indices of the note LUT
  for (byte i = 0; i < STEPS_MAX; i++) {
    m2_pattern_b[i] = m2_pattern_a[i] = m1_pattern_b[i] = m1_pattern_a[i] = i % NOTES_MAX;
  }
}

void sampleAnalogInputs() {
  pot1 = analogRead(POT1_PIN);
  pot2 = analogRead(POT2_PIN);
  pot3 = analogRead(POT3_PIN);
  pot4 = analogRead(POT4_PIN);
  pot5 = analogRead(POT5_PIN);

  jack1 = analogRead(JACK1_PIN);
  switch1 = analogRead(SWITCH1_PIN);
}

void updateLED() {
  digitalWrite(LED_BUILTIN, trigger);
}

int voltageToPWM(int millivolts) {
  return map(millivolts, 0, 5000, 0, 1023);
}

void loop() {
  sampleAnalogInputs();

  int m1_probability = map(pot2, 0, 1023, 0, 1000);
  int m1_steps = map(pot3, 0, 1023, 1, STEPS_MAX);

  int m2_probability = map(pot4, 0, 1023, 0, 1000);
  int m2_steps = map(pot5, 0, 1023, 1, STEPS_MAX);

  if (trigger && jack1 < 512) { // falling edge
    trigger = false;
    updateLED();
  } else if (!trigger && jack1 >= 512) { // rising edge
    trigger = true;
    updateLED();

    // if a random number is under our PROB threshold,
    // change the current LUT index in the pattern
    // leave a small gap at the bottom to have a solid 'lock' area
    if (random(25, 1000) < m1_probability) {
      m1_pattern_a[m1_pattern_index] = random(0, NOTES_MAX);
    }
    if (random(25, 1000) < m1_probability) {
      m1_pattern_b[m1_pattern_index] = random(0, NOTES_MAX);
    }
    if (random(25, 1000) < m2_probability) {
      m2_pattern_a[m2_pattern_index] = random(0, NOTES_MAX);
    }
    if (random(25, 1000) < m2_probability) {
      m2_pattern_b[m2_pattern_index] = random(0, NOTES_MAX);
    }

    // pick the current value out of the pattern
    // and look up in the note LUT what voltage to output

    // m1 out
    // int out = notes[m1_pattern_a[m1_pattern_index]];
    // out = voltageToPWM(out);
    // Timer1.setPwmDuty(JACK2_PIN, out);

    // out = notes[m1_pattern_b[m1_pattern_index]];
    // out = voltageToPWM(out);
    // Timer1.setPwmDuty(JACK3_PIN, out);

    // m2 out
    int out = notes[m2_pattern_a[m2_pattern_index]];
    out = voltageToPWM(out);
    Timer1.setPwmDuty(JACK4_PIN, out);

    out = notes[m2_pattern_b[m2_pattern_index]];
    out = voltageToPWM(out);
    Timer1.setPwmDuty(JACK5_PIN, out);

    // reset pattern index to zero when it reaches our step count
    m1_pattern_index = (m1_pattern_index + 1) % m1_steps;
    m2_pattern_index = (m2_pattern_index + 1) % m2_steps;
  }
}
