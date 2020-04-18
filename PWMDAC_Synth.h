//
// PWM DAC Synthesizer ver.20200418
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/pwmdac_synth_lib/
//  https://osdn.jp/users/kamide/pf/PWMDAC_Synth/
//
#pragma once
#pragma GCC push_options
#pragma GCC optimize ("-O3")

#include <Arduino.h>
#include <wiring_private.h>
#include <limits.h>

#define NumberOf(array) (sizeof(array)/sizeof(*(array)))
#define cbi16(sfr, bit) (_SFR_WORD(sfr) &= ~_BV(bit))
#define sbi16(sfr, bit) (_SFR_WORD(sfr) |= _BV(bit))

// Default parameter
#ifndef PWMDAC_OUTPUT_PIN
#define PWMDAC_OUTPUT_PIN 3
#endif
#ifndef PWMDAC_NOTE_A_FREQUENCY
#define PWMDAC_NOTE_A_FREQUENCY 440 // [Hz]
#endif
#ifndef PWMDAC_POLYPHONY
#define PWMDAC_POLYPHONY 6
#endif

//
// Built-in wavetable functions
//  x = Phase angle : 0x00...0x80(PI_radian)...0xFF
//  f(x) = Wave voltage at the x : 0x00(min)...0x80(center)...0xFF(max)
#define PWMDAC_SQUARE_WAVE(x)   (((x) < 0x80 ? 0x00 : 0xFF) / PWMDAC_POLYPHONY)
#define PWMDAC_SAWTOOTH_WAVE(x) ((x) / PWMDAC_POLYPHONY)
#define PWMDAC_TRIANGLE_WAVE(x) (((x) < 0x80 ? (x) : 0x100 - (x)) * 2 / PWMDAC_POLYPHONY)
#define PWMDAC_MAX_VOLUME_SINE_WAVE(x)  ((byte)( 0x80 * (1 + sin(PI * (x) / 0x80)) ))
#define PWMDAC_SINE_WAVE(x)     ((byte)( 0x80 * (1 + sin(PI * (x) / 0x80)) / PWMDAC_POLYPHONY ))
#define PWMDAC_SHEPARD_TONE(x)  ((byte)( 0x80 * (8 \
  +sin(PI * (x) / 0x80) \
  +sin(PI * (x) / 0x40) \
  +sin(PI * (x) / 0x20) \
  +sin(PI * (x) / 0x10) \
  +sin(PI * (x) / 0x08) \
  +sin(PI * (x) / 0x04) \
  +sin(PI * (x) / 0x02) \
  +sin(PI * (x) / 0x01) \
  ) / (8 * PWMDAC_POLYPHONY) ))

#define FX2(f,x)    f(x), f(x + 1)
#define FX4(f,x)    FX2(f,x), FX2(f,x + 2)
#define FX8(f,x)    FX4(f,x), FX4(f,x + 4)
#define FX16(f,x)   FX8(f,x), FX8(f,x + 8)
#define FX32(f,x)   FX16(f,x), FX16(f,x + 16)
#define FX64(f,x)   FX32(f,x), FX32(f,x + 32)
#define FX128(f,x)  FX64(f,x), FX64(f,x + 64)
#define FX256(f,x)  FX128(f,x), FX128(f,x + 128)

#define PWMDAC_CREATE_WAVETABLE(table, func) PROGMEM const byte table[] = { FX256(func,0) }

// [Phase-correct PWM dual-slope]
//    TCNTn value changes to:
//       00(BOTTOM) 01 02 03 ... FC FD FE
//       FF(TOP)    FE FD FC ... 03 02 01
//    -> # of values = 0xFF * 2 = 510 (NOT 512)
//
// ISR() call interval = (# of values) / F_CPU(16MHz) = 31.875us
//
// [MIDI Tuning Standard]
// http://en.wikipedia.org/wiki/MIDI_Tuning_Standard
//    fn(d) = 440 Hz * 2^( (d - 69) / 12 )  MIDI note # d = 0..127
//
#define PHASE_SPEED_OF(note_number) (static_cast<unsigned long>( \
  pow( 2, static_cast<double>(note_number - 69)/12 + (sizeof(unsigned long) * 8 + 1) ) \
  * PWMDAC_NOTE_A_FREQUENCY * 0xFF / F_CPU ))

// PWM Port macros
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

enum AdsrParam : byte {
  ADSR_RELEASE_VALUE, // 0..15
  ADSR_SUSTAIN_VALUE, // 0..255
  ADSR_DECAY_VALUE,   // 0..15
  ADSR_ATTACK_VALUE,  // 0..15
  NUMBER_OF_ADSR
};

typedef struct _Instrument {
  const byte *wavetable;
  const byte envelope[NUMBER_OF_ADSR];
} Instrument;

class MidiChannel {
  public:
    static const byte MAX_NUMBER = 16;
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
    const byte *wavetable;
    byte envelope[NUMBER_OF_ADSR];
    MidiChannel(const Instrument * const instrument) {
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
      setPriority(0);
#endif
      reset(instrument);
    }
    void reset(const Instrument * const instrument) {
      resetAllControllers();
      rpn.LSB = rpn.MSB = RPN_Null;
      data_entry_source = NULL;
      programChange(instrument);
    }
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
    void setPriority(const byte priority) {
      this->priority_volume_threshold = ~priority;
    }
#endif
    int getPitchBend() const { return pitch_bend; }
    unsigned long bendedPitchOf(const unsigned long original_pitch) const {
      return pitch_bend ? original_pitch * pitch_rate : original_pitch;
    }
    boolean pitchBendChange(const int new_pitch_bend) {
      int diff = new_pitch_bend - pitch_bend;
      if( diff < 16 && diff > -16 ) return false;
      pitch_bend = new_pitch_bend;
      updatePitchRate();
      return true;
    }
    byte getPitchBendSensitivity() const { return pitch_bend_sensitivity; }
    boolean pitchBendSensitivityChange(const byte value) {
      if ( pitch_bend_sensitivity == value ) return false;
      pitch_bend_sensitivity = value;
      updatePitchRate();
      return true;
    }
    void programChange(const Instrument * const instrument) {
      this->wavetable = (const byte *)pgm_read_word(&(instrument->wavetable));
      memcpy_P(this->envelope, instrument->envelope, sizeof(this->envelope));
    }
    void resetAllControllers() {
      modulation = 0;
      pitch_bend = 0;
      pitch_rate = 1.0;
      pitch_bend_sensitivity = 2;
    }
    void controlChange(const byte number, const byte value) {
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

class PWMDACSynth {
  protected:
    static PROGMEM const byte maxVolumeSineWavetable[];
    static MidiChannel channels[MidiChannel::MAX_NUMBER];
    static PROGMEM const Instrument * const defaultInstrument;
    class Voice {
      protected:
        enum AdsrStatus : byte {
          ADSR_OFF,
          ADSR_RELEASE,
          ADSR_SUSTAIN,
          ADSR_DECAY,
          ADSR_ATTACK
        };
        union {
          unsigned long v32;
          struct {
            byte lowest;
            byte lowest2;
            byte highest2;
            byte highest;
          } v8;
        } phase;
        unsigned long dphase32_real;     // Real phase speed
        unsigned long dphase32_bended;   // Pitch-bended phase speed
        unsigned long dphase32_original; // Original phase speed
        long dphase32_moffset;           // Modulation pitch offset
        union {
          unsigned int v16;
          struct {
            byte low;
            byte high;
          } v8;
        } volume;
        unsigned int dv;      // Diff of volume
        unsigned int sustain; // Volume when ADSR Sustain
        byte note; // 0..127
        AdsrStatus adsr_status;
        const byte *wavetable;
        const MidiChannel *channel;
        const byte *envelope;
        const byte *modulation;	// 0 .. 127 (Unsigned 7bit)
      public:
        static const byte HIGHEST_PRIORITY = UCHAR_MAX;
        static const byte LOWEST_PRIORITY = 0;
        Voice() { allSoundOff(); }
        byte getPriority() const {
          byte t = volume.v8.high >> 1;
          if( adsr_status == ADSR_ATTACK ) t = HIGHEST_PRIORITY - t;
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
          if( channel != nullptr && t > channel->priority_volume_threshold ) {
	    t >>= 1; t |= 0x80;
          } else {
	    t >>= 1;
          }
#endif
          return t;
        }
        void noteOn(const byte note, const MidiChannel * const new_channel = nullptr) {
          if( new_channel != nullptr && this->channel != new_channel ) {
	    volume.v16 = 0;
	    this->channel = new_channel;
          }
          wavetable = this->channel->wavetable;
          envelope = this->channel->envelope;
          modulation = &(this->channel->modulation);
          static PROGMEM const unsigned long phase_speed_table[] = { FX128(PHASE_SPEED_OF,0) };
          dphase32_original = pgm_read_dword(phase_speed_table + (this->note = note));
          dphase32_bended = this->channel->bendedPitchOf(dphase32_original);
          dphase32_real = dphase32_bended + dphase32_moffset;
          adsr_status = ADSR_ATTACK;
          dv = UINT_MAX >> envelope[ADSR_ATTACK_VALUE];
        }
        boolean reAttackIfAssigned(const byte note, const MidiChannel * const cp) {
          if( adsr_status > ADSR_RELEASE && this->note == note && channel == cp ) {
	    adsr_status = ADSR_ATTACK;
	    dv = UINT_MAX >> envelope[ADSR_ATTACK_VALUE];
	    return true;
          }
          return false;
        }
        void noteOff(const byte note, const MidiChannel * const cp) {
          if( adsr_status > ADSR_RELEASE && this->note == note && channel == cp )
	    adsr_status = ADSR_RELEASE;
        }
        void allNotesOff(const MidiChannel * const cp) {
          if( adsr_status > ADSR_RELEASE && channel == cp ) adsr_status = ADSR_RELEASE;
        }
        void allSoundOff(const MidiChannel * const cp) {
          if( adsr_status > ADSR_OFF && channel == cp ) allSoundOff();
        }
        void allSoundOff() {
          volume.v16 = 0;
          adsr_status = ADSR_OFF;
          note = UCHAR_MAX;
          phase.v32 = dphase32_real = dphase32_moffset = dphase32_bended = dphase32_original = 0L;
          channel = nullptr;
        }
        unsigned int nextPulseWidth() {
          phase.v32 += dphase32_real;
          return volume.v8.high * pgm_read_byte(wavetable + phase.v8.highest);
        }
        void update(const byte modulation_offset) {
          // Update volume by ADSR envelope
          switch(adsr_status) {
            case ADSR_ATTACK:
              if( volume.v16 < UINT_MAX - dv ) volume.v16 += dv;
              else {
                volume.v16 = UINT_MAX;
                adsr_status = ADSR_DECAY;
                sustain = static_cast<unsigned int>(envelope[ADSR_SUSTAIN_VALUE]) << 8;
              }
              break;
            case ADSR_DECAY:
              dv = volume.v16 >> envelope[ADSR_DECAY_VALUE];
              if( dv == 0 ) dv = 1;
              if( volume.v16 > sustain + dv ) volume.v16 -= dv;
              else { volume.v16 = sustain; adsr_status = ADSR_SUSTAIN; }
              break;
            case ADSR_SUSTAIN: break;
            case ADSR_RELEASE:
              dv = volume.v16 >> envelope[ADSR_RELEASE_VALUE];
              if( dv == 0 ) dv = 1;
              if( volume.v16 > 0x100 + dv ) volume.v16 -= dv;
              else allSoundOff();
              break;
            case ADSR_OFF: break;
          }
          // Update frequency by modulation
          if( *modulation <= 0x10 ) {
            // When Moduletion OFF
            if( dphase32_moffset == 0 ) return;
            dphase32_moffset = 0;
            dphase32_real = dphase32_bended;
            return;
          }
          dphase32_moffset = (dphase32_real >> 19) * (*modulation) * modulation_offset;
          dphase32_real = dphase32_bended + dphase32_moffset;
        }
        void updatePitch(const MidiChannel * const cp) {
          // Must be called at pitch-bend change on the channel
          if( adsr_status > ADSR_OFF && channel == cp ) {
            dphase32_bended = channel->bendedPitchOf(dphase32_original);
            dphase32_real = dphase32_bended + dphase32_moffset;
          }
        }
    };
    static Voice voices[PWMDAC_POLYPHONY];
  public:
    static void setup() {
      // must be called from your setup() once
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
    static MidiChannel *getChannel(const char channel) {
      return channels + (channel - 1);
    }
    static char getChannel(const MidiChannel *cp) {
      return (cp - channels) + 1;
    }
    static void noteOff(const byte channel, const byte pitch, const byte velocity) {
      (void)velocity;
      MidiChannel *cp = getChannel(channel);
      byte i = NumberOf(voices); while( i > 0 ) voices[--i].noteOff(pitch, cp);
    }
    static void noteOn(const byte channel, const byte pitch, const byte velocity) {
      (void)velocity;
      const MidiChannel * const cp = getChannel(channel);
      byte i = NumberOf(voices);
      while(i) if( voices[--i].reAttackIfAssigned(pitch, cp) ) return;
      // Search voice to assign
      byte lowest_priority = Voice::HIGHEST_PRIORITY;
      byte lowest_priority_index = 0;
      i = NumberOf(voices);
      while(i) {
        const byte priority = voices[--i].getPriority();
        if( priority > lowest_priority ) continue;
        lowest_priority = priority;
        lowest_priority_index = i;
      }
      voices[lowest_priority_index].noteOn(pitch, cp);
    }
    static void pitchBend(const byte channel, const int bend) {
      MidiChannel *cp = getChannel(channel);
      if ( ! cp->pitchBendChange(bend) ) return;
      byte i = NumberOf(voices); while(i) voices[--i].updatePitch(cp);
    }
    static void controlChange(const byte channel, const byte number, const byte value) {
      MidiChannel *cp = getChannel(channel);
      cp->controlChange(number, value);
      byte i;
      switch(number) {
        case 120: // All sound off
          i = NumberOf(voices); while(i) voices[--i].allSoundOff(cp);
          break;
        case 123: // All notes off
          i = NumberOf(voices); while(i) voices[--i].allNotesOff(cp);
          break; 
      }
    }
    static void systemReset() {
      byte i = NumberOf(voices); while(i) voices[--i].allSoundOff();
      i = NumberOf(channels); while(i) channels[--i].reset(defaultInstrument);
    }
    static byte __nextPulseWidth() {
      union { unsigned int v16; struct {byte low; byte high;} v8; } pw = {0};
      byte i = NumberOf(voices);
      while(i) pw.v16 += voices[--i].nextPulseWidth();
      return pw.v8.high;
    }
    static void update() {
      // must be called from your loop() repeatedly
      static byte modulation_phase = 0;
      const byte *addr = maxVolumeSineWavetable + modulation_phase++;
      const byte modulation_offset = pgm_read_byte(addr) - 0x7F;
      byte i = NumberOf(voices);
      while(i) voices[--i].update(modulation_offset);
    }
};

#define PWMDAC_CREATE_INSTANCE(instrument) \
  PWMDAC_CREATE_WAVETABLE(PWMDACSynth::maxVolumeSineWavetable, PWMDAC_MAX_VOLUME_SINE_WAVE); \
  MidiChannel PWMDACSynth::channels[] = { \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
\
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
\
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
\
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
    MidiChannel(instrument), \
  }; \
  const Instrument * const PWMDACSynth::defaultInstrument PROGMEM = instrument; \
  PWMDACSynth::Voice PWMDACSynth::voices[]; \
  ISR(PWMDAC_OVF_vect) { PWMDAC_OCR = PWMDACSynth::__nextPulseWidth(); }

#pragma GCC pop_options

