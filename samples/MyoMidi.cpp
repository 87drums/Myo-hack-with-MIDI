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
#define USAGE_ON_RUNNING	"\nEsc �L�[�ŏI��\nTIME STAMP  St Dt Dt : [  ch] Messages\n     0.000  -- -- -- : [--ch] ----\n"
//#define VK_ENTER	13	// Enter
#define VK_ESC		27	// Esc

#pragma comment(lib, "winmm.lib")

//MIDI filename
char *file_name = "data/test_MIDI.dat";
ofstream fout;

bool flag = false; //flag that is to start reading of emg

//�^�C���X�^���v�擾�֐�
/*//�^�C���X�^���v�擾��
long long now = unix_timestamp();
std::cout << now;
*/
long long unix_timestamp() {
	long long unix_epoch_from_1601_int64 = 116444736000000000LL;

	//���ݎ�����FILETIME�\���̂Ɋi�[
	FILETIME current_ft_from_1601;
	GetSystemTimeAsFileTime(&current_ft_from_1601);

	//���ݎ������i�[����FILETIME�\���̂�64�r�b�g�����^�ɕϊ�
	ULARGE_INTEGER current_from_1601;
	current_from_1601.HighPart = current_ft_from_1601.dwHighDateTime;
	current_from_1601.LowPart = current_ft_from_1601.dwLowDateTime;

	//���ݎ����t�@�C�����Ԃ���Unix�G�|�b�N�t�@�C�����Ԃ������Z
	//��Unix�G�|�b�N����̌o�ߎ���(100�i�m�b�P��)�֕ϊ�
	long long diff = current_from_1601.QuadPart - unix_epoch_from_1601_int64;

	//10�Ŋ����Čo�ߎ���(100�i�m�b�P��)���o�ߎ���(�}�C�N���b�P��)�֕ϊ�
	long long t = diff / 10;

	return t;
}

// MIDI ��M�֘A�̊֐�
/*--------------------------------------------------------------
MIDI IN �R�[���o�b�N�֐�

midiInOpen() �Ŏw�肵���v���V�[�W���BOS ��MIDI IN�֘A�̃�
�b�Z�[�W�𔭍s�����ۂɎ����I�ɌĂяo����܂��B
�v���V�[�W���ɏ����𒼐ڏ����Ƃ����Ⴒ���Ⴗ��̂ŁA���ʂ�
���b�Z�[�W���Ƃɏ������֐��ɕ����ď����܂��B����� MIM_DATA
����M�����ꍇ�ɂ̂ݏ�����؂蕪����悤�ɂ��܂����B
--------------------------------------------------------------*/
/*--------------------------------------------------------------
MIDI �f�[�^�̃V���[�g ���b�Z�[�W��M

�V���[�g ���b�Z�[�W����M�����ۂɌĂ΂��֐��ł��B��M��
�b�Z�[�W dwData ����͂��ĕ\�����܂��B
dwData �̉��ʃo�C�g���珇�ɃX�e�[�^�X �o�C�g�A�f�[�^ �o�C
�g 1 �A�f�[�^ �o�C�g 2 �A��o�C�g�Ƃ��܂��B
�X�e�[�^�X �o�C�g�̏�� 4bits �� 1111 �ł���΃V�X�e�����b
�Z�[�W�A1000�`1110 �ł���΃`���l�� ���b�Z�[�W�ł��B�܂���
�� 4bits �̓V�X�e�� ���b�Z�[�W�Ȃ炻�̎�ނ��A�`���l�����b
�Z�[�W�̏ꍇ�͔��s�`���l����\���Ă��܂��B
�ڂ����͋K�i�̉�����Ȃǂ��Q�Ƃ��ĉ������B
--------------------------------------------------------------*/
#define SKIP_TIMING_CLOCK	1	// ���b�Z�[�W 0xF8=timing clock �͖�������
void OnMimData(DWORD dwData, DWORD dwTimeStamp)
{
	unsigned char st, dt1, dt2;	// ��M�f�[�^�i�[�o�b�t�@

								///////	��M�f�[�^�𕪉�
	st = dwData & 0xFF;	// �X�e�[�^�X �o�C�g
	dt1 = dwData >> 8 & 0xFF;	// �f�[�^ �o�C�g�P
	dt2 = dwData >> 16 & 0xFF;	// �f�[�^ �o�C�g�Q

#if SKIP_TIMING_CLOCK
	if (st == 0xF8)
		return;
#endif

	///////	��M�f�[�^��\��
	if ((int)dt2 > 0) {
		long long t_stamp = std::time(0);//unix_timestamp();

		printf("%10.3f  %02X %02X %02X : ", t_stamp, st, dt1, dt2);//printf("%10.3f  %02X %02X %02X : ", (double)dwTimeStamp / 1000, st, dt1, dt2);

																   //write midi file
		fout.open(file_name, ios::app);
		fout << t_stamp << "\t" << (long)dt1 << "\t" << (long)dt2 << endl;//fout << (double)dwTimeStamp << "\t" << (long)dt1 << "\t" << (long)dt2 << endl;
		fout.close();

		///////	��M�f�[�^�𕪐�
		if (st >> 4 == 0xF) {
			///////	�V�X�e�� ���b�Z�[�W
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
			/////// �`���l�� ���b�Z�[�W
			printf("[%2dch] ", (st & 0xF) + 1);
			static const char keys[] = "CDEFGAB";	// ���K
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
	case MIM_OPEN:		// ���̓f�o�C�X �I�[�v�� �R�[���o�b�N
		printf("*** MIM_OPEN ***\n");
		break;
	case MIM_CLOSE:		// ���̓f�o�C�X���N���[�Y
		printf("*** MIM_CLOSE ***\n");
		break;
	case MIM_DATA:		// ���̓f�o�C�X �f�[�^ �R�[���o�b�N
		OnMimData(dwParam1, dwParam2);
		break;
	case MIM_LONGDATA:	// ���̓o�b�t�@�̃R�[���o�b�N
		printf("*** MIM_LONGDATA ***\n");
		break;
	case MIM_ERROR:		// ���̓f�o�C�X �G���[ �R�[���o�b�N
		printf("*** MIM_ERROR ***\n");
		break;
	case MIM_LONGERROR:	// �����ȃV�X�e�� �G�N�X�N���[�V�u ���b�Z�[�W�ɑ΂���R�[���o�b�N
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

//wave�ۑ��@���t�����f�[�^�^��
void do_audioget() {
	//�^�C���X�^���v�ϐ�
	char *ATfile_name = "data/Audio_timestamp.dat";
	ofstream Afout;

	MCIERROR mciError = 0;
	static MCI_OPEN_PARMS mciOpen;
	MCI_WAVE_SET_PARMS mciSet;

	//MCI�R�}���h�@������
	mciOpen.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;
	mciOpen.lpstrElementName = TEXT("");
	mciError = mciSendCommand(0, MCI_OPEN,
		MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID |
		MCI_OPEN_ELEMENT, (DWORD)&mciOpen);

	// �I�[�v�����̃G���[����
	if (mciError != 0) {
		TCHAR szBuf[256];
		mciGetErrorString(mciError, szBuf, sizeof(szBuf) / sizeof(TCHAR));
		printf(szBuf);
		return;
	}

	// �f�o�C�XID�̎擾
	MCIDEVICEID mciDeviceId = mciOpen.wDeviceID;

	// �^���̂��߂̐ݒ�
	mciSet.wFormatTag = WAVE_FORMAT_PCM;
	mciSet.wBitsPerSample = 16;
	mciSet.nSamplesPerSec = 16000;
	mciSet.nChannels = 1;
	mciSet.nBlockAlign = mciSet.nChannels*mciSet.wBitsPerSample / 8;
	mciSet.nAvgBytesPerSec = mciSet.nSamplesPerSec*mciSet.nBlockAlign;

	// �ݒ�̔��f
	mciSendCommand(mciDeviceId, MCI_SET, MCI_WAVE_SET_FORMATTAG | MCI_WAVE_SET_CHANNELS |
		MCI_WAVE_SET_SAMPLESPERSEC | MCI_WAVE_SET_AVGBYTESPERSEC | MCI_WAVE_SET_BLOCKALIGN |
		MCI_WAVE_SET_BITSPERSAMPLE, (DWORD_PTR)&mciSet);


	//MCI�R�}���h�@�^���J�n
	mciSendCommand(mciDeviceId, MCI_RECORD, 0, 0);
	long long start_time = unix_timestamp(); //�I�[�f�B�I�^���J�n�����̃^�C���X�^���v�擾

											 //�I�[�f�B�I�^���������Cwait
	while (!flag) {
		//wait 0.5sec
		Sleep(500);
	}

	while (flag) {
		//wait 0.5sec
		Sleep(500);
	}

	// �^���I���`�f�o�C�X�N���[�Y�܂�
	// MCI�ւ̓��͈����̏���
	MCI_SAVE_PARMS mciSave;

	// �^���̒�~
	long long end_time = unix_timestamp(); //�I�[�f�B�I�^���I�������̃^�C���X�^���v�擾
	mciSendCommand(mciDeviceId, MCI_STOP, MCI_WAIT, 0);

	// �^���������̂��Z�[�u
	mciSave.lpfilename = TEXT("data/test_Audio.wav");
	mciSendCommand(mciDeviceId, MCI_SAVE, MCI_WAIT | MCI_SAVE_FILE, (DWORD_PTR)&mciSave);

	//write midi file
	Afout.open(ATfile_name, ios::app);
	Afout << start_time << "\t" << end_time << endl;
	Afout.close();

	// MCI�̃N���[�Y
	mciSendCommand(mciDeviceId, MCI_CLOSE, 0, 0);
}

//myo signal�擾���C�T���v�����O���[�g��200Hz
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
		}//midi���͂����܂őҋ@

		 // Finally we enter our main loop.
		while (flag) {
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 200 times a second, so we run for 1000/200 milliseconds.
			hub.run(1000 / 200); //�T���v�����O���[�g�w��C����̐��l�ŕύX�\�C1�`200Hz
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
	UINT wNumDevs; //�f�o�C�X��
	UINT wSelDev;
	MIDIINCAPSA MidiInCaps;
	HMIDIIN		hMidiIn;	// �f�o�C�X�n���h��

							/*//MIDI initialize //IOI������ꍇ�̏����l�ݒ�
							fout.open(file_name, ios::out);
							fout << 0 << "\t" << 0 << "\t" << 0 << endl;
							fout.close();
							*/

							///////	�f�o�C�X���m�F
	printf("Number of MIDI IN devices is ...");
	printf("\b\b\b   \b\b\b: %d\n", (wNumDevs = midiInGetNumDevs()));
	if (wNumDevs < 1) {
		printf("MIDI IN �f�o�C�X������܂���B\n");
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}

	///////	�f�o�C�X����\�����đI��
	char inp[4];
	for (int i = 0; i<(int)wNumDevs; i++) {
		if (midiInGetDevCaps(i, &MidiInCaps, sizeof(MidiInCaps)) != MMSYSERR_NOERROR) {
			printf("���炩�̃G���[���������܂����B\n");	// �����͎蔲���̃G���[����
			printf("�I�����܂��B�����L�[�������ĉ������B\n");
			getch();
			exit(1);
		}
		printf("[%d]\n", i + 1);
		printf("      manufacturer ID : %d\n", MidiInCaps.wMid);
		printf("           product ID : %d\n", MidiInCaps.wPid);
		printf("version of the driver : %d\n", MidiInCaps.vDriverVersion);
		printf("         product name : %s\n", MidiInCaps.szPname);
		// �����o�͂����ЂƂAdwSupport (functionality supported by driver) ������ꍇ������
	}
	if (wNumDevs > 1) {
		printf("�g�p����f�o�C�X�̔ԍ�����͂��ĉ�����:");
		fflush(stdin);
		fgets(inp, sizeof(inp), stdin);
		wSelDev = atoi(inp) - 1;
	}
	else {
		wSelDev = 0;	// �ЂƂ����f�o�C�X���Ȃ���΁A�����I�ɂ����I��
	}

	///////	MIDI IN �I�[�v��
	switch ((res = midiInOpen(&hMidiIn, wSelDev, (DWORD)MidiInProc, NULL, CALLBACK_FUNCTION))) {
	case MMSYSERR_NOERROR:
		break;
	case MMSYSERR_ALLOCATED:
		printf("�w�肳�ꂽ���\�[�X�͊��Ɋ��蓖�Ă��Ă��܂�\n");
		break;
	case MMSYSERR_BADDEVICEID:
		printf("�w�肳�ꂽ�f�o�C�X���ʎq %d �͔͈͊O�ł��B\n", wSelDev);
		break;
	case MMSYSERR_INVALFLAG:
		printf("dwFlags �p�����[�^�Ŏw�肳�ꂽ�t���O�͖����ł��B\n");
		break;
	case MMSYSERR_INVALPARAM:
		printf("�w�肳�ꂽ�|�C���^�܂��͍\���͖̂����ł��B\n");
		break;
	case MMSYSERR_NOMEM:
		printf("�V�X�e���̓����������蓖�Ă��Ȃ����A�܂��̓��b�N�ł��܂���B\n");
		break;
	default:
		printf("�s���ȃG���[�ł��B\n");
		break;
	}
	if (res != MMSYSERR_NOERROR) {
		printf("�I�����܂��B\n");
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}

	///////	�f�o�C�X����̎�t���J�n
	if ((res = midiInStart(hMidiIn)) != MMSYSERR_NOERROR) {
		if (res == MMSYSERR_INVALHANDLE)
			printf("�w�肳�ꂽ�f�o�C�X�n���h���͖����ł��B\n");
		else
			printf("�s���ȃG���[�ł��B\n");
		midiInClose(hMidiIn);
		printf("�I�����܂��B�����L�[�������ĉ������B\n");
		getch();
		exit(1);
	}
	printf("MIDI �̓��͂��J�n���܂��B\n");
	printf(USAGE_ON_RUNNING);

	flag = true;
	///////	�������[�v
	while (true) {
		///////	�L�[���삪����܂Ń��[�v
		while (!kbhit()) {
			// �v���V�[�W���́AOS �����b�Z�[�W�𔭍s�����ꍇ�Ɏ����I�ɌĂяo������s����܂��B
			Sleep(50);//wait 50msec
		}

		///////	���̓L�[�𔻒f
		unsigned char key = getch();
		/*if (key == VK_ENTER) {
		// Enter �L�[�Ȃ���͂����������~���čĊJ
		midiInStop(hMidiIn);
		printf(USAGE_ON_RUNNING);
		midiInStart(hMidiIn);
		continue;
		}
		else*/
		if (key == VK_ESC) {
			// Esc �L�[�Ȃ�E�o
			break;
		}
		else if (key == 0 || key == 0xe0) {
			// ���� Fn �L�[�̏ꍇ�� 2byte �Ȃ̂ł������ getch() ����
			getch();
		}
	}

	flag = false;

	///////	�I������
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);

	printf("�I�����܂��B�����L�[�������ĉ������B\n");
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

