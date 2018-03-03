#include <stdio.h>

#include <windows.h> 
#include <mmsystem.h> 

#include<iostream>
#include<string>
using namespace std;

#pragma comment(lib, "winmm.lib")

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
		printf("MIM_DATA: wMsg=%08X, note_num=%08X, time_stamp=%08X\n", wMsg, dwParam1, dwParam2);
		printf("note = %u, velocity = %u, time = %10d\n", note, velocity, time);
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

int sub_main1(int argc, char** argv)
{
	HMIDIIN midi_in_handle;
	MMRESULT res;
	UINT device_id;
	WCHAR errmsg[MAXERRORLENGTH];
	char errmsg_buff[MAXERRORLENGTH];

	device_id = 1u;

	res = midiInOpen(&midi_in_handle, device_id, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);
	if (res != MMSYSERR_NOERROR) {
		printf("Cannot open MIDI input device %u", device_id);
		return 1;
	}

	printf("Successfully opened a MIDI input device %u.\n", device_id);

	midiInStart(midi_in_handle);

	string input;
	cin >> input;//while (true) {}

	midiInStop(midi_in_handle);
	midiInReset(midi_in_handle);

	midiInClose(midi_in_handle);

	return 0;
}