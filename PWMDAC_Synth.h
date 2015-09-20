//
// PWM DAC Synthesizer ver.20150920
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/pwmdac_synth_lib/
//  https://osdn.jp/users/kamide/pf/PWMDAC_Synth/
//
#pragma once

#include <Arduino.h>
#include <wiring_private.h>
#include <limits.h>

#define NumberOf(array) (sizeof(array)/sizeof((array)[0]))
#define BitSizeOf(type) (8 * sizeof(type))
#define cbi16(sfr, bit) (_SFR_WORD(sfr) &= ~_BV(bit))
#define sbi16(sfr, bit) (_SFR_WORD(sfr) |= _BV(bit))

// Function-to-array generator
#define FX(f,x)     f(x)
#define FX2(f,x)    FX(f,x),  FX(f,x + 1)
#define FX4(f,x)    FX2(f,x), FX2(f,x + 2)
#define FX8(f,x)    FX4(f,x), FX4(f,x + 4)
#define FX16(f,x)   FX8(f,x), FX8(f,x + 8)
#define FX32(f,x)   FX16(f,x),FX16(f,x + 16)
#define FX64(f,x)   FX32(f,x),FX32(f,x + 32)
#define FX128(f,x)  FX64(f,x),FX64(f,x + 64)
#define ARRAY128(f) {FX128(f,0)}
#define ARRAY256(f) {FX128(f,0),FX128(f,128)}

#ifndef PWMDAC_OUTPUT_PIN
#define PWMDAC_OUTPUT_PIN 3
#endif
#ifndef PWMDAC_NOTE_A_FREQUENCY
#define PWMDAC_NOTE_A_FREQUENCY 440 // [Hz]
#endif
#ifndef PWMDAC_POLYPHONY
#define PWMDAC_POLYPHONY 6
#endif


// Built-in wavetable generator
#define PWMDAC_SQUARE_WAVE(x)   (((x) < 128 ? 0 : 255) / PWMDAC_POLYPHONY)
#define PWMDAC_SAWTOOTH_WAVE(x) ((x) / PWMDAC_POLYPHONY)
#define PWMDAC_TRIANGLE_WAVE(x) (((x) < 128 ? (x) : 255 - (x)) * 2 / PWMDAC_POLYPHONY)

#define SINPI(x, t) sin(PI * (x) / (t))
#define PWMDAC_MAX_VOLUME_SINE_WAVE(x)  ((SINPI(x,128) + 1) * 128)
#define PWMDAC_SINE_WAVE(x)     (PWMDAC_MAX_VOLUME_SINE_WAVE(x) / PWMDAC_POLYPHONY)
#define PWMDAC_SHEPARD_TONE(x)  (( \
  SINPI(x,128) + SINPI(x,64) + SINPI(x,32) + SINPI(x,16) + \
  SINPI(x,8)   + SINPI(x,4)  + SINPI(x,2)  + SINPI(x,1)  + 8 ) * 16 / PWMDAC_POLYPHONY)

#define PWMDAC_CREATE_WAVETABLE(table, function) PROGMEM const byte table[] = ARRAY256(function)

typedef struct _EnvelopeParam {
  unsigned int attack_speed;
  byte decay_time; // 0..15
  unsigned int sustain_level;
  byte release_time; // 0..15
} EnvelopeParam;

class MidiChannel {
  protected:
    enum ByteSignificance {LSB, MSB};
    byte rpns[2]; // RPN (Registered Parameter Number)
    byte pitch_bend_sensitivity; // +/- max semitones 0 .. 24(= 2 octaves)
    int pitch_bend; // Signed 14bit value : -8192(lowest) .. 0(center) .. +8191(highest)
    double pitch_rate; // positive value only: low .. 1.0(center) .. high
  public:
    byte modulation;  // 0 ... 127 (unsigned 7 bit - MSB only)
    PROGMEM const byte *wavetable;
    EnvelopeParam env_param;
    MidiChannel(PROGMEM const byte *wavetable) {
      modulation = 0;
      pitch_bend_sensitivity = 2;
      pitch_bend = 0;
      pitch_rate = 1.0;
      env_param.attack_speed = 0x8000;
      env_param.decay_time = 7;
      env_param.sustain_level = 0x8000;
      env_param.release_time = 7;
      rpns[LSB] = rpns[MSB] = UCHAR_MAX;
      this->wavetable = wavetable;
    }
    double getPitchRate() const { return pitch_rate; }
    boolean pitchBendChange(int bend) {
      if( (pitch_bend & 0xFFF0) == (bend & 0xFFF0) ) return false;
      pitch_rate = pow( 2,
        (float)pitch_bend_sensitivity/(float)12 *
        (float)(pitch_bend = bend)/(float)8191 );
      return true;
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
};

// [Phase-correct PWM dual-slope]
//    TCNTn =
//       00(BOTTOM) 01 02 03 ... FC FD FE
//       FF(TOP)    FE FD FC ... 03 02 01
//    -> 0xFF * 2 = 510 values (NOT 512)
//
// ISR()-call interval = (0xFF * 2) / F_CPU(16MHz) = 31.875us
// 
// [MIDI Tuning Standard]
// http://en.wikipedia.org/wiki/MIDI_Tuning_Standard
//    fn(d) = 440 Hz * 2^( (d - 69) / 12 )  MIDI note # d = 0..127
//
#define PHASE_SPEED_OF(note_number) ( \
  pow( 2, ((double)note_number - 69)/12 + BitSizeOf(unsigned long) ) \
  * PWMDAC_NOTE_A_FREQUENCY * 0xFF * 2 / F_CPU )

class VoiceStatus {
  public:
    VoiceStatus() { soundOff(); }
    void newChannel(byte channel) {
      if( this->channel == channel ) return;
      soundOff();
      this->channel = channel;
    }
    byte getChannel() const { return channel; }
    void attack(PROGMEM const byte *wavetable, EnvelopeParam *ep, double pitch_rate, byte note) {
      static PROGMEM const unsigned long phase_speed_table[] = ARRAY128(PHASE_SPEED_OF);
      this->wavetable = wavetable;
      env_param_p = ep;
      dphase_original = pgm_read_dword(phase_speed_table + (this->note = note));
      setPitchRate(pitch_rate);
      adsr = ADSR_ATTACK;
    }
    void release() { adsr = ADSR_RELEASE; }
    inline unsigned int nextPulseWidth() {
      phase += dphase;
      return volume8 * pgm_read_byte(wavetable + (phase >> 24));
    }
    unsigned int getPriority() { return ((unsigned int)adsr << 8) + volume8; }
    boolean isVoiceOn() const { return volume8; }
    boolean isVoiceOn(byte channel) const {
      return isVoiceOn() && this->channel == channel;
    }
    boolean isVoiceOn(byte channel, byte note) const {
      return isVoiceOn(channel) && this->note == note;
    }
    boolean isNoteOn() const { return adsr > 1; }
    boolean isNoteOn(byte channel) const {
      return isNoteOn() && this->channel == channel;
    }
    boolean isNoteOn(byte channel, byte note) const {
      return isNoteOn(channel) && this->note == note;
    }
    void update(byte modulation, int modulation_offset) {
      updateModulationStatus(modulation, modulation_offset);
      updateEnvelopeStatus();
    }
    void setPitchRate(double rate) {
      dphase = dphase_pitch_bend = dphase_original * rate;
    }
  protected:
    enum AdsrStatus : byte {ADSR_OFF, ADSR_RELEASE, ADSR_SUSTAIN, ADSR_DECAY, ADSR_ATTACK};
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
    AdsrStatus adsr;
    void soundOff() {
      adsr = ADSR_OFF; volume8 = 0; volume16 = 0;
      phase = dphase = dphase_pitch_bend = dphase_original = 0L;
    }
    void setVolume(unsigned int v) { volume8 = (volume16 = v) >> 8; }
    void updateEnvelopeStatus() {
      switch(adsr) {
        case ADSR_ATTACK: {
          long next_volume16 = volume16;
          next_volume16 += env_param_p->attack_speed;
          if( next_volume16 <= UINT_MAX ) { setVolume(next_volume16); break; }
          adsr = ADSR_DECAY; setVolume(UINT_MAX);
          break;
        }
        case ADSR_DECAY: {
          setVolume( volume16 - (volume16 >> env_param_p->decay_time) );
          if( volume16 <= env_param_p->sustain_level ) adsr = ADSR_SUSTAIN;
          break;
        }
        case ADSR_RELEASE: {
          unsigned int dv = volume16 >> env_param_p->release_time;
          if( dv < 0x0020 ) dv = 0x0020;
          long next_volume16 = volume16;
          next_volume16 -= dv;
          if( next_volume16 < 0x0100 ) soundOff();
          else setVolume((unsigned int)next_volume16);
          break;
        }
      }
    }
    void updateModulationStatus(byte modulation, char modulation_offset) {
      if( modulation <= 0x10 ) { dphase = dphase_pitch_bend; return; }
      long dphase_offset = (dphase_pitch_bend >> 19) * modulation * (int)modulation_offset;
      dphase = dphase_pitch_bend + dphase_offset;
    }
};

#if PWMDAC_OUTPUT_PIN == 6 || PWMDAC_OUTPUT_PIN == 5
// In Arduino, TIMER0 has been reserved by wiring.c in Arduino core,
//   so defining PWMDAC_OUTPUT_PIN = 5 or 6 causes compile error
//   (multiple definition of `__vector_16')
#define PWMDAC_USE_TIMER0
#define PWMDAC_OVF_vect TIMER0_OVF_vect
#if PWMDAC_OUTPUT_PIN == 6
#define PWMDAC_OCR OCR0A
#else
#define PWMDAC_OCR OCR0B
#endif
#elif PWMDAC_OUTPUT_PIN == 9 || PWMDAC_OUTPUT_PIN == 10
#define PWMDAC_USE_TIMER1
#define PWMDAC_OVF_vect TIMER1_OVF_vect
#if PWMDAC_OUTPUT_PIN == 9
#define PWMDAC_OCR OCR1A  // OC1A maybe used as MOSI for SPI
#else
#define PWMDAC_OCR OCR1B  // OC1B maybe used as SS for SPI
#endif
#elif PWMDAC_OUTPUT_PIN == 11 || PWMDAC_OUTPUT_PIN == 3
#define PWMDAC_USE_TIMER2
#define PWMDAC_OVF_vect TIMER2_OVF_vect
#if PWMDAC_OUTPUT_PIN == 11
#define PWMDAC_OCR OCR2A
#else
#define PWMDAC_OCR OCR2B  // OC2B maybe used as INT1
#endif
#endif

class PWMDACSynth {
  public:
    static void setup() { // must be called from setup() once
      pinMode(PWMDAC_OUTPUT_PIN,OUTPUT);
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
#define EACH_VOICE(p) for(VoiceStatus *(p)=voices; (p)<= voices + (PWMDAC_POLYPHONY - 1); (p)++)
    inline static void updatePulseWidth() {
      unsigned int pw = 0;
      EACH_VOICE(v) pw += v->nextPulseWidth();
      PWMDAC_OCR = pw >> 8;
    }
    static void update() { // must be called from loop() repeatedly
      static byte modulation_phase = 0;
      int modulation_offset = pgm_read_byte(maxVolumeSineWavetable + (++modulation_phase)) - 0x7F;
      EACH_VOICE(v) v->update(getChannel(v->getChannel())->modulation, modulation_offset);
    }
#define ALL_CHANNEL for( int i=0; i<NumberOf(channels); i++ ) channels[i]
    static void setEnvelope(struct _EnvelopeParam ep) { ALL_CHANNEL.env_param = ep; }
    static void setWave(PROGMEM byte *wt) { ALL_CHANNEL.wavetable = wt; }
#undef ALL_CHANNEL
    static void noteOff(byte channel, byte pitch, byte velocity) {
      VoiceStatus *vsp = getVoiceStatus(channel, pitch);
      if( vsp != NULL ) vsp->release();
    }
    static void noteOn(byte channel, byte pitch, byte velocity) {
      VoiceStatus *vsp = getVoiceStatus(channel, pitch);
      if( vsp == NULL ) (vsp = getLowestPriorityVoiceStatus())->newChannel(channel);
      MidiChannel *cp = getChannel(channel);
      vsp->attack(cp->wavetable, &(cp->env_param), cp->getPitchRate(), pitch);
    }
    static void pitchBend(byte channel, int bend) {
      MidiChannel *cp = getChannel(channel);
      if ( ! cp->pitchBendChange(bend) ) return;
      double pitch_rate = cp->getPitchRate();
      EACH_VOICE(v) if(v->isVoiceOn(channel)) v->setPitchRate(pitch_rate);
    }
    static void controlChange(byte channel, byte number, byte value) {
      getChannel(channel)->controlChange(number, value);
    }
    static MidiChannel *getChannel(char channel) { return channels + (channel - 1); }
    static char getChannel(MidiChannel *cp) { return (cp - channels) + 1; }
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
    static VoiceStatus voices[PWMDAC_POLYPHONY];
    static PROGMEM const byte maxVolumeSineWavetable[];
    static VoiceStatus *getVoiceStatus(byte channel, byte note) {
      EACH_VOICE(v) if( v->isNoteOn(channel,note) ) return v;
      return NULL;
    }
    static VoiceStatus *getLowestPriorityVoiceStatus() {
      unsigned int priority;
      unsigned int lowest_priority = UINT_MAX;
      VoiceStatus *lowest_priority_vsp = voices;
      EACH_VOICE(v) {
        if( (priority = v->getPriority()) >= lowest_priority ) continue;
        lowest_priority = priority;
        lowest_priority_vsp = v;
      }
      return lowest_priority_vsp;
    }
#undef EACH_VOICE
};

#define PWMDAC_CREATE_INSTANCE(table, function) \
  ISR(PWMDAC_OVF_vect) { PWMDACSynth::updatePulseWidth(); } \
  VoiceStatus PWMDACSynth::voices[PWMDAC_POLYPHONY]; \
  PWMDAC_CREATE_WAVETABLE(PWMDACSynth::maxVolumeSineWavetable, PWMDAC_MAX_VOLUME_SINE_WAVE); \
  PWMDAC_CREATE_WAVETABLE(table, function); \
  MidiChannel PWMDACSynth::channels[16] = MidiChannel(table);

