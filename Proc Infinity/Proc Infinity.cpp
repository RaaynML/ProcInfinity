// Proc Infinity: Start a given process with a specific process affinity via Windows API.
// The affinity is a bitmask of logical CPU cores in Hex.

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601 //Try to work on Win 7
#include <sdkddkver.h>

#include <windows.h>
#include <stdio.h> 
#include <iostream>
#include <conio.h> // For _getch()
#include <tchar.h> // for _TCHAR
#include <string_view>
#include <string> //getline

//#include "MouseInput.h"
#include "RMSLib.h"

#define RS_FONT_SETTINGS 1 //set the console font?



int _tmain(int nArgs, _TCHAR* cvArgs[]) try
{
	std::ios_base::sync_with_stdio(false);
	ShowWindow(::GetConsoleWindow(), SW_HIDE);  //CREATE_NO_WINDOW

	std::wstring filename{};

	if (nArgs < 2) {
		//Allow drag and drop into the console window
		::DragAcceptFiles(::GetConsoleWindow(), TRUE);
		//Ask user for path to exe with std::wcin and store in a string_vew
		std::wcout << "Drag and drop an executable into this window, then press Enter.\n";

		std::getline(std::wcin, filename);
		if (filename.empty()) {
			std::wcerr << "No file name provided\n";
			return -1;
		}
		
		//Delete quotes
		filename.erase(0, 1);
		filename.erase(filename.size()-1, 1);
	}
	else {
		filename = cvArgs[1];
	}

	
	if (filename[0] == 56797) { //Nonsense character
		std::wcerr << "File path is unreadable or corrupt.\n";
		return -1;
	}


	//Find the end of the program's path, split the string after 
	//the SPACE after ".exe", then pass the target's args to CreateProcess
	auto end_of_target_str = filename.find(L".exe "); //copy subtring AFTER ".exe"

	//npos is string.find's response for "not found"
	bool target_is_simple = (end_of_target_str == std::string::npos);

	auto target_args = target_is_simple ?
		std::wstring{} : filename.substr(end_of_target_str + 4)
	;
	//if there was nothing after ".exe", target_args is empty

	
	if (!target_is_simple) {
		//cut off anything after the target program's extension
		filename = filename.substr(0, end_of_target_str + 4);
	}


#ifdef _WINDOWS_
#undef max //Windows.h is a bit stupid
#endif //_WINDOWS_

	wchar_t ccx_choice = std::numeric_limits<wchar_t>::max();

	//Get virtual key input from console window if argc < 3.
	if (nArgs > 2) { //Was core number provided?
		ccx_choice = cvArgs[2][0];
	}
	else {
		//ask user for which cores to launch the exe on.
		std::wcout
			<< "Which cores should the process be launched on?\n\
				\n\t\tPress keys to select an option:\
				\n 0 - Run on CCX0\
				\n 1 - Run on CCX1\n\
				\n w (lower) - Run on CCX0 minus threads 0 & 1\
				\n W (upper) - Run all threads minus 0, 1, 8, 9\n\
				\n\tFor 4-Core groups:\
				\n 2 - [ 0,  1,  2,  3]\
				\n 3 - [ 4,  5,  6,  7]\
				\n F - Pay respects"
		;
		
		Sleep(160);
		std::wcout
			<< "\r F - [ 8,  9, 10, 11]\
				\n 4 - [12, 13, 14, 15]\
				\n 5 - The 16th core [only]\n"
			;
		//\n 6 - The  6th core [only]"
		//\n 7 - The  7th core [only]"
		;//caret
		ccx_choice = static_cast<wchar_t>(_getch());
	}
	
	//Priority:
	//SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS)

	
	switch(ccx_choice) //at -O2 this is generally a LUT
	{
	case '0':
		SetProcessAffinityMask(GetCurrentProcess(), 0x00FF);
		break;
	case '1':
		SetProcessAffinityMask(GetCurrentProcess(), 0xFF00);
		break;
	case '2':
		SetProcessAffinityMask(GetCurrentProcess(), 0x000F);
		break;
	case '3':
		SetProcessAffinityMask(GetCurrentProcess(), 0x00F0);
		break;
	case '4':
		SetProcessAffinityMask(GetCurrentProcess(), 0xF000);
		break;
	case '9': //Surely it's not important
		SetProcessAffinityMask(GetCurrentProcess(), 0x8000);
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		break;
	case 'f':
		SetProcessAffinityMask(GetCurrentProcess(), 0x0F00);
		break;
	case 'F':
		SetProcessAffinityMask(GetCurrentProcess(), 0x0F00);
		break;
	case 'w':
		SetProcessAffinityMask(GetCurrentProcess(), 0x00FC);
		break;
	case 'W':
		SetProcessAffinityMask(GetCurrentProcess(), 0xFCFC);
		break;
	default:
		std::wcerr << "This is not a valid option" << std::endl;
		exit(-1);
		break;
	}

#ifdef _DEBUG
	std::wcout
		<< "Trying to launch: " << filename
		<< "\nwith " << ccx_choice << '\n';
	;
#endif

	if (target_is_simple) {
		auto safeProc = rs::AutoCreateProcess(filename);
		WaitForSingleObject(safeProc.pi.hProcess, 2000);
		return 0;
	}
	else {
		auto safeProc = rs::AutoCreateProcess(filename, target_args);
		WaitForSingleObject(safeProc.pi.hProcess, 2000);
		return 0;

	}
	
	return 0;
}
catch( std::exception& ex ) {
	std::wcerr << ex.what();
}
