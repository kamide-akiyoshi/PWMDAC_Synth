//
// Example of MIDI synthesizer using MIDI library + PWM DAC Synth library
//
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// Put your configuration here (before including PWMDAC_Synth.h)
#define MIDI_ENABLE_PIN     2
#define PWMDAC_OUTPUT_PIN   3

// Comment this #define out if you don't need to use MIDI channel priority feature
//#define PWMDAC_CHANNEL_PRIORITY_SUPPORT

#include <PWMDAC_Synth.h>

#define DRUM_MIDI_CHANNEL 10

PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);
PWMDAC_CREATE_WAVETABLE(sawtoothWavetable, PWMDAC_SAWTOOTH_WAVE);
PWMDAC_CREATE_WAVETABLE(squareWavetable, PWMDAC_SQUARE_WAVE);
PWMDAC_CREATE_WAVETABLE(triangleWavetable, PWMDAC_TRIANGLE_WAVE);

PROGMEM const byte guitarWavetable[] = {
23, 24, 24, 25, 25, 25, 26, 26,
26, 27, 27, 28, 29, 30, 32, 33,
34, 35, 36, 36, 36, 36, 36, 37,
37, 37, 38, 39, 39, 40, 40, 40,
40, 40, 39, 39, 40, 40, 40, 41,
41, 41, 41, 40, 41, 41, 41, 41,
42, 42, 42, 42, 41, 41, 41, 41,
42, 41, 41, 40, 40, 40, 39, 39,
38, 37, 37, 37, 37, 36, 36, 36,
35, 34, 33, 32, 31, 30, 30, 30,
30, 30, 30, 30, 30, 29, 28, 27,
27, 27, 26, 26, 26, 26, 25, 24,
24, 23, 23, 23, 23, 22, 22, 22,
22, 21, 21, 21, 21, 21, 21, 21,
20, 21, 20, 20, 19, 19, 18, 18,
18, 18, 18, 18, 18, 18, 17, 16,
16, 15, 14, 14, 15, 15, 16, 16,
16, 16, 17, 17, 17, 17, 17, 17,
18, 18, 18, 18, 17, 17, 17, 17,
17, 17, 18, 18, 17, 17, 16, 15,
15, 14, 14, 15, 15, 15, 16, 16,
16, 16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 15, 15, 15, 15, 15,
14, 14, 13, 12, 11, 10, 9,  9,
9,  9,  8,  8,  8,  7,  6,  6,
5,  4,  3,  2,  2,  3,  3,  3,
3,  4,  4,  4,  4,  4,  3,  3,
2,  2,  2,  2,  2,  1,  2,  2,
3,  4,  4,  4,  5,  5,  5,  6,
6,  7,  9,  10, 11, 11, 12, 12,
13, 13, 14, 14, 14, 15, 15, 16,
17, 17, 18, 19, 20, 21, 21, 22,
};

PROGMEM const byte randomWavetable[] = {
39, 22, 21, 9,  23, 13, 28, 31,
15, 30, 40, 8,  29, 26, 27, 8,
34, 30, 4,  22, 39, 25, 35, 33,
38, 17, 7,  38, 18, 24, 12, 9,
8,  36, 27, 17, 33, 0,  13, 35,
20, 15, 34, 0,  34, 6,  29, 38,
30, 20, 16, 39, 26, 18, 28, 28,
24, 38, 27, 31, 25, 14, 9,  25,
31, 13, 3,  17, 29, 23, 18, 6,
12, 20, 30, 27, 1,  40, 9,  19,
26, 21, 4,  25, 1,  16, 11, 18,
15, 23, 30, 7,  37, 23, 11, 19,
30, 36, 7,  9,  17, 27, 8,  41,
4,  9,  26, 0,  24, 18, 6,  15,
30, 23, 7,  9,  9,  41, 1,  0,
33, 9,  34, 18, 19, 22, 25, 16,
22, 31, 41, 21, 27, 35, 38, 32,
17, 16, 10, 39, 36, 9,  37, 13,
16, 12, 10, 14, 21, 12, 19, 12,
11, 19, 0,  32, 13, 27, 32, 10,
18, 5,  22, 9,  6,  33, 29, 2,
4,  2,  17, 0,  38, 14, 18, 16,
25, 10, 8,  3,  3,  39, 3,  5,
7,  22, 6,  11, 16, 23, 23, 5,
3,  23, 22, 25, 17, 7,  19, 29,
41, 0,  10, 15, 14, 41, 6,  40,
8,  0,  19, 16, 30, 18, 2,  24,
17, 37, 11, 6,  1,  33, 10, 31,
33, 19, 20, 23, 24, 41, 4,  40,
29, 12, 4,  11, 38, 17, 41, 8,
15, 37, 19, 34, 36, 8,  19, 10,
32, 14, 19, 1,  1,  35, 35, 3,
};

PROGMEM const Instrument DRUM_INSTRUMENT = {randomWavetable, {5, 0, 5, 0}};

// {wavetableArray, {release, sustain, decay, attack}}
PROGMEM const Instrument INSTRUMENTS[] = {
// General MIDI instruments
  // Piano
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {8, 0, 10, 4}},

  // Chromatic Percussion
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sineWavetable, {9, 0, 11, 4}},
  {sineWavetable, {9, 0, 10, 3}},
  {sineWavetable, {9, 0, 11, 4}},
  {sineWavetable, {8, 0, 8, 4}},
  {sineWavetable, {7, 0, 7, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 9, 4}},

  // Organ
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 8}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Guitar
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {randomWavetable, {9, 8, 10, 4}},

  // Bass
  {guitarWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 192, 9, 3}},
  {guitarWavetable, {9, 192, 9, 6}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 128, 10, 4}},
  {sawtoothWavetable, {9, 192, 11, 4}},

  // Strings
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {guitarWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {sineWavetable, {9, 0, 11, 4}},
  {randomWavetable, {9, 0, 10, 3}},

  // Ensemble
  {triangleWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 9}},
  {triangleWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 128, 9, 4}},
  {sineWavetable, {9, 255, 11, 4}},
  {squareWavetable, {8, 0, 9, 3}},

  // Brass
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 255, 11, 7}},
  {sawtoothWavetable, {9, 128, 9, 5}},
  {sawtoothWavetable, {9, 192, 9, 5}},
  {guitarWavetable, {9, 192, 10, 5}},

  // Reed
  {guitarWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {triangleWavetable, {9, 255, 11, 5}},

  // Pipe
  {guitarWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {randomWavetable, {9, 128, 10, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 7}},

  // Synth Lead
  {squareWavetable, {9, 255, 11, 3}},
  {sawtoothWavetable, {9, 255, 11, 3}},
  {triangleWavetable, {9, 255, 11, 5}},
  {triangleWavetable, {9, 128, 8, 6}},
  {sawtoothWavetable, {9, 192, 8, 5}},
  {sawtoothWavetable, {9, 192, 8, 5}},
  {guitarWavetable, {9, 192, 11, 5}},
  {sawtoothWavetable, {9, 255, 11, 3}},

  // Synth Pad
  {guitarWavetable, {9, 255, 11, 5}},
  {guitarWavetable, {9, 255, 11, 9}},
  {guitarWavetable, {4, 192, 9, 4}},
  {guitarWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 192, 10, 8}},
  {sawtoothWavetable, {9, 255, 11, 10}},
  {guitarWavetable, {9, 255, 11, 8}},
  {sawtoothWavetable, {9, 255, 11, 10}},

  // Synth Effects
  {guitarWavetable, {9, 128, 9, 4}},
  {sawtoothWavetable, {9, 128, 10, 10}},
  {triangleWavetable, {9, 192, 8, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {10, 0, 11, 3}},
  {triangleWavetable, {9, 0, 11, 10}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Ethnic
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 9, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Percussive
  {squareWavetable, {9, 64, 9, 4}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {guitarWavetable, {9, 0, 11, 5}},
  {sineWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {triangleWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 255, 7, 12}},

  // Sound Effects
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {10, 64, 12, 12}},
  {sineWavetable, {9, 255, 11, 4}},
  {randomWavetable, {9, 255, 11, 4}},
  {randomWavetable, {9, 255, 11, 12}},
  {randomWavetable, {10, 255, 10, 11}},
  {randomWavetable, {9, 0, 10, 3}},
};

PWMDAC_CREATE_INSTANCE(INSTRUMENTS);

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 
  if( velocity == 0 ) {
    PWMDACSynth::noteOff(channel,pitch,velocity);
    return;
  }
  PWMDACSynth::noteOn(channel,pitch,velocity);
}

void HandleProgramChange(byte channel, byte number) {
  if( channel == DRUM_MIDI_CHANNEL ) return;
  PWMDACSynth::getChannel(channel)->programChange(INSTRUMENTS + number);
}

void HandleSystemReset() {
  PWMDACSynth::systemReset();
  // In PWMDAC Synth lib, All MIDI channel will be reset to same waveform/envelope even ch#10.
  // So ensure to set drum instrument to ch#10.
  PWMDACSynth::getChannel(DRUM_MIDI_CHANNEL)->programChange(&DRUM_INSTRUMENT);
}

// System Exclusive to reset, without first byte(0xF0)/last byte(0xF7)
PROGMEM const byte GM_SYSTEM_ON[] = {
  0x7E, // Universal
  0x7F, 0x09, 0x01
};
PROGMEM const byte GS_SYSTEM_ON[] = {
  0x41, // Roland
  0x00, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41
};
PROGMEM const byte XG_SYSTEM_ON[] = {
  0x43, // YAMAHA
  0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00
}; 
void HandleSystemExclusive(byte *array, unsigned size) {
  array++; size -= 2;
  if( memcmp_P(array, GM_SYSTEM_ON, size) == 0 ) {
    HandleSystemReset(); return;
  }
  array[1] &= 0xF0; // Clear lower 4-bits of device ID (0x1n -> 0x10)
  if( memcmp_P(array, XG_SYSTEM_ON, size) == 0 ) {
    HandleSystemReset(); return;
  }
  array[1] = 0; // Clear device ID (0xnn -> 0x00)
  if( memcmp_P(array, GS_SYSTEM_ON, size) == 0 ) {
    HandleSystemReset(); return;
  }
}

void setup() {
  PWMDACSynth::setup();
  MIDI.setHandleNoteOff(PWMDACSynth::noteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWMDACSynth::pitchBend);
  MIDI.setHandleControlChange(PWMDACSynth::controlChange);
  MIDI.setHandleSystemReset(HandleSystemReset);
  MIDI.setHandleProgramChange(HandleProgramChange);
  MIDI.setHandleSystemExclusive(HandleSystemExclusive);
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
  PWMDACSynth::getChannel(DRUM_MIDI_CHANNEL)->programChange(&DRUM_INSTRUMENT);
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
  // To set higher priority for melody part on MIDI channel 1
  PWMDACSynth::getChannel(1)->setPriority(0xC0);
#endif
}

void loop() {
  static byte tick=0;
  MIDI.read();
  if( ++tick >= 8 ) {
    tick = 0;
    PWMDACSynth::update();
  }
}
