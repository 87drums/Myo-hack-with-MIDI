#define _USE_MATH_DEFINES

#include <stdio.h>

#include <windows.h> 
#include <mmsystem.h>

#include <iostream>
#include <thread>
#include <exception>
#include <queue>

#include<string>

#include <cmath>
#include <iomanip>
#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <time.h>
#include <conio.h>

#include <myo/myo.hpp>

using namespace std;
#define USAGE_ON_RUNNING	"\nEsc キーで終了\nTIME STAMP  St Dt Dt : [  ch] Messages\n     0.000  -- -- -- : [--ch] ----\n"
//#define VK_ENTER	13	// Enter
#define VK_ESC		27	// Esc

#pragma comment(lib, "winmm.lib")

//MIDI filename
char *file_name = "data/test_MIDI.dat";
ofstream fout;

bool flag = false; //flag that is to start reading of emg

//タイムスタンプ取得関数
/*//タイムスタンプ取得部
long long now = unix_timestamp();
std::cout << now;
*/
long long unix_timestamp() {
	long long unix_epoch_from_1601_int64 = 116444736000000000LL;

	//現在時刻をFILETIME構造体に格納
	FILETIME current_ft_from_1601;
	GetSystemTimeAsFileTime(&current_ft_from_1601);

	//現在時刻を格納したFILETIME構造体を64ビット整数型に変換
	ULARGE_INTEGER current_from_1601;
	current_from_1601.HighPart = current_ft_from_1601.dwHighDateTime;
	current_from_1601.LowPart = current_ft_from_1601.dwLowDateTime;

	//現在時刻ファイル時間からUnixエポックファイル時間を引き算
	//→Unixエポックからの経過時間(100ナノ秒単位)へ変換
	long long diff = current_from_1601.QuadPart - unix_epoch_from_1601_int64;

	//10で割って経過時間(100ナノ秒単位)を経過時間(マイクロ秒単位)へ変換
	long long t = diff / 10;

	return t;
}

// MIDI 受信関連の関数
/*--------------------------------------------------------------
MIDI IN コールバック関数

midiInOpen() で指定したプロシージャ。OS がMIDI IN関連のメ
ッセージを発行した際に自動的に呼び出されます。
プロシージャに処理を直接書くとごちゃごちゃするので、普通は
メッセージごとに処理を関数に分けて書きます。今回は MIM_DATA
を受信した場合にのみ処理を切り分けるようにしました。
--------------------------------------------------------------*/
/*--------------------------------------------------------------
MIDI データのショート メッセージ受信

ショート メッセージを受信した際に呼ばれる関数です。受信メ
ッセージ dwData を解析して表示します。
dwData の下位バイトから順にステータス バイト、データ バイ
ト 1 、データ バイト 2 、空バイトとします。
ステータス バイトの上位 4bits が 1111 であればシステムメッ
セージ、1000～1110 であればチャネル メッセージです。また下
位 4bits はシステム メッセージならその種類を、チャネルメッ
セージの場合は発行チャネルを表しています。
詳しくは規格の解説書などを参照して下さい。
--------------------------------------------------------------*/
#define SKIP_TIMING_CLOCK	1	// メッセージ 0xF8=timing clock は無視する
void OnMimData(DWORD dwData, DWORD dwTimeStamp)
{
	unsigned char st, dt1, dt2;	// 受信データ格納バッファ

								///////	受信データを分解
	st = dwData & 0xFF;	// ステータス バイト
	dt1 = dwData >> 8 & 0xFF;	// データ バイト１
	dt2 = dwData >> 16 & 0xFF;	// データ バイト２

#if SKIP_TIMING_CLOCK
	if (st == 0xF8)
		return;
#endif

	///////	受信データを表示
	if ((int)dt2 > 0) {
		long long t_stamp = std::time(0);//unix_timestamp();

		printf("%10.3f  %02X %02X %02X : ", t_stamp, st, dt1, dt2);//printf("%10.3f  %02X %02X %02X : ", (double)dwTimeStamp / 1000, st, dt1, dt2);

																   //write midi file
		fout.open(file_name, ios::app);
		fout << t_stamp << "\t" << (long)dt1 << "\t" << (long)dt2 << endl;//fout << (double)dwTimeStamp << "\t" << (long)dt1 << "\t" << (long)dt2 << endl;
		fout.close();

		///////	受信データを分析
		if (st >> 4 == 0xF) {
			///////	システム メッセージ
			switch (st & 0xF) {
			case 0x1:
				printf("MTC quarter frame (Type:%d Val:2d)", dt1 >> 4 & 0xF, dt1 & 0xF);
				break;
			case 0x2:
				printf("song position pointer (Beat:%5d)", dt1 + dt2 << 7);
				break;
			case 0x3:
				printf("song select (Num:%3d)", dt1);
				break;
			case 0x6:
				printf("tune request");
				break;
			case 0x7:
				printf("end of exclusive");
				break;
			case 0x8:
				printf("timing clock");
				break;
			case 0xA:
				printf("start");
				break;
			case 0xB:
				printf("continue");
				break;
			case 0xC:
				printf("stop");
				break;
			case 0xE:
				printf("active sensing");
				break;
			case 0xF:
				printf("system reset");
				break;
			default:
				printf("(unknown system mes)");
				break;
			}
		}
		else {
			/////// チャネル メッセージ
			printf("[%2dch] ", (st & 0xF) + 1);
			static const char keys[] = "CDEFGAB";	// 音階
			switch (st >> 4) {
			case 0x8:
				printf("note off* (Note:%c%-2d Vel:%3d)", keys[dt1 % 7], dt1 / 7 - 3, dt2);
				break;
			case 0x9:
				printf("note %s  (Note:%c%-2d Vel:%3d)", (dt2 ? "on " : "off"), keys[dt1 % 7], dt1 / 7 - 3, dt2);
				break;
			case 0xA:
				printf("polyphonic key pressure (Note:%c%-2d Prss:%3d)", keys[dt1 % 7], dt1 / 7 - 3, dt2);
				break;
			case 0xB:
				printf("control change (Num:%3d Val:%3d)", dt1, dt2);
				break;
			case 0xC:
				printf("program change (Num:%3d)", dt1);
				break;
			case 0xD:
				printf("channel pressure (Prss:%3d)", dt1);
				break;
			case 0xE:
				printf("pitch bend change (Val:%5d)", dt1 + dt2 << 7);
				break;
			default:
				printf("(unknown channel message)");
				break;
			}
		}

		printf("\n");
	}

	return;
}

void CALLBACK MidiInProc(
	HMIDIIN hMidiIn,
	UINT wMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
) {
	switch (wMsg) {
	case MIM_OPEN:		// 入力デバイス オープン コールバック
		printf("*** MIM_OPEN ***\n");
		break;
	case MIM_CLOSE:		// 入力デバイスをクローズ
		printf("*** MIM_CLOSE ***\n");
		break;
	case MIM_DATA:		// 入力デバイス データ コールバック
		OnMimData(dwParam1, dwParam2);
		break;
	case MIM_LONGDATA:	// 入力バッファのコールバック
		printf("*** MIM_LONGDATA ***\n");
		break;
	case MIM_ERROR:		// 入力デバイス エラー コールバック
		printf("*** MIM_ERROR ***\n");
		break;
	case MIM_LONGERROR:	// 無効なシステム エクスクルーシブ メッセージに対するコールバック
		printf("*** MIM_LONGERROR ***\n");
		break;
	case MIM_MOREDATA:	// ???
		printf("*** MIM_MOREDATA ***\n");
		break;
	default:
		printf("*** (unknown message) ***\n");
		break;
	}
	return;
}


class DataCollector : public myo::DeviceListener {
public:
	DataCollector()
	{
		openFiles();
	}

	void openFiles() {
		time_t timestamp = std::time(0);

		// Open file for EMG log
		if (emgFile.is_open()) {
			emgFile.close();
		}
		std::ostringstream emgFileString;
		emgFileString << "data/emg.csv";//emgFileString << "data/emg-" << timestamp << ".csv";
		emgFile.open(emgFileString.str(), std::ios::out);
		emgFile << "timestamp,emg1,emg2,emg3,emg4,emg5,emg6,emg7,emg8" << std::endl;

		// Open file for gyroscope log
		if (gyroFile.is_open()) {
			gyroFile.close();
		}
		std::ostringstream gyroFileString;
		//gyroFileString << "data/gyro-" << timestamp << ".csv";
		gyroFileString << "data/gyro.csv";
		gyroFile.open(gyroFileString.str(), std::ios::out);
		gyroFile << "timestamp,x,y,z" << std::endl;

		// Open file for accelerometer log
		if (accelerometerFile.is_open()) {
			accelerometerFile.close();
		}
		std::ostringstream accelerometerFileString;
		//accelerometerFileString << "data/accelerometer-" << timestamp << ".csv";
		accelerometerFileString << "data/accelerometer.csv";
		accelerometerFile.open(accelerometerFileString.str(), std::ios::out);
		accelerometerFile << "timestamp,x,y,z" << std::endl;

		// Open file for orientation log
		if (orientationFile.is_open()) {
			orientationFile.close();
		}
		std::ostringstream orientationFileString;
		//orientationFileString << "data/orientation-" << timestamp << ".csv";
		orientationFileString << "data/orientation.csv";
		orientationFile.open(orientationFileString.str(), std::ios::out);
		orientationFile << "timestamp,x,y,z,w" << std::endl;

		// Open file for orientation (Euler angles) log
		if (orientationEulerFile.is_open()) {
			orientationEulerFile.close();
		}
		std::ostringstream orientationEulerFileString;
		//orientationEulerFileString << "data/orientationEuler-" << timestamp << ".csv";
		orientationEulerFileString << "data/orientationEuler.csv";
		orientationEulerFile.open(orientationEulerFileString.str(), std::ios::out);
		orientationEulerFile << "timestamp,roll,pitch,yaw" << std::endl;

	}

	// onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
	void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	{
		long long now = timestamp;//unix_timestamp();
		printf("\r");
		emgFile << now;
		for (size_t i = 0; i < 8; i++) {
			emgFile << ',' << static_cast<int>(emg[i]);
			printf("[%d] ", emg[i]);
		}
		emgFile << std::endl;

	}

	// onOrientationData is called whenever new orientation data is provided
	// Be warned: This will not make any distiction between data from other Myo armbands
	void onOrientationData(myo::Myo *myo, uint64_t timestamp, const myo::Quaternion< float > &rotation) {
		orientationFile << timestamp
			<< ',' << rotation.x()
			<< ',' << rotation.y()
			<< ',' << rotation.z()
			<< ',' << rotation.w()
			<< std::endl;

		using std::atan2;
		using std::asin;
		using std::sqrt;
		using std::max;
		using std::min;

		// Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
		float roll = atan2(2.0f * (rotation.w() * rotation.x() + rotation.y() * rotation.z()),
			1.0f - 2.0f * (rotation.x() * rotation.x() + rotation.y() * rotation.y()));
		float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (rotation.w() * rotation.y() - rotation.z() * rotation.x()))));
		float yaw = atan2(2.0f * (rotation.w() * rotation.z() + rotation.x() * rotation.y()),
			1.0f - 2.0f * (rotation.y() * rotation.y() + rotation.z() * rotation.z()));

		orientationEulerFile << timestamp
			<< ',' << roll
			<< ',' << pitch
			<< ',' << yaw
			<< std::endl;
	}

	// onAccelerometerData is called whenever new acceleromenter data is provided
	// Be warned: This will not make any distiction between data from other Myo armbands
	void onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &accel) {

		printVector(accelerometerFile, timestamp, accel);

	}

	// onGyroscopeData is called whenever new gyroscope data is provided
	// Be warned: This will not make any distiction between data from other Myo armbands
	void onGyroscopeData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &gyro) {
		printVector(gyroFile, timestamp, gyro);

	}

	void onConnect(myo::Myo *myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion) {
		//Reneable streaming
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);
		openFiles();
	}

	// Helper to print out accelerometer and gyroscope vectors
	void printVector(std::ofstream &file, uint64_t timestamp, const myo::Vector3< float > &vector) {
		file << timestamp
			<< ',' << vector.x()
			<< ',' << vector.y()
			<< ',' << vector.z()
			<< std::endl;
	}

	// The files we are logging to
	std::ofstream emgFile;
	std::ofstream gyroFile;
	std::ofstream orientationFile;
	std::ofstream orientationEulerFile;
	std::ofstream accelerometerFile;

};

//wave保存　演奏音響データ録音
void do_audioget() {
	//タイムスタンプ変数
	char *ATfile_name = "data/Audio_timestamp.dat";
	ofstream Afout;

	MCIERROR mciError = 0;
	static MCI_OPEN_PARMS mciOpen;
	MCI_WAVE_SET_PARMS mciSet;

	//MCIコマンド　初期化
	mciOpen.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;
	mciOpen.lpstrElementName = TEXT("");
	mciError = mciSendCommand(0, MCI_OPEN,
		MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID |
		MCI_OPEN_ELEMENT, (DWORD)&mciOpen);

	// オープン時のエラー処理
	if (mciError != 0) {
		TCHAR szBuf[256];
		mciGetErrorString(mciError, szBuf, sizeof(szBuf) / sizeof(TCHAR));
		printf(szBuf);
		return;
	}

	// デバイスIDの取得
	MCIDEVICEID mciDeviceId = mciOpen.wDeviceID;

	// 録音のための設定
	mciSet.wFormatTag = WAVE_FORMAT_PCM;
	mciSet.wBitsPerSample = 16;
	mciSet.nSamplesPerSec = 16000;
	mciSet.nChannels = 1;
	mciSet.nBlockAlign = mciSet.nChannels*mciSet.wBitsPerSample / 8;
	mciSet.nAvgBytesPerSec = mciSet.nSamplesPerSec*mciSet.nBlockAlign;

	// 設定の反映
	mciSendCommand(mciDeviceId, MCI_SET, MCI_WAVE_SET_FORMATTAG | MCI_WAVE_SET_CHANNELS |
		MCI_WAVE_SET_SAMPLESPERSEC | MCI_WAVE_SET_AVGBYTESPERSEC | MCI_WAVE_SET_BLOCKALIGN |
		MCI_WAVE_SET_BITSPERSAMPLE, (DWORD_PTR)&mciSet);


	//MCIコマンド　録音開始
	mciSendCommand(mciDeviceId, MCI_RECORD, 0, 0);
	long long start_time = unix_timestamp(); //オーディオ録音開始時刻のタイムスタンプ取得

											 //オーディオ録音処理中，wait
	while (!flag) {
		//wait 0.5sec
		Sleep(500);
	}

	while (flag) {
		//wait 0.5sec
		Sleep(500);
	}

	// 録音終了～デバイスクローズまで
	// MCIへの入力引数の準備
	MCI_SAVE_PARMS mciSave;

	// 録音の停止
	long long end_time = unix_timestamp(); //オーディオ録音終了時刻のタイムスタンプ取得
	mciSendCommand(mciDeviceId, MCI_STOP, MCI_WAIT, 0);

	// 録音したものをセーブ
	mciSave.lpfilename = TEXT("data/test_Audio.wav");
	mciSendCommand(mciDeviceId, MCI_SAVE, MCI_WAIT | MCI_SAVE_FILE, (DWORD_PTR)&mciSave);

	//write midi file
	Afout.open(ATfile_name, ios::app);
	Afout << start_time << "\t" << end_time << endl;
	Afout.close();

	// MCIのクローズ
	mciSendCommand(mciDeviceId, MCI_CLOSE, 0, 0);
}

//myo signal取得部，サンプリングレートは200Hz
void do_myoget() {
	// We catch any exceptions that might occur below -- see the catch statement for more details.
	try {
		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		myo::Hub hub("com.undercoveryeti.myo-data-capture");

		std::cout << "Attempting to find a Myo..." << std::endl;

		// Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
		// immediately.
		// waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
		// if that fails, the function will return a null pointer.
		myo::Myo* myo = hub.waitForMyo(10000);

		// If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
		if (!myo) {
			throw std::runtime_error("Unable to find a Myo!");
		}

		// We've found a Myo.
		std::cout << "Connected to a Myo armband! Logging to the file system. Check the folder this appliation lives in." << std::endl << std::endl;

		// Next we enable EMG streaming on the found Myo.
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);

		// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
		DataCollector collector;

		// Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
		// Hub::run() to send events to all registered device listeners.
		hub.addListener(&collector);

		printf("Myo waiting...");
		while (!flag) {
			//wait 50msec
			Sleep(50);
		}//midi入力されるまで待機

		 // Finally we enter our main loop.
		while (flag) {
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 200 times a second, so we run for 1000/200 milliseconds.
			hub.run(1000 / 200); //サンプリングレート指定，分母の数値で変更可能，1～200Hz
		}

		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
	}
	printf("myo finish");
}

void do_midiget() {
	MMRESULT res;
	UINT wNumDevs; //デバイス数
	UINT wSelDev;
	MIDIINCAPSA MidiInCaps;
	HMIDIIN		hMidiIn;	// デバイスハンドル

							/*//MIDI initialize //IOIを見る場合の初期値設定
							fout.open(file_name, ios::out);
							fout << 0 << "\t" << 0 << "\t" << 0 << endl;
							fout.close();
							*/

							///////	デバイスを確認
	printf("Number of MIDI IN devices is ...");
	printf("\b\b\b   \b\b\b: %d\n", (wNumDevs = midiInGetNumDevs()));
	if (wNumDevs < 1) {
		printf("MIDI IN デバイスがありません。\n");
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}

	///////	デバイス情報を表示して選択
	char inp[4];
	for (int i = 0; i<(int)wNumDevs; i++) {
		if (midiInGetDevCaps(i, &MidiInCaps, sizeof(MidiInCaps)) != MMSYSERR_NOERROR) {
			printf("何らかのエラーが発生しました。\n");	// ここは手抜きのエラー処理
			printf("終了します。何かキーを押して下さい。\n");
			getch();
			exit(1);
		}
		printf("[%d]\n", i + 1);
		printf("      manufacturer ID : %d\n", MidiInCaps.wMid);
		printf("           product ID : %d\n", MidiInCaps.wPid);
		printf("version of the driver : %d\n", MidiInCaps.vDriverVersion);
		printf("         product name : %s\n", MidiInCaps.szPname);
		// メンバはもうひとつ、dwSupport (functionality supported by driver) がある場合もある
	}
	if (wNumDevs > 1) {
		printf("使用するデバイスの番号を入力して下さい:");
		fflush(stdin);
		fgets(inp, sizeof(inp), stdin);
		wSelDev = atoi(inp) - 1;
	}
	else {
		wSelDev = 0;	// ひとつしかデバイスがなければ、自動的にそれを選択
	}

	///////	MIDI IN オープン
	switch ((res = midiInOpen(&hMidiIn, wSelDev, (DWORD)MidiInProc, NULL, CALLBACK_FUNCTION))) {
	case MMSYSERR_NOERROR:
		break;
	case MMSYSERR_ALLOCATED:
		printf("指定されたリソースは既に割り当てられています\n");
		break;
	case MMSYSERR_BADDEVICEID:
		printf("指定されたデバイス識別子 %d は範囲外です。\n", wSelDev);
		break;
	case MMSYSERR_INVALFLAG:
		printf("dwFlags パラメータで指定されたフラグは無効です。\n");
		break;
	case MMSYSERR_INVALPARAM:
		printf("指定されたポインタまたは構造体は無効です。\n");
		break;
	case MMSYSERR_NOMEM:
		printf("システムはメモリを割り当てられないか、またはロックできません。\n");
		break;
	default:
		printf("不明なエラーです。\n");
		break;
	}
	if (res != MMSYSERR_NOERROR) {
		printf("終了します。\n");
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}

	///////	デバイスからの受付を開始
	if ((res = midiInStart(hMidiIn)) != MMSYSERR_NOERROR) {
		if (res == MMSYSERR_INVALHANDLE)
			printf("指定されたデバイスハンドルは無効です。\n");
		else
			printf("不明なエラーです。\n");
		midiInClose(hMidiIn);
		printf("終了します。何かキーを押して下さい。\n");
		getch();
		exit(1);
	}
	printf("MIDI の入力を開始します。\n");
	printf(USAGE_ON_RUNNING);

	flag = true;
	///////	無限ループ
	while (true) {
		///////	キー操作があるまでループ
		while (!kbhit()) {
			// プロシージャは、OS がメッセージを発行した場合に自動的に呼び出され実行されます。
			Sleep(50);//wait 50msec
		}

		///////	入力キーを判断
		unsigned char key = getch();
		/*if (key == VK_ENTER) {
		// Enter キーなら入力をいったん停止して再開
		midiInStop(hMidiIn);
		printf(USAGE_ON_RUNNING);
		midiInStart(hMidiIn);
		continue;
		}
		else*/
		if (key == VK_ESC) {
			// Esc キーなら脱出
			break;
		}
		else if (key == 0 || key == 0xe0) {
			// 矢印と Fn キーの場合は 2byte なのでもう一回 getch() する
			getch();
		}
	}

	flag = false;

	///////	終了処理
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);

	printf("終了します。何かキーを押して下さい。\n");
	getch();

}

int main(int argc, char *argv[]) {
	try {
		std::thread t1(do_audioget);
		std::thread t2(do_myoget);
		std::thread t3(do_midiget);
		t1.join();
		t2.join();
		t3.join();
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
	}

	return (0);
}

