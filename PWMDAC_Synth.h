//
// Header for PWM DAC Synthesizer ver.20150914
//
// Usage:
//    #define PWMDAC_OUTPUT_PIN << Your using pin# 3/9/10/11 >>
//    #include <PWMDAC_Synth.h>
//       :
//       :
//    Your codes
//
#pragma once

#include <Arduino.h>
#include <wiring_private.h>
#define NumberOf(array) (sizeof(array)/sizeof((array)[0]))
#define cbi16(sfr, bit) (_SFR_WORD(sfr) &= ~_BV(bit))
#define sbi16(sfr, bit) (_SFR_WORD(sfr) |= _BV(bit))

#ifndef PWMDAC_SYNTH_POLYPHONY
#define PWMDAC_SYNTH_POLYPHONY  6
#endif

#ifndef NOTE_A_FREQUENCY
#define NOTE_A_FREQUENCY 440 // [Hz]
#endif

// PWM output pin:
//
//   Reserved by wiring.c (Arduino core):
//     6: OC0A TIMER0
//     5: OC0B TIMER0
//
//   Available pins:
//     9: OC1A TIMER1
//    10: OC1B TIMER1 - maybe used as SS for SPI
//    11: OC2A TIMER2 - maybe used as MOSI for SPI
//     3: OC2B TIMER2 - maybe used as INT1
//
#ifdef PWMDAC_OUTPUT_PIN
#if PWMDAC_OUTPUT_PIN == 9 || PWMDAC_OUTPUT_PIN == 10
#define PWMDAC_USE_TIMER1
#elif PWMDAC_OUTPUT_PIN == 11 || PWMDAC_OUTPUT_PIN == 3
#define PWMDAC_USE_TIMER2
#endif
#endif // PWMDAC_OUTPUT_PIN

typedef struct _EnvelopeParam {
  unsigned int attack_speed;
  byte decay_time; // 0..15
  unsigned int sustain_level;
  byte release_time; // 0..15
} EnvelopeParam;

class VoiceStatus {
  public:
    VoiceStatus() { soundOff(); }
    void newChannel(byte channel) { soundOff(); this->channel = channel; }
    void soundOff() {
      ADSR_countdown = 0; volume8 = 0; volume16 = 0;
      phase = dphase = dphase_pitch_bend = dphase_original = 0L;
    }
    void release() { ADSR_countdown = 1; }
    void attack(byte);
    unsigned int getPriority() {
      unsigned int p = ADSR_countdown;
      return (p << 8) + volume8;
    }
    boolean isVoiceOn() const { return volume8 != 0; }
    boolean isVoiceOn(byte channel) const {
      return isVoiceOn() && this->channel == channel;
    }
    boolean isVoiceOn(byte channel, byte note) const {
      return isVoiceOn(channel) && this->note == note;
    }
    boolean isNoteOn() const { return ADSR_countdown > 1; }
    boolean isNoteOn(byte channel) const {
      return isNoteOn() && this->channel == channel;
    }
    boolean isNoteOn(byte channel, byte note) const {
      return isNoteOn(channel) && this->note == note;
    }
    unsigned int nextPulseWidth() {
      return volume8 * pgm_read_byte(wavetable + (byte)((phase += dphase) >> 24));
    }
    void update(int modulation_offset) {
      updateModulationStatus(modulation_offset);
      updateEnvelopeStatus();
    }
    void setPitchRate(double rate) {
      dphase = dphase_pitch_bend = dphase_original * rate;
    }
  protected:
    byte volume8; // 0..255
    byte channel; // 1..16
    byte note;    // 0..127
    unsigned long phase;
    unsigned long dphase; // Real phase speed (diff of phase each ISR() call)
    unsigned long dphase_pitch_bend; // Pitch-bended phase speed
    unsigned long dphase_original;   // Original phase speed
    unsigned int volume16;
    PROGMEM const byte *wavetable;
    EnvelopeParam *env_param_p;
    byte ADSR_countdown;
    void setVolume(unsigned int next_volume16) {
      volume8 = (volume16 = next_volume16) >> 8;
    }
    void tickAttack() {
      long next_volume16 = volume16;
      next_volume16 += env_param_p->attack_speed;
      if( next_volume16 > 0xFFFF ) {
        ADSR_countdown = 3; setVolume(0xFFFF);
      }
      else setVolume(next_volume16);
    }
    void tickDecay() {
      setVolume( volume16 - (volume16 >> env_param_p->decay_time) );
      if( volume16 <= env_param_p->sustain_level ) ADSR_countdown = 2;
    }
    void tickRelease() {
      unsigned int dv = volume16 >> env_param_p->release_time;
      if( dv < 0x0020 ) dv = 0x0020;
      long next_volume16 = volume16;
      next_volume16 -= dv;
      if( next_volume16 < 0x0100 ) soundOff();
      else setVolume((unsigned int)next_volume16);
    }
    void updateModulationStatus(int);
    void updateEnvelopeStatus() {
      switch(ADSR_countdown) {
        case 1: tickRelease(); break;
        case 3: tickDecay();   break;
        case 4: tickAttack();  break;
      }
    }
};

class MidiChannel {
  public:
    byte modulation;  // 0 ... 127 (unsigned 7 bit - MSB only)
    PROGMEM const byte *wavetable;
    EnvelopeParam env_param;
    MidiChannel() {
      modulation = 0;
      pitch_bend_sensitivity = 2;
      coarse_pitch_bend = 0;
      pitch_rate = 1.0;
      env_param.attack_speed = 0x8000;
      env_param.decay_time = 7;
      env_param.sustain_level = 0x8000;
      env_param.release_time = 7;
      rpns[LSB] = rpns[MSB] = 0xFF;
    }
    double getPitchRate() const { return pitch_rate; }
    void setPitchBend(byte channel, const VoiceStatus *voices, int bend) {
      // Signed 14bit value : -8192(lowest) .. 0(center) .. +8191(highest)
      char coarse = bend >> PITCH_BEND_COARSENESS;
      if( coarse_pitch_bend == coarse ) return;
      coarse_pitch_bend = coarse;
      updatePitchRate(channel, voices);
    }
    void controlChange(byte number, byte value) {
      switch(number) {
        case 1: modulation = value; break;
        case 6: // RPN/NRPN Data Entry
          if ( rpns[LSB] == 0 && rpns[MSB] == 0 ) {
            pitch_bend_sensitivity = value; 
          }
          break;
        case 100: rpns[LSB] = value; break;
        case 101: rpns[MSB] = value; break;
      }
    }
  protected:
    enum ByteSignificance {LSB, MSB};
    byte rpns[2]; // RPN (Registered Parameter Number)
    static const byte PITCH_BEND_COARSENESS = 6; // 6(FINEST -128..127) .. 13(COARSEST: -1..0)
    byte pitch_bend_sensitivity;  // +/- max semitones 0 .. 24(= 2 octaves)
    double pitch_rate; // positive value only: low .. 1.0(center) .. high
    char coarse_pitch_bend;
    void updatePitchRate(byte channel, const VoiceStatus *voices) {
      pitch_rate = pow( 2,
        (float)((int)coarse_pitch_bend * pitch_bend_sensitivity) /
        (float)((8191 >> PITCH_BEND_COARSENESS) * 12)
      );
      VoiceStatus *vsp = (VoiceStatus *)voices;
      do {
        if(vsp->isVoiceOn(channel)) vsp->setPitchRate(pitch_rate);
      } while( ++vsp <= voices + (PWMDAC_SYNTH_POLYPHONY - 1) );
    }
};

class PWMDACSynth {
  public:
    static PROGMEM const byte sineWavetable[];
    static PROGMEM const byte squareWavetable[];
    static PROGMEM const byte triangleWavetable[];
    static PROGMEM const byte sawtoothWavetable[];
    static PROGMEM const byte shepardToneSineWavetable[];
    PWMDACSynth() {
      char i;
      for( i=0; i<NumberOf(channels); i++ ) {
        (channels[i] = MidiChannel()).wavetable = sineWavetable;
      }
      for( i=0; i<PWMDAC_SYNTH_POLYPHONY; i++ )
        voices[i] = VoiceStatus();
    }
    static void setup() { // must be called from setup() once
#ifdef PWMDAC_OUTPUT_PIN
      pinMode(PWMDAC_OUTPUT_PIN,OUTPUT);
#endif
#ifdef PWMDAC_USE_TIMER1
      // No prescaling
      sbi (TCCR1B, CS10);
      cbi (TCCR1B, CS11);
      cbi (TCCR1B, CS12);
      // Phase-correct PWM
      sbi (TCCR1A, WGM10);
      cbi (TCCR1A, WGM11);
      cbi (TCCR1B, WGM12);
      cbi (TCCR1B, WGM13);
#if PWMDAC_OUTPUT_PIN == 9
      // Compare Output Mode
      //    Phase Correct PWM Mode (TCNTn dual-slope operation)
      //      Clear OCnB on Compare Match when up-counting.
      //      Set OCnB on Compare Match when down-counting.
      cbi (TCCR1A, COM1A0);
      sbi (TCCR1A, COM1A1);
#else
      cbi (TCCR1A, COM1B0);
      sbi (TCCR1A, COM1B1);
#endif
      sbi(TIMSK1,TOIE1); // Enable interrupt
#endif
#ifdef PWMDAC_USE_TIMER2
      // No prescaling
      sbi (TCCR2B, CS20);
      cbi (TCCR2B, CS21);
      cbi (TCCR2B, CS22);
      // Phase-correct PWM
      sbi (TCCR2A, WGM20);
      cbi (TCCR2A, WGM21);
      cbi (TCCR2B, WGM22);
#if PWMDAC_OUTPUT_PIN == 11
      cbi (TCCR2A, COM2A0);
      sbi (TCCR2A, COM2A1);
#else
      cbi (TCCR2A, COM2B0);
      sbi (TCCR2A, COM2B1);
#endif
      sbi(TIMSK2,TOIE2); // Enable interrupt
#endif
    }
    static void update() { // must be called from loop() repeatedly
      static byte modulation_phase = 0;
      int modulation_offset = pgm_read_byte(maxVolumeSineWavetable + (++modulation_phase)) - 0x7F;
      VoiceStatus *vsp = voices;
      do {
        vsp->update(modulation_offset);
      } while( ++vsp <= voices + (PWMDAC_SYNTH_POLYPHONY - 1) );
    }
    static void updateEnvelopeStatus() { return update(); }
    static void updatePulseWidth() { // must be called from ISR() repeatedly
#ifdef PWMDAC_USE_TIMER1
#if PWMDAC_OUTPUT_PIN == 9
      OCR1A = nextPulseWidth();
#else
      OCR1B = nextPulseWidth();
#endif
#endif
#ifdef PWMDAC_USE_TIMER2
#if PWMDAC_OUTPUT_PIN == 11
      OCR2A = nextPulseWidth();
#else
      OCR2B = nextPulseWidth();
#endif
#endif
    }
    // MIDI lib compatible methods
    static void noteOff(byte channel, byte pitch, byte velocity) {
      VoiceStatus *vsp = getVoiceStatus(channel, pitch);
      if( vsp != NULL ) vsp->release();
    }
    static void noteOn(byte channel, byte pitch, byte velocity) {
      VoiceStatus *vsp = getVoiceStatus(channel, pitch);
      if( vsp == NULL ) (vsp = getLowestPriorityVoiceStatus())->newChannel(channel);
      vsp->attack(pitch);
    }
    static void pitchBend(byte channel, int bend) {
      getChannel(channel)->setPitchBend(channel, voices, bend);
    }
    static void controlChange(byte channel, byte number, byte value) {
      getChannel(channel)->controlChange(number, value);
    }
    // MIDI channel index <-> pointer converter
    static MidiChannel *getChannel(char channel) {
      return channels + (channel - 1);
    }
    static char getChannel(MidiChannel *cp) {
      return (cp - channels) + 1;
    }
    // All channel
    static void setEnvelope(struct _EnvelopeParam ep) {
      for( int i=0; i<NumberOf(channels); i++ )
        channels[i].env_param = ep;
    }
    static void setWave(PROGMEM byte *wt) {
      for( int i=0; i<NumberOf(channels); i++ )
        channels[i].wavetable = wt;
    }
    // Utility
    static byte musicalMod12(char);
    static byte musicalMod7(char);
    static byte log2(unsigned int);
    static int musicalConstrain12(int note, int min_note, int max_note) {
      if( max_note < note ) {
        note = max_note - musicalMod12(max_note - note);
      }
      else if( min_note > note ) {
        note = min_note + musicalMod12(note - min_note);
      }
      return note;
    }
  protected:
    static MidiChannel channels[16];
    static VoiceStatus voices[PWMDAC_SYNTH_POLYPHONY];
    static PROGMEM const byte maxVolumeSineWavetable[];
    static VoiceStatus *getVoiceStatus(byte channel, byte note) {
      VoiceStatus *vsp = voices;
      for( byte i=0; i<PWMDAC_SYNTH_POLYPHONY; i++, vsp++ )
        if( vsp->isNoteOn(channel,note) ) return vsp;
      return NULL;
    }
    static VoiceStatus *getLowestPriorityVoiceStatus() {
      unsigned int priority;
      unsigned int lowest_priority = 0xFFFF;
      byte i_lowest_priority = 0;
      for( byte i=0; i<PWMDAC_SYNTH_POLYPHONY; i++ ) {
        if( (priority = voices[i].getPriority()) < lowest_priority ) {
          lowest_priority = priority;
          i_lowest_priority = i;
        }
      }
      return voices + i_lowest_priority;
    }
    static byte nextPulseWidth() { 
      unsigned int pulse_width = 0;
      VoiceStatus *vsp = voices;
      do {
        pulse_width += vsp->nextPulseWidth();
      } while( ++vsp <= voices + (PWMDAC_SYNTH_POLYPHONY - 1) );
      return pulse_width >> 8;
    }
};

extern PWMDACSynth PWM_SYNTH;

// Interrupt Service Routine
#ifdef PWMDAC_USE_TIMER1
ISR(TIMER1_OVF_vect) { PWMDACSynth::updatePulseWidth(); }
#endif
#ifdef PWMDAC_USE_TIMER2
ISR(TIMER2_OVF_vect) { PWMDACSynth::updatePulseWidth(); }
#endif

