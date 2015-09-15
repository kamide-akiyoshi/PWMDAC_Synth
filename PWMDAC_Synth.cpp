//
// PWM DAC Synthesizer ver.20150915
//
#include "PWMDAC_Synth.h"

MidiChannel PWMDACSynth::channels[16] = MidiChannel(PWMDACSynth::sineWavetable);
VoiceStatus PWMDACSynth::voices[POLYPHONY];

byte PWMDACSynth::musicalMod7(char x) {
  while( x & 0xF8 ) x = (x >> 3) + (x & 7);
  if(x==7) return 0;
  return x;
}
byte PWMDACSynth::musicalMod12(char x) {
  char n = x >> 2;
  while( n & 0xFC ) n = (n >> 2) + (n & 3);
  x &= 3;
  if(n==3||n==0) return x;
  return x + (n << 2);
}
byte PWMDACSynth::log2(unsigned int x) {
  byte index = 0;
  if (x & 0xFF00) { index += 8; x &= 0xFF00; }
  if (x & 0xF0F0) { index += 4; x &= 0xF0F0; }
  if (x & 0xCCCC) { index += 2; x &= 0xCCCC; }
  if (x & 0xAAAA) { index += 1; x &= 0xAAAA; }
  return index;
}

//
// Built-in wavetables (in 6-polyphonic, 255/6 = 42.5 -> 0..42)
//
PROGMEM const byte PWMDACSynth::sineWavetable[] = {
21,	21,	22,	22,	23,	23,	24,	24,
25,	25,	26,	26,	27,	27,	28,	28,
29,	29,	30,	30,	31,	31,	32,	32,
32,	33,	33,	34,	34,	34,	35,	35,
36,	36,	36,	37,	37,	37,	38,	38,
38,	39,	39,	39,	39,	40,	40,	40,
40,	40,	41,	41,	41,	41,	41,	41,
41,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
41,	41,	41,	41,	41,	41,	41,	40,
40,	40,	40,	40,	39,	39,	39,	39,
38,	38,	38,	37,	37,	37,	36,	36,
36,	35,	35,	34,	34,	34,	33,	33,
32,	32,	32,	31,	31,	30,	30,	29,
29,	28,	28,	27,	27,	26,	26,	25,
25,	24,	24,	23,	23,	22,	22,	21,
21,	20,	20,	19,	19,	18,	18,	17,
17,	16,	16,	15,	15,	14,	14,	13,
13,	12,	12,	11,	11,	10,	10,	9,
9,	8,	8,	8,	7,	7,	6,	6,
6,	5,	5,	5,	4,	4,	4,	3,
3,	3,	3,	2,	2,	2,	2,	1,
1,	1,	1,	1,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	1,	1,	1,
1,	1,	2,	2,	2,	2,	3,	3,
3,	3,	4,	4,	4,	5,	5,	5,
6,	6,	6,	7,	7,	8,	8,	8,
9,	9,	10,	10,	11,	11,	12,	12,
13,	13,	14,	14,	15,	15,	16,	16,
17,	17,	18,	18,	19,	19,	20,	20,
};
PROGMEM const byte PWMDACSynth::squareWavetable[] = {
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	0,	0,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
42,	42,	42,	42,	42,	42,	42,	42,
};
PROGMEM const byte PWMDACSynth::triangleWavetable[] = {
0,	0,	0,	1,	1,	1,	2,	2,
2,	3,	3,	3,	4,	4,	4,	5,
5,	5,	6,	6,	6,	7,	7,	7,
8,	8,	8,	9,	9,	9,	10,	10,
10,	11,	11,	11,	12,	12,	12,	13,
13,	13,	14,	14,	14,	15,	15,	15,
16,	16,	16,	17,	17,	17,	18,	18,
18,	19,	19,	19,	20,	20,	20,	21,
21,	21,	22,	22,	22,	23,	23,	23,
24,	24,	24,	25,	25,	25,	26,	26,
26,	27,	27,	27,	28,	28,	28,	29,
29,	29,	30,	30,	30,	31,	31,	31,
32,	32,	32,	33,	33,	33,	34,	34,
34,	35,	35,	35,	36,	36,	36,	37,
37,	37,	38,	38,	38,	39,	39,	39,
40,	40,	40,	41,	41,	41,	42,	42,
42,	42,	42,	41,	41,	41,	40,	40,
40,	39,	39,	39,	38,	38,	38,	37,
37,	37,	36,	36,	36,	35,	35,	35,
34,	34,	34,	33,	33,	33,	32,	32,
32,	31,	31,	31,	30,	30,	30,	29,
29,	29,	28,	28,	28,	27,	27,	27,
26,	26,	26,	25,	25,	25,	24,	24,
24,	23,	23,	23,	22,	22,	22,	21,
21,	21,	20,	20,	20,	19,	19,	19,
18,	18,	18,	17,	17,	17,	16,	16,
16,	15,	15,	15,	14,	14,	14,	13,
13,	13,	12,	12,	12,	11,	11,	11,
10,	10,	10,	9,	9,	9,	8,	8,
8,	7,	7,	7,	6,	6,	6,	5,
5,	5,	4,	4,	4,	3,	3,	3,
2,	2,	2,	1,	1,	1,	0,	0,
};
PROGMEM const byte PWMDACSynth::sawtoothWavetable[] = {
0,	0,	0,	0,	0,	0,	1,	1,
1,	1,	1,	1,	2,	2,	2,	2,
2,	2,	3,	3,	3,	3,	3,	3,
4,	4,	4,	4,	4,	4,	5,	5,
5,	5,	5,	5,	6,	6,	6,	6,
6,	6,	7,	7,	7,	7,	7,	7,
8,	8,	8,	8,	8,	8,	9,	9,
9,	9,	9,	9,	10,	10,	10,	10,
10,	10,	11,	11,	11,	11,	11,	11,
12,	12,	12,	12,	12,	12,	13,	13,
13,	13,	13,	13,	14,	14,	14,	14,
14,	14,	15,	15,	15,	15,	15,	15,
16,	16,	16,	16,	16,	16,	17,	17,
17,	17,	17,	17,	18,	18,	18,	18,
18,	18,	19,	19,	19,	19,	19,	19,
20,	20,	20,	20,	20,	20,	21,	21,
21,	21,	21,	21,	22,	22,	22,	22,
22,	22,	23,	23,	23,	23,	23,	23,
24,	24,	24,	24,	24,	24,	25,	25,
25,	25,	25,	25,	26,	26,	26,	26,
26,	26,	27,	27,	27,	27,	27,	27,
28,	28,	28,	28,	28,	28,	29,	29,
29,	29,	29,	29,	30,	30,	30,	30,
30,	30,	31,	31,	31,	31,	31,	31,
32,	32,	32,	32,	32,	32,	33,	33,
33,	33,	33,	33,	34,	34,	34,	34,
34,	34,	35,	35,	35,	35,	35,	35,
36,	36,	36,	36,	36,	36,	37,	37,
37,	37,	37,	37,	38,	38,	38,	38,
38,	38,	39,	39,	39,	39,	39,	39,
40,	40,	40,	40,	40,	40,	41,	41,
41,	41,	41,	41,	42,	42,	42,	42,
};
PROGMEM const byte PWMDACSynth::shepardToneSineWavetable[] = {
21,	33,	33,	29,	33,	36,	29,	25,
32,	40,	35,	27,	28,	30,	24,	21,
31,	42,	39,	32,	34,	34,	26,	20,
27,	34,	28,	21,	22,	25,	19,	18,
29,	41,	39,	34,	37,	38,	30,	25,
31,	38,	32,	23,	23,	24,	17,	14,
24,	34,	31,	24,	25,	26,	18,	12,
19,	26,	21,	14,	16,	19,	15,	14,
26,	38,	37,	33,	36,	38,	30,	26,
33,	39,	34,	26,	26,	27,	20,	17,
27,	36,	33,	26,	27,	27,	19,	12,
19,	25,	19,	12,	13,	15,	10,	8,
19,	30,	29,	24,	26,	27,	19,	14,
20,	27,	21,	12,	13,	14,	7,	4,
14,	24,	21,	15,	16,	17,	9,	4,
11,	18,	14,	7,	9,	13,	9,	8,
21,	33,	33,	29,	32,	34,	28,	23,
30,	38,	32,	25,	25,	27,	20,	18,
28,	37,	34,	28,	29,	29,	21,	15,
21,	28,	22,	14,	16,	18,	13,	11,
22,	33,	32,	26,	29,	30,	22,	16,
23,	29,	23,	14,	14,	15,	8,	5,
14,	24,	21,	14,	15,	16,	8,	2,
9,	16,	11,	4,	6,	9,	4,	3,
15,	28,	27,	22,	25,	27,	20,	15,
22,	29,	24,	16,	16,	17,	11,	8,
17,	27,	24,	17,	18,	18,	10,	4,
10,	17,	11,	3,	5,	7,	2,	0,
12,	23,	22,	17,	19,	21,	13,	8,
15,	21,	15,	7,	8,	9,	3,	0,
10,	20,	17,	11,	13,	14,	6,	1,
9,	17,	12,	6,	8,	12,	8,	8,
};
PROGMEM const byte PWMDACSynth::maxVolumeSineWavetable[] = {
127,	130,	133,	136,	139,	142,	145,	148,
151,	154,	157,	160,	163,	166,	169,	172,
175,	178,	181,	184,	186,	189,	192,	194,
197,	200,	202,	205,	207,	209,	212,	214,
216,	218,	221,	223,	225,	227,	229,	230,
232,	234,	235,	237,	239,	240,	241,	243,
244,	245,	246,	247,	248,	249,	250,	250,
251,	252,	252,	253,	253,	253,	253,	253,
254,	253,	253,	253,	253,	253,	252,	252,
251,	250,	250,	249,	248,	247,	246,	245,
244,	243,	241,	240,	239,	237,	235,	234,
232,	230,	229,	227,	225,	223,	221,	218,
216,	214,	212,	209,	207,	205,	202,	200,
197,	194,	192,	189,	186,	184,	181,	178,
175,	172,	169,	166,	163,	160,	157,	154,
151,	148,	145,	142,	139,	136,	133,	130,
127,	123,	120,	117,	114,	111,	108,	105,
102,	99,	96,	93,	90,	87,	84,	81,
78,	75,	72,	69,	67,	64,	61,	59,
56,	53,	51,	48,	46,	44,	41,	39,
37,	35,	32,	30,	28,	26,	24,	23,
21,	19,	18,	16,	14,	13,	12,	10,
9,	8,	7,	6,	5,	4,	3,	3,
2,	1,	1,	0,	0,	0,	0,	0,
0,	0,	0,	0,	0,	0,	1,	1,
2,	3,	3,	4,	5,	6,	7,	8,
9,	10,	12,	13,	14,	16,	18,	19,
21,	23,	24,	26,	28,	30,	32,	35,
37,	39,	41,	44,	46,	48,	51,	53,
56,	59,	61,	64,	67,	69,	72,	75,
78,	81,	84,	87,	90,	93,	96,	99,
102,	105,	108,	111,	114,	117,	120,	123,
};

