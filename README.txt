
[PWMDAC_Synth - PWM DAC synthesizer library for Arduino]

ver.20160611

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
	//
	#define PWMDAC_OUTPUT_PIN  3 // PWM出力ピン番号（省略可：下記参照）
	#define PWMDAC_POLYPHONY   6 // 同時発音数（省略可：下記参照）
	#define PWMDAC_NOTE_A_FREQUENCY 440  // A音のチューニング周波数（省略可：デフォルト440Hz）

	#include <PWMDAC_Synth.h>
	//
	// 波形テーブルを、必要な分だけ定義します。
	// （不要な波形を定義しないようにすることで、プログラムメモリ領域の節約になります）
	//
	PWMDAC_CREATE_WAVETABLE(squareWavetable, PWMDAC_SQUARE_WAVE);      // 矩形波
	PWMDAC_CREATE_WAVETABLE(triangleWavetable, PWMDAC_TRIANGLE_WAVE);  // 三角波
	PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);          // 正弦波
	PWMDAC_CREATE_WAVETABLE(shepardToneSineWavetable, PWMDAC_SHEPARD_TONE);  // シェパードトーン（無限音階）
	PWMDAC_CREATE_WAVETABLE(sawtoothWavetable, PWMDAC_SAWTOOTH_WAVE);  // のこぎり波
	//
	// 波形とエンベロープパラメータ（ADSR）を Instrument 構造体に束ねて
	// プログラムメモリ領域に音色データを生成します。
	// 内側の {} にはADSRの値を逆順で指定します（値の範囲については後述の「●音色変更」を参照）。
	//
	PROGMEM const Instrument instrument = {sawtoothWavetable, {9, 0, 11, 4}};
	//
	// 生成した音色を指定してPWMDAC_Synthの実体（インスタンス）を生成します。
	// MIDIチャンネル1～16がすべてこの音色に初期化されます。
	//
	PWMDAC_CREATE_INSTANCE(&instrument);

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

	PWMDAC_POLYPHONY に数値を設定してコンパイルし直すことで、
	同時発音数を増減させることができます。

	同時発音数を増やしすぎると、その分、割り込み処理に時間がかかって
	割り込み以外の処理を行う余裕がなくなり、正常に動作しなくなります。


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

	MIDIチャンネルへのポインタを介してチャンネル単位に持たせる
	パラメータを操作できます。

	ただし、ノートオン、ノートオフ、ピッチベンドなどを今出ている音に
	即座に反映させるためには、MidiChannel ではなく PWMDACSynth の
	メンバ関数を呼び出す必要があります。


●MIDI関数

	PWM_SYNTH 静的オブジェクトには、MIDIライブラリの MIDI.setHandleXxxx() に
	直接指定できるよう、引数の順序や型を合わせた関数をいくつか用意しています。

	ただし、接続相手のMIDIデバイスによっては、NOTE OFF の代わりに
	velocity=0 の NOTE ON を送ってくることがあるので、その場合だけ
	NOTE OFF を呼び出すといった工夫が必要になります。

	MIDIチャンネル番号は、MIDIライブラリに合わせて 1～16 の範囲で
	指定するように作ってあります。


●音色変更

	波形とエンベロープパラメータ（ADSR）を MidiChannel クラスの
	wavetable と env_param に指定することで、音色を変更できます。

	ADSRの設定は EnvelopeParam クラスを介して行います。
	コンストラクタで次の値を指定して初期化できます。

	・attack_time - アタック時間（大きいほどノートオン直後の立ち上がりがゆっくり）
	・decay_time - ディケイ時間（大きいほどノートオン後の減衰がゆっくり）
	・sustain_level - サスティンレベル（減衰が止まったあと維持する音量）
	・release_time - リリース時間（大きいほどノートオフ後の減衰がゆっくり）

	値の範囲は sustain_level が 0～255、それ以外は 0～15 です。
	実時間は loop() 内で update() を呼び出す頻度によって変わります。

	各ADSRパラメータ値へのポインタは getParam() メソッドで取得できます。

	波形テーブルは、要素数256のbyte型PROGMEM配列として生成します。
	それを wavetable に指定することで、波形を切り替えることができます。

	波形テーブル配列の生成は、PWMDAC_Synth.h 上で定義された
	マクロを使い、必要な波形テーブルだけを生成してください。

	このマクロは、同時発音数が増えても信号レベルが255を超えて音割れせず、
	かつ最大音量の出るような計算式になっています。
	これに習って自分で波形を定義すれば、それを指定して
	独自の音色を生み出すことも可能です。

	プログラムメモリ領域に Instrument 構造体定数を作って、
	それを指定することで音色を変更することもできます。
	MIDIのプログラムチェンジの実装にはこの方法がおすすめです。
	現在のバージョンでは、プログラムチェンジ対応に伴い、
	エンベロープの初期化に Instrument 構造体定数を使うインターフェースに
	してRAMへの展開を最小限に抑えられるようにしました。


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
