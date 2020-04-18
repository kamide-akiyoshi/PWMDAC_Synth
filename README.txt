
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20200418

https://osdn.jp/users/kamide/pf/PWMDAC_Synth/wiki/FrontPage


����́ACAmiDion

  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/

�̂Q���@�ȍ~�Ŏ����������������C�u�����Ƃ��ēƗ��������A
Arduino�݊��@�p��PWM�V���Z�T�C�U���C�u�����ł��B


��Arduino�Ŋy�����낤�Ƃ��āA����Ȗ��ɂԂ������������Ƃ͂���܂��񂩁H

�E�����o���Ȃ� tone() �������葁�����ǁA�a�����o�Ȃ��c

�EanalogWrite() �� PWM �o�͂��Ă��A�p���X�g�̎��g����490Hz��
  �Ⴗ���邩��A1kHz�O��̉��Ȃ�ď悹���Ȃ��A�ǂ����悤�c

����Ȗ����AAVR�̃v���X�P�[���i������j�̐ݒ��ς���
�p���X�g�̎��g�����ő�ɂ���Έ�C�ɉ����B

���̃��C�u�����́A�K���������ő��̎����Ŕ����������^�C�}�[���荞�݂ŁA
���̏u�Ԃ̔g�̍����ɉ����APWM�̃p���X���𖈉񃊃A���^�C���ɑf�����X�V����
���Ƃɂ��A��`�g�����łȂ��A�F�X�Ȕg�`�̉����o�͂��܂��B

�����̃{�C�X���g�������d���Z�ɂ��PWM�̃p���X�����v�Z���Ă���̂ŁA
�a���̏o�͂��\�ł��B

�O�������`�b�v�Ȃ��� Arduino �{�̂��̂��̂ɃV���Z�T�C�U�̉�����
�����������Ƃ��Ɋ��p���Ă��������B


���C���X�g�[��

	�W�J���� PWMDAC_Synth �t�H���_���AWindows ��

		�}�C�h�L�������g\Arduino\libraries\

	�ɒu�������ŃC���X�g�[���ł��܂��B

	�u������AArduino IDE �̃��j���[ [�t�@�C��] �� [�X�P�b�`�̗�] ��
	PWMDAC_Synth ������邱�Ƃ��m�F���Ă��������B


���g����

	��{�I�Ȏg���������L�Ɏ����܂��B�Ȃ��A�m�[�g�I���A�m�[�g�I�t�ȂǁA���̏o�����ɂ��Ă�
	Arduino IDE �̃��j���[���炽�ǂ��u�X�P�b�`�̗�v���Q�l�ɂ��Ă��������B

	�y�T���v���R�[�h�z
	//
	// �K�v�ɉ����APWMDAC_Synth.h ���C���N���[�h����O�� #define �ŉ��L���w��ł��܂��B
	//
	#define PWMDAC_OUTPUT_PIN  3 // PWM�o�̓s���ԍ��i�ȗ��F���L�u���o�̓s��(PWM)�v�Q�Ɓj
	#define PWMDAC_POLYPHONY   6 // �����������i�ȗ��F���L�u�������������v���Q�Ɓj
	#define PWMDAC_NOTE_A_FREQUENCY 440  // A���̃`���[�j���O���g���i�ȗ��F�f�t�H���g440Hz�j
	#define PWMDAC_CHANNEL_PRIORITY_SUPPORT // ����`�����l���̗D��x���グ��@�\��L�����i�g��Ȃ��ꍇ�͏ȗ��j
	#include <PWMDAC_Synth.h>
	//
	// �g�`�e�[�u�����A�K�v�ȕ�������`���܂��B
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
#if defined(PWMDAC_CHANNEL_PRIORITY_SUPPORT)
	// �K�v�ɉ����āA�����f�B�p�[�g�������Ă���MIDI�`�����l���̗D��x���������܂�
	//�i�ڍׂɂ��Ă͌�q�́u���`�����l���D��x�w��v���Q�Ɓj�B
	PWMDACSynth::getChannel(1)->setPriority(0xC0);
#endif
	}

	loop() {
		PWMDACSynth::update(); // ��ԍX�V�i�K�{�j
		// �� ����́A�����Ȃǂ�ADSR�G���x���[�v�`��̌��݈ʒu����i�߂���A
		// ���W�����[�V�����ɂ���ĕω����鎞�Ԃ��Ƃ̎��g�����X�V���邽�߂̊֐��ł��B
		// �Ăяo���p�x�͎����ŃJ�E���^��p�ӂ���Ȃǂ̕��@�ŃR���g���[�����Ă��������B
	}


�������������i�{�C�X���j

	�f�t�H���g��6�d�a���ł��B
	PWMDAC_POLYPHONY �ɐ��l��ݒ肵�ăR���p�C�����������Ƃő����\�ł��B

	�� �����������𑝂₵�Ă����ƁA���̕�
	ISR() (Interrupt Service Routine: ���荞�݃T�[�r�X���[�`��)�ɂ�����
	���[�v�񐔂������A���Z�ɂȂ��Ă��܂��B���܂�ɑ��Z�߂��� loop() �����s����
	�ɂ��Ȃ��Ȃ�ƁA����s�\�Ɋׂ�܂��B�����Ȃ鐡�O���A�����������̎�����̏���ł��B

	�� ISR() �Ȃǂœ��쑬�x���v������邽�߁APWMDAC_Synth.h �ɂ�
	#pragma GCC optimize ("-O3")
	���w�肳��Ă��܂��B�R���p�C���̍œK������A���̃��C�u�����������x�D��ɂ��邽�߂ł��B

	�����MIDI�`�����l���ɑ΂��{�C�X�A�T�C���̗D��x���グ�邱�Ƃ��ł��܂��B
	��q�́u���`�����l���D��x�w��v���Q�ƁB


���o�̓s��(PWM)

	PWM�����o�͗p�Ƃ��Ďw��ł���s���ԍ��� 3,9,10,11 �̂����ꂩ�ł��B
	�f�t�H���g�̃s���ԍ��� 3 �ł��B

	3,11 ��I�������ꍇ�� TIMER2�A9,10 ��I�������ꍇ�� TIMER1 ���g���܂��B

	�Ȃ��A�s���ԍ� 5,6 �� PWM �[�q�ł����A������w�肵�Ă��R���p�C���G���[�ɂȂ�A
	���p�ł��܂���B����� TIMER0 �p�� ISR() ���AArduino �� millis() �֐���
	�������邽�߂��łɎg���Ă��āA�Ē�`�ł��Ȃ��������߂ł��B

	PWM �o�̓s���́A���̂܂� PC �� LINE IN �ȂǂɂȂ��ł��ꉞ���͕������܂����A
	���[�p�X�t�B���^�ŉ����g��������ʂ��悤�ɂ���ƁA��蕷���₷���Ȃ�܂��B


��MIDI�`�����l������

	PWMDACSynth::getChannel() ��

		MIDI�`�����l���ԍ�(1�`16) �� MIDI�`�����l���ւ̃|�C���^

	�𑊌ݕϊ��ł��܂��BMIDI�`�����l���ւ̃|�C���^����āA
	�`�����l���P�ʂɎ�������p�����[�^�𑀍�ł��܂��B
	�i������@�ɂ��Ă� PWMDAC_Synth.h ���Q�Ɓj

	�Ȃ��A�m�[�g�I���A�m�[�g�I�t�A�s�b�`�x���h�ȂǁA���o�Ă��鉹
	�i�A�T�C������Ă���{�C�X�j�ɑ����ɔ��f������ɂ́APWMDACSynth �̂ق���
	�p�ӂ��Ă��郁���o�֐����Ăяo���A�`�����l���ƃ{�C�X�𓯎��ɍX�V����K�v������܂��B


��MIDI�֐�

	PWMDACSynth �Œ�`���ꂽ�������� static �I�u�W�F�N�g�́A
	MIDI���C�u������ MIDI.setHandleXxxx() �ɒ��ڎw��ł���`���ɂȂ��Ă��܂��B

	�������A�ڑ������MIDI�f�o�C�X�ɂ���ẮANOTE OFF �̑����
	velocity=0 �� NOTE ON �𑗂��Ă��邱�Ƃ�����̂ŁA���̏ꍇ����
	NOTE OFF ���Ăяo���Ƃ������H�v���K�v�ɂȂ�܂��B

	MIDI�`�����l���ԍ��́AMIDI���C�u�����ɍ��킹�� 1�`16 �͈̔͂�
	�w�肷��悤�ɍ���Ă���܂��B


�����F�ύX

	�g�`�e�[�u����ADSR�G���x���[�v�p�����[�^���A���ꂼ�� MidiChannel �N���X��
	wavetable �� envelope[] �Ɏw�肷�邱�ƂŁA���F��ύX�ł��܂��B

	�܂��AInstrument �\���̒萔���v���O�����������̈�ɍ��A�����
	programChange() �Ɏw�肷�邾���ŁA�g�`�ƃG���x���[�v����x�ɐݒ�ł��܂��B
	MIDI�̃v���O�����`�F���W����������Ƃ��ɕ֗��ł��B

	�y�g�`�z wavetable
	�g�`�e�[�u���́A�v�f��256��byte�^PROGMEM�z��Ƃ��Đ������܂��B
	����� wavetable �Ɏw�肷�邱�ƂŁA�g�`��؂�ւ��邱�Ƃ��ł��܂��B
	�g�`�֐��̃}�N���� PWMDAC_CREATE_WAVETABLE() �}�N���Ɏw�肷�邩�A
	�܂��͂���ɂȂ���Ď����Ŕz����`���邱�ƂŁA�C�ӂ̔g�`�𐶐����邱�Ƃ��\�ł��B

	�� �g���́i255�������������F�f�t�H���g6�d�a���̂Ƃ�42�j�𒴂��Ȃ��悤�ɂ��Ă��������B
	���������������ς��ɖ炵���Ƃ��ɍő剹��255�𒴂��ăI�[�o�[�t���[���A�m�C�Y��
	�������鋰�ꂪ����܂��B

	�yADSR�G���x���[�v�z envelope[]
	  ADSR_ATTACK_VALUE - �A�^�b�N���ԁi0�`15�A�傫���قǃm�[�g�I������̗����オ�肪�������j
	  ADSR_DECAY_VALUE - �f�B�P�C���ԁi0�`15�A�傫���قǃm�[�g�I����̌������������j
	  ADSR_SUSTAIN_VALUE - �T�X�e�B�����x���i0�`255�A�������~�܂������ƈێ����鉹�ʁj
	  ADSR_RELEASE_VALUE - �����[�X���ԁi0�`15�A�傫���قǃm�[�g�I�t��̌������������j
	�����Ԃ� loop() ���� update() ���Ăяo���p�x�ɂ���ĕς��܂��B


���`�����l���D��x�w��

	���̋@�\�́A�����MIDI�`�����l���i��F�����f�B�p�[�g�j�̉�������MIDI�`�����l���̉���
	����������錻�ۂ��ڗ��ꍇ�ɁA����MIDI�`�����l���ɑ΂���{�C�X�A�T�C���̗D��x��
	�������邽�߂̋@�\�ł��B

	���̋@�\���g���ɂ� PWMDAC_CHANNEL_PRIORITY_SUPPORT �� #define ����K�v������܂��B

		#define PWMDAC_CHANNEL_PRIORITY_SUPPORT

		PWMDACSynth::getChannel(�`�����l���ԍ�)->setPriority(�`�����l���D��x);

	�`�����l���D��x�ɂ� 0x00�i0�A�Œ�F�f�t�H���g�j�` 0xFF�i255�A�ō��j���w��ł��܂��B

	��{�I�ȍl�����Ƃ��āAADSR�G���x���[�v�ŕϓ��������ʂ̍ł��������{�C�X���珇�ɁA
	����MIDI�`�����l���ɏ��邱�Ƃ������Ƃ��Ă��܂��B

	���̂����ŁA�`�����l���D��x�̃r�b�g���]�l�i1�̕␔�j���u����臒l�v�Ƃ��A
	����𒴂��鉹�ʂ̃{�C�X���A���̃`�����l�������D��I�ɐ�L��������悤�ɂ��Ă��܂��B

	�D��x�����܂荂����������ƁA�\�����������ʂɂ�������炸�{�C�X���L�������Ă��܂��A
	���d�a�����������邱�Ƃ�����܂��̂ŁA�K�X�������Ă��������B



��ҁF�����悵 - Akiyoshi Kamide
	http://www.yk.rim.or.jp/~kamide/

