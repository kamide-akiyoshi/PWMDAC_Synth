
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20150919

Arduinoで動作する簡易シンセサイザライブラリです。

https://osdn.jp/users/kamide/pf/PWMDAC_Synth/wiki/FrontPage

これは CAmiDion

  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/

の２号機以降で実装した音源をライブラリ化したものです。


●Arduinoで楽器を作ろうとして、こんな問題にぶち当たったことはありませんか？

・音を出すなら tone() が手っ取り早いけど、和音が出ない…

・analogWrite() で PWM 出力しても、パルス波の周波数が490Hzと
  低すぎるから、1kHz前後の音なんて乗せられない、どうしよう…

そんな問題も、AVRのプリスケーラ（分周器）の設定を変えて
パルス波の周波数を最大にすれば一気に解決。

このライブラリは、規則正しい最速の周期で発生させたタイマー割り込みで、
その瞬間の波の高さに応じ、PWMのパルス幅を毎回リアルタイムに
素早く更新することにより、矩形波だけでなく色々な波形の音を出力します。

外部音源チップなしで Arduino 本体そのものに
シンセサイザの音源を実装したいときに活用してください。


●インストール

展開した PWMDAC_Synth フォルダを下記のフォルダに
置くだけでインストールできます。

	マイドキュメント\Arduino\libraries\

置いた後、Arduino IDE のメニュー [ファイル] → [スケッチの例] に
PWMDAC_Synth が現れることを確認してください。


●使い方

	// 必要に応じ、PWMDAC_Synth.h をインクルードする前に #define で下記を指定できます。
	#define PWMDAC_OUTPUT_PIN  3 // PWM出力ピン番号（省略可：下記参照）
	#define PWMDAC_POLYPHONY   6 // 同時発音数（省略可：下記参照）
	#define PWMDAC_NOTE_A_FREQUENCY 440  // A音のチューニング周波数（省略可：デフォルト440Hz）

	#include <PWMDAC_Synth.h>
	PWMDAC_INSTANCE; // インスタンス生成（必須）

	setup() {
		PWMDACSynth::setup(); // 初期化（必須）
	}

	loop() {
		PWMDACSynth::update(); // 状態更新（必須）
	}


PWMDACSynth::update() は、減衰などのADSRエンベロープ形状の
現在位置を一つ進めたり、モジュレーションによって変化する時間ごとの
周波数を更新するための関数です。呼び出し頻度は自分でカウンタを
用意するなどの方法でコントロールしてください。

付属のサンプルスケッチ（examples/*.ino）に実装例があります。
電子楽器 CAmiDion など、実際に PWMDAC_Synth を使っているスケッチも
参考にしてください。

その他、使える関数については PWMDAC_Synth.h を参照してください。


●同時発音数

	デフォルトは6重和音です。
	PWMDAC_POLYPHONY に数値を設定してコンパイルし直すことで変更可能です。

	あまり大きすぎると処理が追いつかず動作しない場合があります。
	7重和音ぐらいまでが限界のようです。


●出力ピン(PWM)

	PWM音声出力用として指定できるピン番号は 3,9,10,11 のいずれかです。
	デフォルトのピン番号は 3 です。

	3,11 を選択した場合は TIMER2、9,10 を選択した場合は TIMER1 が使われます。

	なお、5,6 は PWM 端子であっても指定できません。
	これは TIMER0 用の ISR() が Arduino の millis() などのために使われていて
	再定義できなかったためです。

	PWM 出力ピンは、そのまま PC の LINE IN などにつないでも一応音は聞こえますが、
	ローパスフィルタで可聴周波数だけを通すようにすると、より聞きやすくなります。


●MIDIチャンネル操作

	PWM_SYNTH のメソッドで、

		MIDIチャンネル番号(1～16) ⇔ MIDIチャンネルへのポインタ

	を相互変換できます。

	MidiChannel *getChannel(char channel)
	char getChannel(MidiChannel *cp)

	MIDIチャンネルへのポインタは、主に波形やエンベロープパラメータと
	いった、次回以降の音出し以降に反映するパラメータの設定に使います。

	NOTE OFF、NOTE ON、ピッチベンドは、リアルタイム性が要求されるので
	PWMDACSynth から呼び出してください。

	特にピッチベンドは、MidiChannel::setPitchBend() で設定しただけでは
	今出ている音にリアルタイムに反映されません。
	PWMDACSynth::pitchBend() を使えばリアルタイムに反映されます。


●MIDI関数

	PWM_SYNTH 静的オブジェクトには、MIDIライブラリの MIDI.setHandleXxxx() に
	直接指定できるよう、引数の順序や型を合わせた関数をいくつか用意しています。

	ただし、接続相手のMIDIデバイスによっては、NOTE OFF の代わりに
	velocity=0 の NOTE ON を送ってくることがあるので、その場合だけ
	NOTE OFF を呼び出すといった工夫が必要になります。

	MIDIチャンネル番号は、MIDIライブラリに合わせて 1～16 の範囲で
	指定するように作ってあります。


●音色変更
	プログラムチェンジには対応していません。
	代わりににエンベロープパラメータと波形を指定して音色を変更します。
	（MidiChannel クラスの wavetable や env_param で指定します）

	エンベロープパラメータは EnvelopeParam 構造体を介して行います。

	・attack_speed - アタック速度（小さいほどアタックタイムがゆっくり）
	・decay_time - ディケイ時間（大きいほどノートオン後の減衰がゆっくり）
	・sustain_level - サスティンレベル（減衰が止まったあと維持する音量）
	・release_time - リリース時間（大きいほどノートオフ後の減衰がゆっくり）

	なお、これらの時間は loop() 内で update() を呼び出す頻度によって変わります。

	波形は、PROGMEM 配列（要素数256のbyte型に限る）を指定して行います。

	和音によって音が重なったとき、レベルが 255 を超えると
	音割れの原因になります。同時発音数（デフォルト：６重和音）で
	割った小さめの値で波形を作るようにしてください。
	なお、PWMDACSynth:: にも組み込みの波形配列があるので
	これを指定すると簡単です。

	・sineWavetable[]	正弦波
	・squareWavetable[]	矩形波
	・triangleWavetable[]	三角波
	・sawtoothWavetable[]	のこぎり波
	・shepardToneSineWavetable[]	正弦波の無限音階（シェパードトーン）

●ユーティリティ
	byte musicalMod7(char x)
	byte musicalMod12(char x)
		それぞれ7で割った余り、12で割った余りを返します。
		% 演算子と違い、負数を与えても正数で返します。
		内部的にはビット演算と加算だけで高速に計算します。
		音階の計算に便利です。

	byte log2(unsigned int x)
		２を底とする対数を、小数点以下を切捨てた整数で返します。
		y = log2(x) と x = 1 << y は逆関数の関係になります。

	int musicalConstrain12(int note, int min_note, int max_note)
		Arduino の constrain() を音階用に実装し直した関数。
		note が min_note ～ max_note の音域にあれば
		note をそのまま返します。音域をはみ出していた場合、
		音階を変えずにオクターブ位置を変えることにより、
		範囲に収まるよう調整されます。


作者：＠きよし - Akiyoshi Kamide
	http://www.yk.rim.or.jp/~kamide/
