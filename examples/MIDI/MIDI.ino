//
// A simple MIDI synthesizer
//	using MIDI library + PWM DAC Synth library
//
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define MIDI_ENABLE_PIN     2

#define PWMDAC_OUTPUT_PIN   3
#include <PWMDAC_Synth.h>
PWMDAC_INSTANCE;

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 
  if( velocity == 0 ) {
    PWMDACSynth::noteOff(channel,pitch,velocity);
    return;
  }
  PWMDACSynth::noteOn(channel,pitch,velocity);
}
void setupMIDI() {
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  MIDI.setHandleNoteOff(PWMDACSynth::noteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWMDACSynth::pitchBend);
  MIDI.setHandleControlChange(PWMDACSynth::controlChange);
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
}

void setup() {
  PWMDACSynth::setup();
  setupMIDI();
}

void loop()
{
  static byte tick=0;
  MIDI.read();
  if( ++tick >= 16 ) {
    tick = 0;
    PWMDACSynth::update();
  }
}

