// Proc Infinity: Start a given process with a specific process affinity via Windows API.
// The affinity is a bitmask of logical CPU cores.

#include <windows.h>
#include <stdio.h> 
#include <iostream>
#include <conio.h> // For _getch()
#include <tchar.h> // for _TCHAR
#include <string_view>
#include <string> //getline
//#include "MouseInput.h"
#include "RMSLib.h"

int _tmain(int nArgs, _TCHAR* cvArgs[]) try
{
	std::ios_base::sync_with_stdio(false);
	ShowWindow(::GetConsoleWindow(), SW_SHOW);

	std::wstring filename{};

	if (nArgs < 2) {
		//Get dropped exe file name
		//Allow drag and drop into the console window
		::DragAcceptFiles(::GetConsoleWindow(), TRUE);
		//Ask user for path to exe with std::wcin and store in a string_vew
		std::wcout << "Drag and drop an executable into this window.\n";

		std::getline(std::wcin, filename);
		if (filename.empty()) {
			std::wcerr << "No file name provided\n";
			return -1;
		}
		
		//Delete quotations
		filename.erase(0, 1);
		filename.erase(filename.size()-1, 1);
	}
	else {
		filename = cvArgs[1];
	}

	
	if (filename[0] == 56797) { //Nonsense character
		std::wcerr << "File path has unreadable characters or string was corrupt.\n";
		return -1;
	}

	//If the target program has extra arguments ~to be passed to it~,
	//Find the end of the program's path, split the string after 
	//the SPACE after ".exe", then pass the target's args to CreateProcess

	auto end_of_target_str = filename.find(L".exe "); //copy subtring AFTER ".exe"

	//npos is string.find's code for "not found", meaning no args for target
	bool target_is_simple = (end_of_target_str == std::string::npos);

	auto target_args = target_is_simple ?
		std::wstring{} : filename.substr(end_of_target_str + 4)
	;
	//if there was nothing after ".exe", target_args is empty

	
	if (!target_is_simple) {
		//cut off anything after the target program's extension
		filename = filename.substr(0, end_of_target_str + 4);
	}
	

	wchar_t ccx_choice{};

	//Get virtual key input from console window if argc < 3.
	if (nArgs > 2) { //Was core number provided?
		ccx_choice = cvArgs[2][0];
	}
	else {
		//ask user for which cores to launch the exe on.
		std::wcout
			<< "Which cores should the process be launched on?\n"
			<< "Press 0 to run the exe on CCX0\nPress 1 to run the exe on CCX1\n"
			<< "2, 3, 4 are each 4-Core groups:\n"
			<< "First, Second, and Fourth.\n";
		;
		ccx_choice = static_cast<wchar_t>(_getch());
	}
	
	
	//Set the process affinity to the first 8 cores if ccx_num is '0', 1 to set to the second 8 cores,
	//'2' to set to the first 4 cores, and '3' to set to the second 4 cores,
	//and '4' to set to the last 4 cores. //the third 4 cores are intentionally unsued
	switch (ccx_choice) //switch more like swift
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
	default:
		break;
	}

	
	//endl because it's the last post
	std::wcout << "Trying to launch: " << filename << "\nwith " << ccx_choice << std::endl;

	if (target_is_simple) {
		auto safeProc = rs::SafeCreateProcess(filename);
		WaitForSingleObject(safeProc.pi.hProcess, INFINITE);
		return 0;
	}
	else {
		auto safeProc = rs::SafeCreateProcess(filename, target_args);
		WaitForSingleObject(safeProc.pi.hProcess, INFINITE);
		return 0;

	}
	//safeProc is destroyed here via RAII
	
	return 0;
}
catch( std::exception& ex ) {
	std::wcerr << ex.what();
}
