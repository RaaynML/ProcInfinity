//This C++ header includes a class that handles mouse input for the command prompt.

#include <windows.h>
#include <stdio.h>

class MouseInput {
public:
	MouseInput(auto& _consolein) {
		console = &_consolein;
		this->mouse_pos.X = 0;
		this->mouse_pos.Y = 0;
	}


	//mouseEventProc checks if the mouse clicked and where the mouse is.
	void mouseEventProc() {
		INPUT_RECORD input_record;
		DWORD num_events;
		GetNumberOfConsoleInputEvents(console, &num_events);
		if (num_events > 0) {
			ReadConsoleInput(console, &input_record, 1, &num_events);
			if (input_record.EventType == MOUSE_EVENT) {
				this->mouse_event = input_record.Event.MouseEvent;
			}
		}

		this->mouse_pos = this->mouse_event.dwMousePosition;
		SetConsoleCursorPosition(console, this->mouse_pos);
	}

	//void resizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

public:
	COORD mouse_pos;
	bool mouse_clicked;
	
	MOUSE_EVENT_RECORD mouse_event;
	HANDLE console;
};
