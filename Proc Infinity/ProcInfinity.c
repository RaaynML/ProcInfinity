#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TEXT_CAPACITY 32768

static BOOL create_process(const wchar_t* filename,const wchar_t* target_args, PROCESS_INFORMATION* pi,DWORD* error_code){
	STARTUPINFOW si;
	wchar_t command_line[TEXT_CAPACITY];

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(pi, sizeof(*pi));

	if(swprintf_s(command_line,_countof(command_line), L"\"%ls\"%ls", filename,target_args) < 0){
		*error_code = ERROR_INSUFFICIENT_BUFFER;
		return FALSE;
	}

	if(!CreateProcessW(filename,command_line, NULL,NULL, FALSE, CREATE_NO_WINDOW, NULL,NULL, &si,pi)){
		*error_code = GetLastError();
		return FALSE;
	}

	return TRUE;
}

int wmain(int argc, wchar_t* argv[]){
	wchar_t filename[TEXT_CAPACITY] = L"";
	wchar_t target_args[TEXT_CAPACITY] = L"";
	wchar_t* exe_suffix;
	wchar_t choice;
	size_t length;
	DWORD_PTR affinity_mask = 0;
	DWORD error_code = ERROR_SUCCESS;
	BOOL set_idle_priority = FALSE;
	PROCESS_INFORMATION pi;


	if(argc < 2){
		DragAcceptFiles(GetConsoleWindow(), TRUE);
		wprintf(L"Drag and drop an executable into this window, then press Enter.\n");

		if(fgetws(filename, (int)_countof(filename), stdin) == NULL){
			fwprintf(stderr, L"No file name provided.\n");
			return -1;
		}

		length = wcslen(filename);
		if(length > 0 && filename[length - 1] == L'\n'){
			filename[--length] = L'\0';
		}
		if(length > 0 && filename[length - 1] == L'\r'){
			filename[--length] = L'\0';
		}

		if(length >= 2 && filename[0] == L'"' && filename[length - 1] == L'"'){
			memmove(filename, filename + 1, (length - 2) * sizeof(wchar_t));
			filename[length - 2] = L'\0';
		}

		if(filename[0] == L'\0'){
			fwprintf(stderr, L"No file name provided.\n");
			return -1;
		}
	}else{
		if(wcsncpy_s(filename,_countof(filename), argv[1], _TRUNCATE) != 0){
			fwprintf(stderr, L"File name is too long.\n");
			return -1;
		}
	}

	if(filename[0] == (wchar_t)56797){
		fwprintf(stderr, L"File path is unreadable or corrupt.\n");
		return -1;
	}

	exe_suffix = wcsstr(filename, L".exe ");
	if(exe_suffix != NULL){
		if(wcsncpy_s(target_args, _countof(target_args), exe_suffix + 4, _TRUNCATE) != 0){
			fwprintf(stderr, L"Arguments are too long.\n");
			return -1;
		}
		exe_suffix[4] = L'\0';
	}

	if(argc > 2){
		choice = argv[2][0];
	}else{
		wprintf(
			L"Which cores should the process be launched on?\n"
			L"\n\t\tPress keys to select an option:"
			L"\n 0 - Run on CCX0"
			L"\n 1 - Run on CCX1"
			L"\n w (lower) - Run on CCX0 minus threads 0 & 1"
			L"\n W (upper) - Run all threads minus 0, 1, 8, 9"
			L"\n\n\t\tFor 4-Core groups:"
			L"\n 2 - [ 0,  1,  2,  3]"
			L"\n 3 - [ 4,  5,  6,  7]"
			L"\n F - Pay respects"
		);
		fflush(stdout);
		Sleep(160);
		wprintf(
			L"\r F - [ 8,  9, 10, 11]"
			L"\n 4 - [12, 13, 14, 15]"
			L"\n 5 - The 7th core [only]\n"
			L"\n 9 - The 16th core [only]\n"
		);

		choice = (wchar_t)_getch();
	}

	switch(choice){
	case L'0':
		affinity_mask = 0x00FF;
		break;
	case L'1':
		affinity_mask = 0xFF00;
		break;
	case L'2':
		affinity_mask = 0x000F;
		break;
	case L'3':
		affinity_mask = 0x00F0;
		break;
	case L'4':
		affinity_mask = 0xF000;
		break;
	case L'5':
		affinity_mask = 0x0080;
		set_idle_priority = TRUE;
		break;
	case L'9':
		affinity_mask = 0x8000;
		set_idle_priority = TRUE;
		break;
	case L'f':
	case L'F':
		affinity_mask = 0x0F00;
		break;
	case L'w':
		affinity_mask = 0x00FC;
		break;
	case L'W':
		affinity_mask = 0xFCFC;
		break;
	default:
		fwprintf(stderr, L"This is not a valid option.\n");
		return -1;
	}

	if(!SetProcessAffinityMask(GetCurrentProcess(),affinity_mask)){
		fwprintf(stderr, L"SetProcessAffinityMask failed: %lu\n", (unsigned long)GetLastError());
		return -1;
	}

#ifdef _DEBUG
	wprintf(L"Trying to launch: %ls\nwith %lc\n", filename,choice);
#endif

	/* Before create_process(), after the affinity call: */
	if(set_idle_priority && !SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS)){
		fwprintf(stderr, L"SetPriorityClass failed: %lu\n", (unsigned long)GetLastError());
		return -1;
	}

	if(!create_process(filename,target_args, &pi,&error_code)){
		fwprintf(stderr, L"CreateProcessW failed: %lu\n", (unsigned long)error_code);
		return -1;
	}

	ShowWindow(GetConsoleWindow(),SW_HIDE); //idk

	WaitForSingleObject(pi.hProcess,2000);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}
