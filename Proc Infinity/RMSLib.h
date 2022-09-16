#pragma once
#include <string_view>
#include <windows.h>    //MultiByteToWideChar
#include <stdexcept>


namespace rs {
	//wrapper for CreateProcess
	class SafeCreateProcess {
	public:
		SafeCreateProcess(std::wstring _filename) {
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			
			CreateProcess(
				_filename.c_str(),
				NULL, NULL, NULL,	//cmd line, procAttribs, threadAttribs
				FALSE, 0, NULL,	 	//binheritHandles, creationFlags, env
				NULL, &si, &pi	 	//workingDirectory, startupInfo, processInfo
			);

			//::ShowWindow(::GetConsoleWindow(), SW_HIDE);
		}

		SafeCreateProcess(std::wstring _filename, std::wstring _target_args) {
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			CreateProcess(
				_filename.c_str(), _target_args.data(),
				NULL, NULL, 		//procAttribs, threadAttribs
				FALSE, 0, NULL,	 	//binheritHandles, creationFlags, env
				NULL, &si, &pi	 	//workingDirectory, startupInfo, processInfo
			);

			//::ShowWindow(::GetConsoleWindow(), SW_HIDE);
		}

		~SafeCreateProcess() {
			//Close process and thread handles
			if(pi.hProcess != nullptr){ CloseHandle(pi.hProcess);	}
			if(pi.hThread != nullptr) { CloseHandle(pi.hThread);	}
		}
	public:
		PROCESS_INFORMATION pi;
	};
}
