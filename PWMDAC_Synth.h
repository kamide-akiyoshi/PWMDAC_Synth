//
// PWM DAC Synthesizer ver.20150926
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
#define FX2(f,x)    f(x), f(x + 1)
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
  SINPI(x,8)   + SINPI(x,4)  + SINPI(x,2) + cos(PI * (x)) + 8 ) * 16 / PWMDAC_POLYPHONY)

#define PWMDAC_CREATE_WAVETABLE(table, function) PROGMEM const byte table[] = ARRAY256(function)

typedef struct _EnvelopeParam {
  byte attack_time;   // 0..15
  byte decay_time;    // 0..15
  byte sustain_level; // 0..255
  byte release_time;  // 0..15
} EnvelopeParam;

class MidiChannel {
  protected:
    enum ByteSignificance {LSB, MSB}; // AVR is Little Endian
    byte rpns[2]; // RPN (Registered Parameter Number)
    int pitch_bend; // Signed 14bit value : -8192(lowest) .. 0(center) .. +8191(highest)
    byte pitch_bend_sensitivity; // +/- max semitones 0 .. 24(= 2 octaves)
    double pitch_rate; // positive value only: low .. 1.0(center) .. high
    void updatePitchRate() {
      pitch_rate = pow( 2,
        (float)pitch_bend_sensitivity/(float)12 *
        (float)pitch_bend/(float)8191 );
    }
  public:
    byte modulation;  // 0 ... 127 (unsigned 7 bit - MSB only)
    PROGMEM const byte *wavetable;
    EnvelopeParam env_param;
    MidiChannel(PROGMEM const byte *wavetable, const EnvelopeParam env_param) {
      modulation = 0;
      rpns[LSB] = rpns[MSB] = UCHAR_MAX;
      pitch_bend_sensitivity = 2;
      pitch_bend = 0;
      pitch_rate = 1.0;
      this->env_param = env_param;
      this->wavetable = wavetable;
    }
    double getPitchRate() const { return pitch_rate; }
    int getPitchBend() const { return pitch_bend; }
    byte getPitchBendSensitivity() const { return pitch_bend_sensitivity; }
    boolean pitchBendChange(int bend) {
      if( (pitch_bend & 0xFFF0) == (bend & 0xFFF0) ) return false;
      pitch_bend = bend;
      updatePitchRate();
      return true;
    }
    boolean pitchBendSensitivityChange(byte value) {
      if ( pitch_bend_sensitivity == value ) return false;
      pitch_bend_sensitivity = value;
      updatePitchRate();
      return true;
    }
    void controlChange(byte number, byte value) {
      switch(number) {
        case 1: modulation = value; break;
        case 6: // RPN/NRPN Data Entry
          if ( rpns[LSB] == 0 && rpns[MSB] == 0 ) pitchBendSensitivityChange(value);
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
  pow( 2, (double)(note_number - 69)/12 + (BitSizeOf(unsigned long) + 1) ) \
  * PWMDAC_NOTE_A_FREQUENCY * 0xFF / F_CPU )

class VoiceStatus {
  public:
    enum AdsrStatus : byte {ADSR_OFF, ADSR_RELEASE, ADSR_SUSTAIN, ADSR_DECAY, ADSR_ATTACK};
    VoiceStatus() { soundOff(); }
    void newChannel(byte channel) {
      if( this->channel == channel ) return;
      soundOff();
      this->channel = channel;
    }
    byte getChannel() const { return channel; }
    byte getNote() const { return note; }
    void attack(MidiChannel *cp, byte note) {
      static PROGMEM const unsigned long phase_speed_table[] = ARRAY128(PHASE_SPEED_OF);
      wavetable = cp->wavetable;
      env_param_p = &(cp->env_param);
      dphase_original = pgm_read_dword(phase_speed_table + (this->note = note));
      setPitchRate(cp->getPitchRate());
      adsr = ADSR_ATTACK;
    }
    void release() { adsr = ADSR_RELEASE; }
    inline unsigned int nextPulseWidth() {
      phase.p32 += dphase;
      return getVolume8() * pgm_read_byte( wavetable + getPhase() );
    }
    inline byte getVolume8() const { return volume.v8[sizeof(volume.v8) - 1]; }
    inline unsigned int getVolume16() const { return volume.v16; }
    inline AdsrStatus getAdsrStatus() const { return adsr; }
    boolean isVoiceOn() const { return getVolume8(); }
    boolean isVoiceOn(byte channel) const {
      return isVoiceOn() && this->channel == channel;
    }
    boolean isVoiceOn(byte channel, byte note) const {
      return isVoiceOn(channel) && this->note == note;
    }
    boolean isNoteOn() const { return adsr > ADSR_RELEASE; }
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
    byte channel; // 1..16
    byte note;    // 0..127
    PROGMEM const byte *wavetable;
    EnvelopeParam *env_param_p;
    AdsrStatus adsr;
    union {
      unsigned int v16;
      byte v8[2];
    } volume;
    union {
      unsigned long p32;
      byte p8[4];
    } phase;
    inline byte getPhase() const { return phase.p8[sizeof(phase.p8) - 1]; }
    unsigned long dphase; // Real phase speed (diff of phase each ISR() call)
    unsigned long dphase_pitch_bend; // Pitch-bended phase speed
    unsigned long dphase_original;   // Original phase speed
    void soundOff() {
      adsr = ADSR_OFF; volume.v16 = 0;
      phase.p32 = dphase = dphase_pitch_bend = dphase_original = 0L;
    }
    void updateEnvelopeStatus() {
      switch(adsr) {
        case ADSR_ATTACK: {
          long v32 = volume.v16;
          if( (v32 += (UINT_MAX >> env_param_p->attack_time)) > UINT_MAX ) {
             volume.v16 = UINT_MAX; adsr = ADSR_DECAY; break;
          }
          volume.v16 = v32; break;
        }
        case ADSR_DECAY: {
          volume.v16 -= volume.v16 >> env_param_p->decay_time;
          if( getVolume8() <= env_param_p->sustain_level ) adsr = ADSR_SUSTAIN;
          break;
        }
        case ADSR_RELEASE: {
          unsigned int dv = volume.v16 >> env_param_p->release_time;
          if( dv == 0 ) dv = 1;
          long v32 = volume.v16;
          if( (v32 -= dv) < 0x100 ) { soundOff(); break; }
          volume.v16 = v32; break;
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
      EACH_VOICE(v) if( v->isNoteOn(channel, pitch) ) { v->release(); break; }
    }
    static void noteOn(byte channel, byte pitch, byte velocity) {
      reserveVoiceFor(channel, pitch)->attack(getChannel(channel), pitch);
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
    static VoiceStatus *reserveVoiceFor(byte channel, byte note) {
      EACH_VOICE(v) if( v->isNoteOn(channel,note) ) return v;
      VoiceStatus *lowest_priority_vsp = voices;
      VoiceStatus::AdsrStatus oldest_adsr = VoiceStatus::ADSR_ATTACK;
      unsigned int lowest_volume = UINT_MAX;
      EACH_VOICE(v) {
        VoiceStatus::AdsrStatus adsr = v->getAdsrStatus();
        if( adsr > oldest_adsr ) continue;
        unsigned int volume = v->getVolume16();
        if( volume > lowest_volume ) continue;
        oldest_adsr = adsr;
        lowest_volume = volume;
        lowest_priority_vsp = v;
      }
      lowest_priority_vsp->newChannel(channel);
      return lowest_priority_vsp;
    }
#undef EACH_VOICE
};

#define PWMDAC_CREATE_INSTANCE(table, function, env_param) \
  ISR(PWMDAC_OVF_vect) { PWMDACSynth::updatePulseWidth(); } \
  VoiceStatus PWMDACSynth::voices[PWMDAC_POLYPHONY]; \
  PWMDAC_CREATE_WAVETABLE(PWMDACSynth::maxVolumeSineWavetable, PWMDAC_MAX_VOLUME_SINE_WAVE); \
  PWMDAC_CREATE_WAVETABLE(table, function); \
  MidiChannel PWMDACSynth::channels[16] = MidiChannel(table, env_param);

