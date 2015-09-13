
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20150913

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

�E�Ăяo�����ł́A�Œ���A���L��2�_����������K�v������܂��B

	�Esetup() �̒��� PWM_SYNTH.setup() ���g���ď��������܂��B

	�Eloop() �̒��� PWM_SYNTH.updateEnvelopeStatus() �����I�ɌĂяo���܂��B
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

	�Ȃ��A�����̎��Ԃ� loop() ���� updateEnvelopeStatus() ���Ăяo��
	�p�x�ɂ���ĕς��܂��B

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

���X�V����

	ver.20150913
		�E�s�b�`�x���h�E�Z���V�e�B�r�e�B�ɑΉ�
		�E���̑��\�[�X�̐����E���t�@�N�^�����O

	var.20130327
		�E�V����voice�����蓖�Ă�ۂɔ�������voice������肳���
			���܂����Ƃ�����s����C��

		�E���[�e�B���e�B�֐� musicalConstrain12() ��ǉ�

	var.20130113
		�E���W�����[�V�����̂�炬������
			�ϐ� PWM_SYNTH.modulation_dphase �Œ����ł���悤�ɂ��܂����B
		�E�����[�X��ɉ��i�]�C�j�����Ă���Ƃ��ɃA�^�b�N�������Ă�
			���F���ς��Ȃ��s��̏C��

	var.20130112
		�E�s�b�`�x���h�������l����O��Ă���Ƃ��� NOTE ON �����
			���ۂ̃s�b�`�������l�ɖ߂���Ă��܂��s��̏C��
		�E���[�e�B���e�B�֐� log2() ��ǉ�
		�EMidiChannel�N���X�̊O�o���A�֐��� .h �ɏo���ăC�����C�����A�Ȃ�

	var.20121110
		�E�m�[�g�ԍ��O�i�ŏ��F���̂������Ⴂ 8Hz ���炢�̉��K�j��
			�^�����Ƃ��h�ł͂Ȃ��~�̉��ɂȂ��Ă��܂��o�O���C��

		�E���[�e�B���e�B�֐� musicalMod7() ��ǉ�

	var.20121108
		�E���[�e�B���e�B�֐� musicalMod12() ��ǉ����A�����I�ɂ�
			������g���悤�ɂ��܂����B

	ver.20121029
		�E���������̃`���[�j���O�i�s�v�ȏ������̏ȗ��Ȃǁj

	ver.20121013
		�E�o�� PWM �[�q�̕ύX���A�Ăяo�����X�P�b�`�łł���悤�ɂ��܂����B
			�i����܂ł̓��C�u���������������Ȃ��Ƃł��܂���ł����j
		�E�������[�`���̐���

	ver.20120805
		�EgetEnvelope()�AgetWave() ��ǉ�

	ver.20120719
		�E�o�� PWM �[�q���S�̒�����I���ł���悤�ɂ��܂���
			�i���F3 �̂� �� �V�F3,9,10,11 ����I���j�B

	ver.20120716
		�E�����g�A��`�g�A�O�p�g�A�̂�����g�̔g�`�e�[�u����
			���C�u�����ɑg�ݍ��݂܂����B

	ver.20120611
		�E�g�`�̎w����@��ύX���܂����B
			�g�`�e�[�u����RAM���g�킸�S�ʓI��PROGMEM���i�t���b�V���������ֈړ��j
			�������Ƃɔ����A�g�`�e�[�u�����֐��Ŏw�肷��C���^�[�t�F�[�X��p�~���A
			�����PROGMEM�z��̔g�`�e�[�u���i�Ăяo�����ŏ������K�v�j���w�肷��
			�C���^�[�t�F�[�X�ɂ��܂����B
			����ɂ��RAM�̈悪�팸���ꂽ�ق��AMIDI�`�����l�����Ƃ�
			�g�`���Ⴄ���̂ɂ��邱�Ƃ��\�ɂȂ�܂����B

	ver.20120609
		�E���g���v�Z�̌�����
			Phase correct�i���ʑ���jPWM �ł̓^�C�}�[�J�E���^(TCNT2)��
			�O�p�g��`���悤�ɕς��܂����A�㉺�̒��_�����P���Ȃ��̂�
			���ۂ�512�N���b�N�ł͂Ȃ�510�N���b�N���ƂɊ��荞�݂�����
			���邱�Ƃ��������܂����B����𓥂܂��Ċ�l�𒲐����܂����B

	ver.20120607
		�E���荞�ݏ����̍������B
			PWM���荞�݂�ISR()����Ă΂�� nextPulseWidth() �֐�����
			�W�r�b�g�V�t�g�̉��Z�����[�v�O�֒ǂ��o�����B

		�ETIMER0���~�߂鏈�����R�����g�A�E�g

	ver.20120321
		�E�g�`�ύX��setWave()�֐��Ɉ�{��

	ver.20120318(������)


��ҁF�����悵 - Akiyoshi Kamide
	http://www.yk.rim.or.jp/~kamide/
