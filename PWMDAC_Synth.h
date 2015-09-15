//
// Header for PWM DAC Synthesizer ver.20150915
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

#define PWMDAC_CREATE_DEFAULT_INSTANCE PWMDACSynth PWM_SYNTH = PWMDACSynth

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

// Phase speed table to determine tone pitch
//
// [Phase-correct PWM dual-slope]
//    TCNTn = 00(BOTTOM) 01 02 03 ... FD FE FF(TOP) FE FD ... 03 02 01
//    -> 510 values (NOT 512)
//
// ISR()-call interval = 510 / 16MHz = 31.875us
// 
// [MIDI Tuning Standard]
// http://en.wikipedia.org/wiki/MIDI_Tuning_Standard
//    fn(d) = 440 Hz * 2^( (d - 69) / 12 )  MIDI note # d = 0..127
//
#define NOTE_A_FREQUENCY 440 // [Hz]
#define PHASE_SPEED_OF(note_number) (\
  pow( 2, (double)(note_number - 69)/12 + sizeof(unsigned long) * 8 ) \
  * NOTE_A_FREQUENCY * 510 / F_CPU \
)

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
    byte getChannel() const { return channel; }
    void soundOff() {
      ADSR_countdown = 0; volume8 = 0; volume16 = 0;
      phase = dphase = dphase_pitch_bend = dphase_original = 0L;
    }
    void release() { ADSR_countdown = 1; }
    void attack(
      PROGMEM const byte *wavetable,
      EnvelopeParam *env_param_p,
      double pitch_rate, byte note
    ) {
#define P2(x)   PHASE_SPEED_OF(x), PHASE_SPEED_OF(x + 1)
#define P4(x)   P2(x),  P2(x + 2)
#define P8(x)   P4(x),  P4(x + 4)
#define P16(x)  P8(x),  P8(x + 8)
#define P32(x)  P16(x), P16(x + 16)
#define P64(x)  P32(x), P32(x + 32)
#define P128(x) P64(x), P64(x + 64)
      static PROGMEM const unsigned long phase_speed_table[] = { P128(0) };
      this->wavetable = wavetable;
      this->env_param_p = env_param_p;
      dphase_original = pgm_read_dword(phase_speed_table + (this->note = note));
      setPitchRate(pitch_rate);
      ADSR_countdown = 4;
#undef P128
#undef P64
#undef P32
#undef P16
#undef P8
#undef P4
#undef P2
    }
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
    void update(byte modulation, int modulation_offset) {
      updateModulationStatus(modulation, modulation_offset);
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
    void updateModulationStatus(byte modulation, char modulation_offset) {
      if( modulation <= 0x10 ) { dphase = dphase_pitch_bend; return; }
      long dphase_offset = (dphase_pitch_bend >> 19) * modulation * (int)modulation_offset;
      dphase = dphase_pitch_bend + dphase_offset;
    }
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
    MidiChannel(PROGMEM const byte wavetable[]) {
      initialize();
      this->wavetable = wavetable;
    }
    double getPitchRate() const { return pitch_rate; }
    void setPitchBend(int bend) {
      // Signed 14bit value : -8192(lowest) .. 0(center) .. +8191(highest)
      char coarse = bend >> PITCH_BEND_COARSENESS;
      if( coarse_pitch_bend == coarse ) return;
      coarse_pitch_bend = coarse;
      pitch_rate = pow( 2,
        (float)((int)coarse_pitch_bend * pitch_bend_sensitivity) /
        (float)((8191 >> PITCH_BEND_COARSENESS) * 12)
      );
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
    void initialize() {
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
};

class PWMDACSynth {
  public:
    static const byte POLYPHONY = 6;
    static PROGMEM const byte sineWavetable[];
    static PROGMEM const byte squareWavetable[];
    static PROGMEM const byte triangleWavetable[];
    static PROGMEM const byte sawtoothWavetable[];
    static PROGMEM const byte shepardToneSineWavetable[];
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
        vsp->update(getChannel(vsp->getChannel())->modulation, modulation_offset);
      } while( ++vsp <= voices + (POLYPHONY - 1) );
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
      MidiChannel *channel_p = getChannel(channel);
      vsp->attack(channel_p->wavetable, &(channel_p->env_param), channel_p->getPitchRate(), pitch);
    }
    static void pitchBend(byte channel, int bend) {
      MidiChannel *channel_p = getChannel(channel);
      channel_p->setPitchBend(bend);
      VoiceStatus *vsp = voices;
      do {
        if(vsp->isVoiceOn(channel)) vsp->setPitchRate(channel_p->getPitchRate());
      } while( ++vsp <= voices + (POLYPHONY - 1) );
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
    static VoiceStatus voices[POLYPHONY];
    static PROGMEM const byte maxVolumeSineWavetable[];
    static VoiceStatus *getVoiceStatus(byte channel, byte note) {
      VoiceStatus *vsp = voices;
      for( byte i=0; i<POLYPHONY; i++, vsp++ )
        if( vsp->isNoteOn(channel,note) ) return vsp;
      return NULL;
    }
    static VoiceStatus *getLowestPriorityVoiceStatus() {
      unsigned int priority;
      unsigned int lowest_priority = 0xFFFF;
      byte i_lowest_priority = 0;
      for( byte i=0; i<POLYPHONY; i++ ) {
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
      } while( ++vsp <= voices + (POLYPHONY - 1) );
      return pulse_width >> 8;
    }
};

// Interrupt Service Routine
#ifdef PWMDAC_USE_TIMER1
ISR(TIMER1_OVF_vect) { PWMDACSynth::updatePulseWidth(); }
#endif
#ifdef PWMDAC_USE_TIMER2
ISR(TIMER2_OVF_vect) { PWMDACSynth::updatePulseWidth(); }
#endif

