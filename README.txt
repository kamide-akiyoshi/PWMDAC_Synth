
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20170514

https://osdn.jp/users/kamide/pf/PWMDAC_Synth/wiki/FrontPage

Arduino�œ��삷��ȈՃV���Z�T�C�U���C�u�����ł��B

����́ACAmiDion

  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/

�̂Q���@�ȍ~�Ŏ����������������C�u�������������̂ł��B


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

	PWMDAC_POLYPHONY �ɐ��l��ݒ肵�ăR���p�C�����������ƂŁA
	�����������𑝌������邱�Ƃ��ł��܂��B

	�����������𑝂₵������ƁA���̕��A���荞�ݏ����Ɏ��Ԃ���������
	���荞�݈ȊO�̏������s���]�T���Ȃ��Ȃ�A����ɓ��삵�Ȃ��Ȃ�܂��B

	������������1�̔����́u�{�C�X�v�Ƃ����P�ʂŊǗ�����A�{�C�X�̏�Ԃ�
	�ێ�����N���X�Ƃ��� VoiceStatus �N���X���`���Ă��܂��B
	VoiceStatus �N���X�̔z��̗v�f�����A���̂܂ܓ����������ƂȂ�܂��B

	�����MIDI�`�����l���ɂ��ă{�C�X�A�T�C���̗D��x���グ����@�ɂ��ẮA
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

	PWM_SYNTH �̃��\�b�h�ŁA

		MIDI�`�����l���ԍ�(1�`16) �� MIDI�`�����l���ւ̃|�C���^

	�𑊌ݕϊ��ł��܂��B

	MidiChannel *getChannel(char channel)
	char getChannel(MidiChannel *cp)

	MIDI�`�����l���ւ̃|�C���^����ă`�����l���P�ʂɎ�������
	�p�����[�^�𑀍�ł��܂��B

	�Ȃ��A�m�[�g�I���A�m�[�g�I�t�A�s�b�`�x���h�ȂǁA���o�Ă��鉹
	�i�A�T�C������Ă���{�C�X�j�ɑ����ɔ��f������ɂ́A
	MidiChannel �ł͂Ȃ� PWMDACSynth �̃����o�֐����Ăяo���K�v������܂��B


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


�����̑��̊֐�

	���̑��A���p�ł���֐��ɂ��Ă̓\�[�X�R�[�h�̃w�b�_ PWMDAC_Synth.h ���Q�ƁB


��ҁF�����悵 - Akiyoshi Kamide
	http://www.yk.rim.or.jp/~kamide/
