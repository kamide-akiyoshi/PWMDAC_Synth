//
// Simple chord music box example
//

// Put your configuration here (before including PWMDAC_Synth.h)
#define PWMDAC_OUTPUT_PIN   3

#include <PWMDAC_Synth.h>

PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);

// {wavetableArray, {release, sustain, decay, attack}}
PROGMEM const Instrument musicbox = {sineWavetable, {9, 0, 11, 3}};

PWMDAC_CREATE_INSTANCE(&musicbox);

// Chord:       C       G       Am      Em      F       Dm     Gsus4 G  C
byte melo1[] = {60, 60, 67, 67, 69, 69, 64, 64, 65, 65, 62, 62, 67, 67, 60};
byte melo2[] = {64, 64, 59, 59, 60, 60, 67, 67, 69, 69, 65, 65, 60, 59, 64};
byte melo3[] = {67, 67, 62, 62, 64, 64, 59, 59, 60, 60, 69, 69, 62, 62, 67};
byte current_melo1 = UCHAR_MAX;
byte current_melo2;
byte current_melo3;
byte loop_count=0;
byte note_step=0;
unsigned long ms;

void setup() {
  PWMDACSynth::setup();
  ms = millis();
}

void loop() {
  unsigned long newms = millis();
  if( newms - ms > 1000 ) {
    ms = newms;
    if( current_melo1 != UCHAR_MAX ) {
      PWMDACSynth::noteOff(1, current_melo1, 0);
      PWMDACSynth::noteOff(1, current_melo2, 0);
      PWMDACSynth::noteOff(1, current_melo3, 0);
      current_melo1 = UCHAR_MAX;
    }
    if( note_step >= 15 ) {
      note_step = 0;
    } else {
      PWMDACSynth::noteOn(1, current_melo1 = melo1[note_step], 64);
      PWMDACSynth::noteOn(1, current_melo2 = melo2[note_step], 64);
      PWMDACSynth::noteOn(1, current_melo3 = melo3[note_step], 64);
      note_step++;
    }
  }
  if( ++loop_count >= 8 ) {
    loop_count = 0;
    PWMDACSynth::update();
  }
}

