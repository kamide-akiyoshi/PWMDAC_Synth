//
// A simple MIDI synthesizer
//	using MIDI library + PWM DAC Synth library
//
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define MIDI_ENABLE_PIN     2

#define PWMDAC_OUTPUT_PIN   3
#include <PWMDAC_Synth.h>
const EnvelopeParam DEFAULT_ENV_PARAM = {4, 10, 128, 8};
PWMDAC_CREATE_INSTANCE(sineWavetable, PWMDAC_SINE_WAVE, DEFAULT_ENV_PARAM);

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
