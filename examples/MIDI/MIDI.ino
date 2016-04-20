//
// A simple MIDI synthesizer
//	using MIDI library + PWM DAC Synth library
//
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define MIDI_ENABLE_PIN     2
#define PWMDAC_OUTPUT_PIN   3
#include <PWMDAC_Synth.h>

PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);

// {wavetableArray, {release, sustain, decay, attack}}
PROGMEM const Instrument instrument = {sineWavetable, {9, 128, 8, 3}};

PWMDAC_CREATE_INSTANCE(&instrument);

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 
  if( velocity == 0 ) {
    PWMDACSynth::noteOff(channel,pitch,velocity);
    return;
  }
  PWMDACSynth::noteOn(channel,pitch,velocity);
}

void setup() {
  PWMDACSynth::setup();
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  MIDI.setHandleNoteOff(PWMDACSynth::noteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWMDACSynth::pitchBend);
  MIDI.setHandleControlChange(PWMDACSynth::controlChange);
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
}

void loop() {
  static byte tick=0;
  MIDI.read();
  if( ++tick >= 16 ) {
    tick = 0;
    PWMDACSynth::update();
  }
}
