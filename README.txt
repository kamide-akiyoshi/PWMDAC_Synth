
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20160611

Arduino�œ��삷��ȈՃV���Z�T�C�U���C�u�����ł��B

https://osdn.jp/users/kamide/pf/PWMDAC_Synth/wiki/FrontPage

����� CAmiDion

  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/

�̂Q���@�ȍ~�Ŏ����������������C�u�������������̂ł��B


��Arduino�Ŋy�����낤�Ƃ��āA����Ȗ��ɂԂ������������Ƃ͂���܂��񂩁H

�E�����o���Ȃ� tone() �������葁�����ǁA�a�����o�Ȃ��c

�EanalogWrite() �� PWM �o�͂��Ă��A�p���X�g�̎��g����490Hz��
  �Ⴗ���邩��A1kHz�O��̉��Ȃ�ď悹���Ȃ��A�ǂ����悤�c

����Ȗ����AAVR�̃v���X�P�[���i������j�̐ݒ��ς���
�p���X�g�̎��g�����ő�ɂ���Έ�C�ɉ����B

���̃��C�u�����́A�K���������ő��̎����Ŕ����������^�C�}�[���荞�݂ŁA
���̏u�Ԃ̔g�̍����ɉ����APWM�̃p���X���𖈉񃊃A���^�C����
�f�����X�V���邱�Ƃɂ��A��`�g�����łȂ��F�X�Ȕg�`�̉����o�͂��܂��B

�O�������`�b�v�Ȃ��� Arduino �{�̂��̂��̂�
�V���Z�T�C�U�̉����������������Ƃ��Ɋ��p���Ă��������B


���C���X�g�[��

�W�J���� PWMDAC_Synth �t�H���_�����L�̃t�H���_��
�u�������ŃC���X�g�[���ł��܂��B

	�}�C�h�L�������g\Arduino\libraries\

�u������AArduino IDE �̃��j���[ [�t�@�C��] �� [�X�P�b�`�̗�] ��
PWMDAC_Synth ������邱�Ƃ��m�F���Ă��������B


���g����

	// �K�v�ɉ����APWMDAC_Synth.h ���C���N���[�h����O�� #define �ŉ��L���w��ł��܂��B
	//
	#define PWMDAC_OUTPUT_PIN  3 // PWM�o�̓s���ԍ��i�ȗ��F���L�Q�Ɓj
	#define PWMDAC_POLYPHONY   6 // �����������i�ȗ��F���L�Q�Ɓj
	#define PWMDAC_NOTE_A_FREQUENCY 440  // A���̃`���[�j���O���g���i�ȗ��F�f�t�H���g440Hz�j

	#include <PWMDAC_Synth.h>
	//
	// �g�`�e�[�u�����A�K�v�ȕ�������`���܂��B
	// �i�s�v�Ȕg�`���`���Ȃ��悤�ɂ��邱�ƂŁA�v���O�����������̈�̐ߖ�ɂȂ�܂��j
	//
	PWMDAC_CREATE_WAVETABLE(squareWavetable, PWMDAC_SQUARE_WAVE);      // ��`�g
	PWMDAC_CREATE_WAVETABLE(triangleWavetable, PWMDAC_TRIANGLE_WAVE);  // �O�p�g
	PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);          // �����g
	PWMDAC_CREATE_WAVETABLE(shepardToneSineWavetable, PWMDAC_SHEPARD_TONE);  // �V�F�p�[�h�g�[���i�������K�j
	PWMDAC_CREATE_WAVETABLE(sawtoothWavetable, PWMDAC_SAWTOOTH_WAVE);  // �̂�����g
	//
	// �g�`�ƃG���x���[�v�p�����[�^�iADSR�j�� Instrument �\���̂ɑ��˂�
	// �v���O�����������̈�ɉ��F�f�[�^�𐶐����܂��B
	// ������ {} �ɂ�ADSR�̒l���t���Ŏw�肵�܂��i�l�͈̔͂ɂ��Ă͌�q�́u�����F�ύX�v���Q�Ɓj�B
	//
	PROGMEM const Instrument instrument = {sawtoothWavetable, {9, 0, 11, 4}};
	//
	// �����������F���w�肵��PWMDAC_Synth�̎��́i�C���X�^���X�j�𐶐����܂��B
	// MIDI�`�����l��1�`16�����ׂĂ��̉��F�ɏ���������܂��B
	//
	PWMDAC_CREATE_INSTANCE(&instrument);

	setup() {
		PWMDACSynth::setup(); // �������i�K�{�j
	}

	loop() {
		PWMDACSynth::update(); // ��ԍX�V�i�K�{�j
	}


PWMDACSynth::update() �́A�����Ȃǂ�ADSR�G���x���[�v�`���
���݈ʒu����i�߂���A���W�����[�V�����ɂ���ĕω����鎞�Ԃ��Ƃ�
���g�����X�V���邽�߂̊֐��ł��B�Ăяo���p�x�͎����ŃJ�E���^��
�p�ӂ���Ȃǂ̕��@�ŃR���g���[�����Ă��������B

�t���̃T���v���X�P�b�`�iexamples/*.ino�j�Ɏ����Ⴊ����܂��B
�d�q�y�� CAmiDion �ȂǁA���ۂ� PWMDAC_Synth ���g���Ă���X�P�b�`��
�Q�l�ɂ��Ă��������B

���̑��A�g����֐��ɂ��Ă� PWMDAC_Synth.h ���Q�Ƃ��Ă��������B


������������

	�f�t�H���g��6�d�a���ł��B

	PWMDAC_POLYPHONY �ɐ��l��ݒ肵�ăR���p�C�����������ƂŁA
	�����������𑝌������邱�Ƃ��ł��܂��B

	�����������𑝂₵������ƁA���̕��A���荞�ݏ����Ɏ��Ԃ���������
	���荞�݈ȊO�̏������s���]�T���Ȃ��Ȃ�A����ɓ��삵�Ȃ��Ȃ�܂��B


���o�̓s��(PWM)

	PWM�����o�͗p�Ƃ��Ďw��ł���s���ԍ��� 3,9,10,11 �̂����ꂩ�ł��B
	�f�t�H���g�̃s���ԍ��� 3 �ł��B

	3,11 ��I�������ꍇ�� TIMER2�A9,10 ��I�������ꍇ�� TIMER1 ���g���܂��B

	�Ȃ��A5,6 �� PWM �[�q�ł����Ă��w��ł��܂���B
	����� TIMER0 �p�� ISR() �� Arduino �� millis() �Ȃǂ̂��߂Ɏg���Ă���
	�Ē�`�ł��Ȃ��������߂ł��B

	PWM �o�̓s���́A���̂܂� PC �� LINE IN �ȂǂɂȂ��ł��ꉞ���͕������܂����A
	���[�p�X�t�B���^�ŉ����g��������ʂ��悤�ɂ���ƁA��蕷���₷���Ȃ�܂��B


��MIDI�`�����l������

	PWM_SYNTH �̃��\�b�h�ŁA

		MIDI�`�����l���ԍ�(1�`16) �� MIDI�`�����l���ւ̃|�C���^

	�𑊌ݕϊ��ł��܂��B

	MidiChannel *getChannel(char channel)
	char getChannel(MidiChannel *cp)

	MIDI�`�����l���ւ̃|�C���^����ă`�����l���P�ʂɎ�������
	�p�����[�^�𑀍�ł��܂��B

	�������A�m�[�g�I���A�m�[�g�I�t�A�s�b�`�x���h�Ȃǂ����o�Ă��鉹��
	�����ɔ��f�����邽�߂ɂ́AMidiChannel �ł͂Ȃ� PWMDACSynth ��
	�����o�֐����Ăяo���K�v������܂��B


��MIDI�֐�

	PWM_SYNTH �ÓI�I�u�W�F�N�g�ɂ́AMIDI���C�u������ MIDI.setHandleXxxx() ��
	���ڎw��ł���悤�A�����̏�����^�����킹���֐����������p�ӂ��Ă��܂��B

	�������A�ڑ������MIDI�f�o�C�X�ɂ���ẮANOTE OFF �̑����
	velocity=0 �� NOTE ON �𑗂��Ă��邱�Ƃ�����̂ŁA���̏ꍇ����
	NOTE OFF ���Ăяo���Ƃ������H�v���K�v�ɂȂ�܂��B

	MIDI�`�����l���ԍ��́AMIDI���C�u�����ɍ��킹�� 1�`16 �͈̔͂�
	�w�肷��悤�ɍ���Ă���܂��B


�����F�ύX

	�g�`�ƃG���x���[�v�p�����[�^�iADSR�j�� MidiChannel �N���X��
	wavetable �� env_param �Ɏw�肷�邱�ƂŁA���F��ύX�ł��܂��B

	ADSR�̐ݒ�� EnvelopeParam �N���X����čs���܂��B
	�R���X�g���N�^�Ŏ��̒l���w�肵�ď������ł��܂��B

	�Eattack_time - �A�^�b�N���ԁi�傫���قǃm�[�g�I������̗����オ�肪�������j
	�Edecay_time - �f�B�P�C���ԁi�傫���قǃm�[�g�I����̌������������j
	�Esustain_level - �T�X�e�B�����x���i�������~�܂������ƈێ����鉹�ʁj
	�Erelease_time - �����[�X���ԁi�傫���قǃm�[�g�I�t��̌������������j

	�l�͈̔͂� sustain_level �� 0�`255�A����ȊO�� 0�`15 �ł��B
	�����Ԃ� loop() ���� update() ���Ăяo���p�x�ɂ���ĕς��܂��B

	�eADSR�p�����[�^�l�ւ̃|�C���^�� getParam() ���\�b�h�Ŏ擾�ł��܂��B

	�g�`�e�[�u���́A�v�f��256��byte�^PROGMEM�z��Ƃ��Đ������܂��B
	����� wavetable �Ɏw�肷�邱�ƂŁA�g�`��؂�ւ��邱�Ƃ��ł��܂��B

	�g�`�e�[�u���z��̐����́APWMDAC_Synth.h ��Œ�`���ꂽ
	�}�N�����g���A�K�v�Ȕg�`�e�[�u�������𐶐����Ă��������B

	���̃}�N���́A�����������������Ă��M�����x����255�𒴂��ĉ����ꂹ���A
	���ő剹�ʂ̏o��悤�Ȍv�Z���ɂȂ��Ă��܂��B
	����ɏK���Ď����Ŕg�`���`����΁A������w�肵��
	�Ǝ��̉��F�𐶂ݏo�����Ƃ��\�ł��B

	�v���O�����������̈�� Instrument �\���̒萔������āA
	������w�肷�邱�Ƃŉ��F��ύX���邱�Ƃ��ł��܂��B
	MIDI�̃v���O�����`�F���W�̎����ɂ͂��̕��@���������߂ł��B
	���݂̃o�[�W�����ł́A�v���O�����`�F���W�Ή��ɔ����A
	�G���x���[�v�̏������� Instrument �\���̒萔���g���C���^�[�t�F�[�X��
	����RAM�ւ̓W�J���ŏ����ɗ}������悤�ɂ��܂����B


�����[�e�B���e�B
	byte musicalMod7(char x)
	byte musicalMod12(char x)
		���ꂼ��7�Ŋ������]��A12�Ŋ������]���Ԃ��܂��B
		% ���Z�q�ƈႢ�A������^���Ă������ŕԂ��܂��B
		�����I�ɂ̓r�b�g���Z�Ɖ��Z�����ō����Ɍv�Z���܂��B
		���K�̌v�Z�ɕ֗��ł��B

	byte log2(unsigned int x)
		�Q���Ƃ���ΐ����A�����_�ȉ���؎̂Ă������ŕԂ��܂��B
		y = log2(x) �� x = 1 << y �͋t�֐��̊֌W�ɂȂ�܂��B

	int musicalConstrain12(int note, int min_note, int max_note)
		Arduino �� constrain() �����K�p�Ɏ������������֐��B
		note �� min_note �` max_note �̉���ɂ����
		note �����̂܂ܕԂ��܂��B������͂ݏo���Ă����ꍇ�A
		���K��ς����ɃI�N�^�[�u�ʒu��ς��邱�Ƃɂ��A
		�͈͂Ɏ��܂�悤��������܂��B


��ҁF�����悵 - Akiyoshi Kamide
	http://www.yk.rim.or.jp/~kamide/
