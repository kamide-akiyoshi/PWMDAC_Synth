//
// PWM DAC Synthesizer ver.20190330
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/pwmdac_synth_lib/
//  https://osdn.jp/users/kamide/pf/PWMDAC_Synth/
//
#pragma once

#include <Arduino.h>
#include <wiring_private.h>
#include <limits.h>

#define NumberOf(array) (sizeof(array)/sizeof(*(array)))
#define HighestElementOf(array) ((array)[NumberOf(array)-1])
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
//  x = Phase angle : 0x00...0x80(PI_radian)...0xFF
//  f(x) = Wave voltage at the x : 0x00(min)...0x80(center)...0xFF(max)
#define PWMDAC_SQUARE_WAVE(x)   (((x) < 0x80 ? 0x00 : 0xFF) / PWMDAC_POLYPHONY)
#define PWMDAC_SAWTOOTH_WAVE(x) ((x) / PWMDAC_POLYPHONY)
#define PWMDAC_TRIANGLE_WAVE(x) (((x) < 0x80 ? (x) : 0x100 - (x)) * 2 / PWMDAC_POLYPHONY)

#define SINPI(x, t) sin(PI * (x) / (t))
#define PWMDAC_MAX_VOLUME_SINE_WAVE(x)  ( 0x80 * ( 1 + SINPI(x,0x80) ) )
#define PWMDAC_SINE_WAVE(x)     (PWMDAC_MAX_VOLUME_SINE_WAVE(x) / PWMDAC_POLYPHONY)
#define PWMDAC_SHEPARD_TONE(x)  ( 0x80 * ( 8 \
    +SINPI(x,0x80) \
    +SINPI(x,0x40) \
    +SINPI(x,0x20) \
    +SINPI(x,0x10) \
    +SINPI(x,0x08) \
    +SINPI(x,0x04) \
    +SINPI(x,0x02) \
    +SINPI(x,0x01) \
    ) / ( 8 * PWMDAC_POLYPHONY ) )

#define PWMDAC_CREATE_WAVETABLE(table, function) PROGMEM const byte table[] = ARRAY256(function)

enum AdsrStatus : byte {ADSR_OFF, ADSR_RELEASE, ADSR_SUSTAIN, ADSR_DECAY, ADSR_ATTACK};

typedef struct _Instrument {
  PROGMEM const byte *wavetable;
  PROGMEM const byte envelope[ADSR_ATTACK];
} Instrument;

class EnvelopeParam {
  public:
    EnvelopeParam() { }
    EnvelopeParam(PROGMEM const byte *envelope) {
      setParam(envelope);
    }
    EnvelopeParam(byte attack_time, byte decay_time, byte sustain_level, byte release_time) {
      *getParam(ADSR_ATTACK) = attack_time;     // 0..15
      *getParam(ADSR_DECAY) = decay_time;       // 0..15
      *getParam(ADSR_SUSTAIN) = sustain_level;  // 0..255
      *getParam(ADSR_RELEASE) = release_time;   // 0..15
    }
    byte *getParam(AdsrStatus adsr) {
      return param + (byte)adsr - 1;
    }
    void setParam(PROGMEM const byte *envelope) {
      memcpy_P(this->param, envelope, sizeof(this->param));
    }
  protected:
    byte param[ADSR_ATTACK];
};

class MidiChannel {
  protected:
    // RPN (Registered Parameter Number) or NRPN(Non-Registered Parameter Number)
    typedef struct { byte LSB; byte MSB; } ParameterNumber;
    static const byte RPN_Null = UCHAR_MAX;
    ParameterNumber rpn;
    ParameterNumber *data_entry_source;
    // Semitones when pitch bend max
    //   0(Min) .. 2(Default: whole tone) .. 12(1 octave) .. 24(Max: 2 octaves)
    byte pitch_bend_sensitivity;
    // Signed 14bit
    //   -8192(Min) .. 0(Center) .. +8191(0x1FFF:Max)
    int pitch_bend;
    double pitch_rate;
    void updatePitchRate() {
      pitch_rate = pow(2, static_cast<double>(static_cast<long>(pitch_bend) * pitch_bend_sensitivity) / (0x2000L * 12));
    }
  public:
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
    // 0xFF:Lowest priority (default)
    // 0x00:Highest priority (The channel occupies a voice even non-zero lowest volume)
    byte priority_volume_threshold;
#endif
    byte modulation;  // 0 .. 127 (Unsigned 7bit)
    PROGMEM const byte *wavetable;
    EnvelopeParam envelope;
    MidiChannel(PROGMEM const Instrument *instrument) {
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
      setPriority(0);
#endif
      reset(instrument);
    }
    void reset(PROGMEM const Instrument *instrument) {
      resetAllControllers();
      rpn.LSB = rpn.MSB = RPN_Null;
      data_entry_source = NULL;
      programChange(instrument);
    }
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
    void setPriority(byte priority) { this->priority_volume_threshold = ~priority; }
#endif
    int getPitchBend() const { return pitch_bend; }
    unsigned long bendedPitchOf(unsigned long original_pitch) {
      return pitch_bend ? original_pitch * pitch_rate : original_pitch;
    }
    boolean pitchBendChange(int new_pitch_bend) {
      int diff = new_pitch_bend - pitch_bend;
      if( diff < 16 && diff > -16 ) return false;
      pitch_bend = new_pitch_bend;
      updatePitchRate();
      return true;
    }
    byte getPitchBendSensitivity() const { return pitch_bend_sensitivity; }
    boolean pitchBendSensitivityChange(byte value) {
      if ( pitch_bend_sensitivity == value ) return false;
      pitch_bend_sensitivity = value;
      updatePitchRate();
      return true;
    }
    void programChange(PROGMEM const Instrument *instrument) {
      this->wavetable = (PROGMEM const byte *)pgm_read_word(&(instrument->wavetable));
      this->envelope.setParam(instrument->envelope);
    }
    void resetAllControllers() {
      modulation = 0;
      pitch_bend = 0;
      pitch_rate = 1.0;
      pitch_bend_sensitivity = 2;
    }
    void controlChange(byte number, byte value) {
      switch(number) {
        case 1: modulation = value; break;
        case 6: // RPN/NRPN Data Entry
          if ( data_entry_source == NULL ) break;
          if ( data_entry_source->LSB == 0 && data_entry_source->MSB == 0 ) {
            pitchBendSensitivityChange(value);
          }
          data_entry_source = NULL;
          break;
        // case  96: break; // Data Increment
        // case  97: break; // Data Decrement
        case  98: // NRPN LSB
        case  99: data_entry_source = NULL; break; // NRPN MSB
        case 100: (data_entry_source = &rpn)->LSB = value; break;
        case 101: (data_entry_source = &rpn)->MSB = value; break;
        case 121: resetAllControllers(); break;
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
  pow( 2, (double)(note_number - 69)/12 + (sizeof(unsigned long) * 8 + 1) ) \
  * PWMDAC_NOTE_A_FREQUENCY * 0xFF / F_CPU )

class VoiceStatus {
  public:
    boolean isNoteOn(byte note, MidiChannel *cp) {
      return this->note == note && adsr > ADSR_RELEASE && channel == cp;
    }
    boolean isSoundOn(MidiChannel *cp) {
      return adsr > ADSR_OFF && channel == cp;
    }
    inline unsigned int nextPulseWidth() {
      // To generate waveform rapidly in ISR(), this method must be inline
      phase.v32 += dphase32.real;
      return HighestElementOf(volume.v8) * pgm_read_byte( wavetable + HighestElementOf(phase.v8) );
    }
    static const byte HIGHEST_PRIORITY = UCHAR_MAX;
    static const byte LOWEST_PRIORITY = 0;
    byte getPriority() {
      byte t = HighestElementOf(volume.v8) >> 1;
      if( adsr == ADSR_ATTACK ) t = HIGHEST_PRIORITY - t;
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
      if( channel != NULL && t > channel->priority_volume_threshold ) {
        t >>= 1; t |= 0x80;
      } else {
        t >>= 1;
      }
#endif
      return t;
    }
    VoiceStatus() { reset(); }
    void setChannel(MidiChannel *cp) { if( channel != cp ) reset(cp); }
    void noteOn(byte note) {
      this->wavetable = channel->wavetable;
      this->note = note;
      updatePitch();
      adsr = ADSR_ATTACK;
    }
    void noteOff(byte note, MidiChannel *cp) {
      if( isNoteOn(note, cp) ) adsr = ADSR_RELEASE;
    }
    void allNotesOff(MidiChannel *cp) {
      if( channel == cp ) adsr = ADSR_RELEASE;
    }
    void allSoundOff(MidiChannel *cp) { if( isSoundOn(cp) ) reset(); }
    void reset(MidiChannel *cp = NULL) {
      adsr = ADSR_OFF; volume.v16 = 0; note = UCHAR_MAX;
      phase.v32 = dphase32.real = dphase32.moffset = dphase32.bended = 0L;
      channel = cp;
    }
    void update(int modulation_offset) {
      updateModulationStatus(modulation_offset);
      updateEnvelopeStatus();
    }
    void updatePitch(MidiChannel *cp) { if( isSoundOn(cp) ) updatePitch(); }
protected:
    PROGMEM const byte *wavetable;
    struct {
      unsigned long real;     // Real phase speed
      unsigned long bended;   // Pitch-bended phase speed
      long moffset;           // Modulation pitch offset
    } dphase32;
    union { unsigned long v32; byte v8[4]; } phase;
    union { unsigned int v16; byte v8[2]; } volume;
    MidiChannel *channel;
    byte note; // 0..127
    AdsrStatus adsr;
    void updatePitch() {
      static PROGMEM const unsigned long phase_speed_table[] = ARRAY128(PHASE_SPEED_OF);
      dphase32.bended = channel->bendedPitchOf(pgm_read_dword(phase_speed_table + note));
      dphase32.real = dphase32.bended + dphase32.moffset;
    }
    void updateModulationStatus(char modulation_offset) {
      if( channel->modulation <= 0x10 ) {
        if( dphase32.moffset == 0 ) return;
        dphase32.moffset = 0;
        dphase32.real = dphase32.bended;
        return;
      }
      dphase32.moffset = (dphase32.real >> 19) * channel->modulation * modulation_offset;
      dphase32.real = dphase32.bended + dphase32.moffset;
    }
    EnvelopeParam *getEnvelope() { return &(channel->envelope); }
    byte *getEnvelope(AdsrStatus adsr) { return getEnvelope()->getParam(adsr); }
    void updateEnvelopeStatus() {
      switch(adsr) {
        case ADSR_ATTACK: {
          unsigned long v = volume.v16;
          if( (v += (UINT_MAX >> *getEnvelope(ADSR_ATTACK))) > UINT_MAX ) {
            volume.v16 = UINT_MAX; adsr = ADSR_DECAY; break;
          }
          volume.v16 = v; break;
        }
        case ADSR_DECAY: {
          EnvelopeParam *e = getEnvelope();
          unsigned int dv = volume.v16 >> *(e->getParam(ADSR_DECAY));
          if( dv == 0 ) dv = 1;
          long v = volume.v16;
          unsigned int sustain16 = (unsigned int)(*(e->getParam(ADSR_SUSTAIN))) << 8;
          if( (v -= dv) <= sustain16 ) {
            volume.v16 = sustain16; adsr = ADSR_SUSTAIN; break;
          }
          volume.v16 = v; break;
        }
        case ADSR_RELEASE: {
          unsigned int dv = volume.v16 >> *getEnvelope(ADSR_RELEASE);
          if( dv == 0 ) dv = 1;
          long v = volume.v16;
          if( (v -= dv) < 0x100 ) {
            reset(); break;
          }
          volume.v16 = v; break;
        }
      }
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
#define EACH_CHANNEL(c) for(MidiChannel *(c)=channels; (c)<= &HighestElementOf(channels); (c)++)
    inline static void updatePulseWidth() {
      // To generate waveform rapidly in ISR(), this method must be inline
      unsigned int pw = 0;
      EACH_VOICE(v) pw += v->nextPulseWidth();
      PWMDAC_OCR = pw >> 8;
    }
    static void update() { // must be called from loop() repeatedly
      static byte modulation_phase = 0;
      int modulation_offset = pgm_read_byte(maxVolumeSineWavetable + (++modulation_phase)) - 0x7F;
      EACH_VOICE(v) v->update(modulation_offset);
    }
    static void noteOff(byte channel, byte pitch, byte velocity) {
      MidiChannel *cp = getChannel(channel);
      EACH_VOICE(v) v->noteOff(pitch, cp);
    }
    static void noteOn(byte channel, byte pitch, byte velocity) {
      assignVoice(getChannel(channel),pitch)->noteOn(pitch);
    }
    static void pitchBend(byte channel, int bend) {
      MidiChannel *cp = getChannel(channel);
      if ( ! cp->pitchBendChange(bend) ) return;
      EACH_VOICE(v) v->updatePitch(cp);
    }
    static void controlChange(byte channel, byte number, byte value) {
      MidiChannel *cp = getChannel(channel);
      cp->controlChange(number, value);
      switch(number) {
        case 120: // All sound off
          EACH_VOICE(v) v->allSoundOff(cp);
          break;
        case 123: // All notes off
          EACH_VOICE(v) v->allNotesOff(cp);
          break; 
      }
    }
    static void systemReset() {
      EACH_VOICE(v) v->reset();
      EACH_CHANNEL(c) c->reset(defaultInstrument);
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
    static PROGMEM const Instrument * const defaultInstrument;
    static MidiChannel channels[16];
    static VoiceStatus voices[PWMDAC_POLYPHONY];
    static PROGMEM const byte maxVolumeSineWavetable[];
    static VoiceStatus *assignVoice(MidiChannel *channel, byte note) {
      EACH_VOICE(v) if( v->isNoteOn(note, channel) ) return v;
      VoiceStatus *lowest_priority_voice = voices;
      byte lowest_priority = VoiceStatus::HIGHEST_PRIORITY;
      EACH_VOICE(v) {
        byte priority = v->getPriority();
        if( priority > lowest_priority ) continue;
        lowest_priority = priority;
        lowest_priority_voice = v;
      }
      lowest_priority_voice->setChannel(channel);
      return lowest_priority_voice;
    }
#undef EACH_VOICE
};

#define PWMDAC_CREATE_INSTANCE(instrument) \
  ISR(PWMDAC_OVF_vect) { PWMDACSynth::updatePulseWidth(); } \
  VoiceStatus PWMDACSynth::voices[PWMDAC_POLYPHONY]; \
  PWMDAC_CREATE_WAVETABLE(PWMDACSynth::maxVolumeSineWavetable, PWMDAC_MAX_VOLUME_SINE_WAVE); \
  PROGMEM const Instrument * const PWMDACSynth::defaultInstrument = instrument; \
  MidiChannel PWMDACSynth::channels[16] = MidiChannel(instrument);

