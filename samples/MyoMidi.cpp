#define _USE_MATH_DEFINES
#define GNUPLOT_PATH "gnuplot"

#include <stdio.h>

#include <fstream>

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

#include <myo/myo.hpp>

using namespace std;

#pragma comment(lib, "winmm.lib")

/*//スタック自作用
show_num = 10; //表示データ数
int q[10][2]; //time & velocity queue
int q_index = 1; //data size
*/
FILE* gp = _popen(GNUPLOT_PATH, "w");

//MIDI filename
char *file_name = "data/test_MIDI.dat";
ofstream fout;

bool flag = false; //flag that is to start reading of emg


void CALLBACK MidiInProc(
	HMIDIIN midi_in_handle,
	UINT wMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
)
{

	switch (wMsg)
	{

		case MIM_OPEN:
			printf("MIDI device was opened\n");

			break;
		case MIM_CLOSE:
			printf("MIDI device was closed\n");
			break;
		case MIM_DATA:
		{
			flag = true;
			unsigned char status_byte = (dwParam1 & 0x000000ff);
			bool is_noteon_event = status_byte == 0x90;
			if (!is_noteon_event)
			{
				break;
			}
			unsigned char velocity = (dwParam1 & 0x00ff0000) >> 16;
			bool is_pressed = velocity != 0;
			if (!is_pressed)
			{
				break;
			}
			unsigned char note = (dwParam1 & 0x0000ff00) >> 8;
			unsigned int time = (dwParam2 & 0xffffffff) >> 0;
			//printf("MIM_DATA: wMsg=%08X, note_num=%08X, time_stamp=%8X\n", wMsg, dwParam1, dwParam2);
			printf("note = %u, velocity = %u, time = %8d\n", note, velocity, time);
			
			//write midi file
			fout.open(file_name, ios::app);
			fout << time << " " << (int)velocity << endl;
			fout.close();

			//send MIDI to gnuplot
			fprintf(gp, "set xrange [%d:%d]\n", time - 10000, time + 1000); //現時点の10秒前から1秒後まで描画範囲設定
			fprintf(gp, "plot '%s' with line linewidth 10 lc rgb '#0000ff'\n", file_name);
			fprintf(gp, "replot 100 notitle lc rgb '#ff0000'\n");
			fprintf(gp, "replot 50 notitle lc rgb '#ff0000'\n");
			fflush(gp);

			/*//do stack & push
			if (q_index >= show_num) {
				for (size_t i = 1; i <= show_num - 1; i++) {
					q[i-1][0] = q[i][0];
					q[i-1][1] = q[i][1];
				}
				q[show_num-1][0] = time;
				q[show_num-1][1] = velocity;
			}
			else {
				q_index++;
				q[q_index - 1][0] = time;
				q[q_index - 1][1] = velocity;
			}

			for (size_t i = 0; i < show_num; i++) {
				//printf("%d\t%d\t\n", q[i][0], q[i][1]);
				fprintf(gp, "%d\t%d\t\n", q[i][0], q[i][1]);
				fprintf(gp, "e\n");
			}*/
		}
		break;
	case MIM_LONGDATA:
	case MIM_ERROR:
	case MIM_LONGERROR:
	case MIM_MOREDATA:
	default:
		break;
	}
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
		//emgFileString << "data/emg-" << timestamp << ".csv";
		emgFileString << "data/emg.csv";
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
		printf("\r");
		emgFile << timestamp;
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

		while(!flag){} //midi入力されるまで待機

		// Finally we enter our main loop.
		while(flag){
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 50 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 200);
			
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
	HMIDIIN midi_in_handle;
	MMRESULT res;
	UINT device_id;
	WCHAR errmsg[MAXERRORLENGTH];
	char errmsg_buff[MAXERRORLENGTH];

	//MIDI initialize
	fout.open(file_name, ios::out);
	fout << 0 << " " << 0 << endl;
	fout.close();

	//fprintf(gp, "set xrange [%d:%d]\n", time - 10000, time + 1000); //現時点の10秒前から1秒後まで描画範囲設定
	fprintf(gp, "set yrange [%d:%d]\n", 0, 127);
	fprintf(gp, "plot '%s' with line linewidth 10\n", file_name);
	fflush(gp);

	device_id = 1u;

	res = midiInOpen(&midi_in_handle, device_id, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);
	if (res != MMSYSERR_NOERROR) {
		printf("Cannot open MIDI input device %u", device_id);
	}

	printf("Successfully opened a MIDI input device %u.\n", device_id);

	midiInStart(midi_in_handle);

	string input;
	cin >> input; //while (true) {}
	flag = false;

	midiInStop(midi_in_handle);
	midiInReset(midi_in_handle);

	midiInClose(midi_in_handle);

}

int main(int argc, char *argv[]) {

	try {
		std::thread t1(do_myoget);
		std::thread t2(do_midiget);
		t1.join();
		t2.join();
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
	}
	fprintf(gp, "exit\n"); // gnuplotの終了
	fflush(gp);
	_pclose(gp); // パイプを閉じる

	return (0);
}