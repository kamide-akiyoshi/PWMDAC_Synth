//
// PWM DAC Synthesizer ver.20150918
//
#include "PWMDAC_Synth.h"

// MIDI channel status holder
MidiChannel PWMDACSynth::channels[16] = MidiChannel(PWMDACSynth::sineWavetable);

byte PWMDACSynth::musicalMod7(char x) {
  while( x & 0xF8 ) x = (x >> 3) + (x & 7);
  if(x==7) return 0;
  return x;
}
byte PWMDACSynth::musicalMod12(char x) {
  char n = x >> 2;
  while( n & 0xFC ) n = (n >> 2) + (n & 3);
  x &= 3;
  if(n==3||n==0) return x;
  return x + (n << 2);
}
byte PWMDACSynth::log2(unsigned int x) {
  byte index = 0;
  if (x & 0xFF00) { index += 8; x &= 0xFF00; }
  if (x & 0xF0F0) { index += 4; x &= 0xF0F0; }
  if (x & 0xCCCC) { index += 2; x &= 0xCCCC; }
  if (x & 0xAAAA) { index += 1; x &= 0xAAAA; }
  return index;
}

//
// Built-in wavetables
//
#define P2(x) P1(x),P1(x + 1)
#define P4(x) P2(x),P2(x + 2)
#define P8(x) P4(x),P4(x + 4)
#define P16(x) P8(x),P8(x + 8)
#define P32(x) P16(x),P16(x + 16)
#define P64(x) P32(x),P32(x + 32)
#define P128(x) P64(x),P64(x + 64)
#define P256(x) P128(x),P128(x + 128)

#define P1(x) (x) / POLYPHONY
PROGMEM const byte PWMDACSynth::sawtoothWavetable[] = { P256(0) };
#undef P1

#define P1(x) ((x) < 128 ? 0 : 255) / POLYPHONY
PROGMEM const byte PWMDACSynth::squareWavetable[] = { P256(0) };
#undef P1

#define P1(x) ((x) < 128 ? (x) : 255 - (x)) * 2 / POLYPHONY
PROGMEM const byte PWMDACSynth::triangleWavetable[] = { P256(0) };
#undef P1

#define P1(x) (sin(PI * (x) / 128) + 1) * 128
PROGMEM const byte PWMDACSynth::maxVolumeSineWavetable[] = { P256(0) };
#undef P1

#define P1(x) (sin(PI * (x) / 128) + 1) * 128 / POLYPHONY
PROGMEM const byte PWMDACSynth::sineWavetable[] = { P256(0) };
#undef P1

#define P1(x) ( \
  sin(PI * (x) / 128) + sin(PI * (x) / 64) + \
  sin(PI * (x) / 32)  + sin(PI * (x) / 16) + \
  sin(PI * (x) / 8)   + sin(PI * (x) / 4) + \
  sin(PI * (x) / 2)   + sin(PI * (x)) + 8 ) * 16 / POLYPHONY
PROGMEM const byte PWMDACSynth::shepardToneSineWavetable[] = { P256(0) };
#undef P1

