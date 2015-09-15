
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20150915

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

���o�̓s��(PWM)

�iPWMDAC_Synth.h �Q�Ɓj

PWMDAC_Synth.h ���C���N���[�h����O�ɁAPWM �o�͂��� Arduino �s���ԍ���
#define PWMDAC_OUTPUT_PIN �Œ�`���Ă��������B

��F
	#define PWMDAC_OUTPUT_PIN 3
	#include <PWMDAC_Synth.h>

�w��ł���s���ԍ��� 3,9,10,11 �̂����ꂩ�ł��B

3,11 ��I�������ꍇ�� TIMER2�A9,10 ��I�������ꍇ�� TIMER1 ���g���܂��B

�Ȃ��A5,6 �� PWM �[�q�ł����Ă��w��ł��܂���B
����� TIMER0 �p�� ISR() �� Arduino �� millis() �Ȃǂ̂��߂Ɏg���Ă���
�Ē�`�ł��Ȃ��������߂ł��B

PWM �o�̓s���́A���̂܂� PC �� LINE IN �ȂǂɂȂ��ł��ꉞ���͕������܂����A
���[�p�X�t�B���^�ŉ����g��������ʂ��悤�ɂ���ƁA��蕷���₷���Ȃ�܂��B


���g����

�E�Ăяo�����ł́A�Œ���A���L��3�_����������K�v������܂��B

	�EPWMDAC_CREATE_DEFAULT_INSTANCE() ���Ăяo���A
		PWM_SYNTH �Ƃ����ϐ��ɃC���X�^���X�𐶐����܂��B

	�Esetup() �̒��� PWM_SYNTH.setup() ���g���ď��������܂��B

	�Eloop() �̒��� PWM_SYNTH.update() �����I�ɌĂяo���܂��B
		����͌����Ȃǂ�ADSR�G���x���[�v�`��̌��݈ʒu����i�߂���A
		���W�����[�V�����ɂ���ĕω����鎞�Ԃ��Ƃ̎��g�����X�V���邽�߂̊֐��ł��B
		�Ăяo���p�x�͎����ŃJ�E���^��p�ӂ���Ȃǂ̕��@�ŃR���g���[�����Ă��������B

	�t���̃T���v���X�P�b�`�iexamples/*.ino�j�Ɏ����Ⴊ����܂��B

	�d�q�y�� CAmiDion �ȂǁA���ۂ� PWMDAC_Synth ���g���Ă���X�P�b�`��
	�Q�l�ɂ��Ă��������B

	�g����֐��ɂ��Ă� PWMDAC_Synth.h ���Q�Ƃ��Ă��������B


�EMIDI�`�����l������

	PWM_SYNTH �̃��\�b�h�ŁA

		MIDI�`�����l���ԍ�(1�`16) �� MIDI�`�����l���ւ̃|�C���^

	�𑊌ݕϊ��ł��܂��B

	MidiChannel *getChannel(char channel)
	char getChannel(MidiChannel *cp)

	MIDI�`�����l���ւ̃|�C���^�́A��ɔg�`��G���x���[�v�p�����[�^��
	�������A����ȍ~�̉��o���ȍ~�ɔ��f����p�����[�^�̐ݒ�Ɏg���܂��B

	NOTE OFF�ANOTE ON�A�s�b�`�x���h�́A���o�Ă��鉹�Ƀ��A���^�C����
	���f�����悤�APWM_SYNTH �̃��\�b�h���炵������ł��Ȃ��悤��
	�Ȃ��Ă��܂��B


�EMIDI�֐�

	PWM_SYNTH �ÓI�I�u�W�F�N�g�ɂ́AMIDI���C�u������ MIDI.setHandleXxxx() ��
	���ڎw��ł���悤�A�����̏�����^�����킹���֐����������p�ӂ��Ă��܂��B

	�������A�ڑ������MIDI�f�o�C�X�ɂ���ẮANOTE OFF �̑����
	velocity=0 �� NOTE ON �𑗂��Ă��邱�Ƃ�����̂ŁA���̏ꍇ����
	NOTE OFF ���Ăяo���Ƃ������H�v���K�v�ɂȂ�܂��B

	MIDI�`�����l���ԍ��́AMIDI���C�u�����ɍ��킹�� 1�`16 �͈̔͂�
	�w�肷��悤�ɍ���Ă���܂��B


�E���F�ύX
	�v���O�����`�F���W�ɂ͑Ή����Ă��܂���B
	����ɂɃG���x���[�v�p�����[�^�Ɣg�`���w�肵�ĉ��F��ύX���܂��B
	�iMidiChannel �N���X�� wavetable �� env_param �Ŏw�肵�܂��j

	�G���x���[�v�p�����[�^�� EnvelopeParam �\���̂���čs���܂��B

	�Eattack_speed - �A�^�b�N���x�i�������قǃA�^�b�N�^�C�����������j
	�Edecay_time - �f�B�P�C���ԁi�傫���قǃm�[�g�I����̌������������j
	�Esustain_level - �T�X�e�B�����x���i�������~�܂������ƈێ����鉹�ʁj
	�Erelease_time - �����[�X���ԁi�傫���قǃm�[�g�I�t��̌������������j

	�Ȃ��A�����̎��Ԃ� loop() ���� update() ���Ăяo���p�x�ɂ���ĕς��܂��B

	�g�`�́APROGMEM �z��i�v�f��256��byte�^�Ɍ���j���w�肵�čs���܂��B

	�a���ɂ���ĉ����d�Ȃ����Ƃ��A���x���� 255 �𒴂����
	������̌����ɂȂ�܂��B�����������i�f�t�H���g�F�U�d�a���j��
	�����������߂̒l�Ŕg�`�����悤�ɂ��Ă��������B
	�Ȃ��APWM_SYNTH �ɂ��g�ݍ��݂̔g�`�z�񂪂���̂�
	������w�肷��ƊȒP�ł��B

	�EsineWavetable[]	�����g
	�EsquareWavetable[]	��`�g
	�EtriangleWavetable[]	�O�p�g
	�EsawtoothWavetable[]	�̂�����g
	�EshepardToneSineWavetable[]	�����g�̖������K�i�V�F�p�[�h�g�[���j

�E���[�e�B���e�B
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
