//
// A simple MIDI synthesizer
//	using MIDI library + PWM DAC Synth library
//
#include <MIDI.h>

#define MIDI_ENABLE_PIN     2

#define PWMDAC_OUTPUT_PIN   3
#include <PWMDAC_Synth.h>

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 
  if( velocity == 0 ) {
    PWM_SYNTH.noteOff(channel,pitch,velocity);
    return;
  }
  PWM_SYNTH.noteOn(channel,pitch,velocity);
}
void setupMIDI() {
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  MIDI.setHandleNoteOff(PWM_SYNTH.noteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWM_SYNTH.pitchBend);
  MIDI.setHandleControlChange(PWM_SYNTH.controlChange);
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
}

void setup() {
  PWM_SYNTH.setup();
  setupMIDI();
}

void loop()
{
  static byte tick=0;
  MIDI.read();
  if( ++tick >= 16 ) {
    tick = 0;
    PWM_SYNTH.updateEnvelopeStatus();
  }
}

