/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Input text on OSD                                                         |
|                                                                            |
|  Copyright (c) 2024 Gallegux (gallegux@gmail.com)                          |
|                                                                            |
|  This program is free software: you can redistribute it and/or modify      |
|  it under the terms of the GNU General Public License as published by      |
|  the Free Software Foundation, either version 3 of the License, or         |
|  any later version.                                                        |
|                                                                            |
|  This program is distributed in the hope that it will be useful, but       |
|  WITHOUT ANY WARRANTY; without even the implied warranty of                |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              |
|  GNU General Public License for more details.                              |
|                                                                            |
|  You should have received a copy of the GNU General Public License         |
|  along with this program; if not, see <http://www.gnu.org/licenses/>.      |
|                                                                            |
|  If you use this code, please attribute the original source by mentioning  |
|  the author and providing a link to the original repository.               |
|___________________________________________________________________________*/


#include <cctype> // std::to_upper(caracter)
#include <string>
#include <algorithm>  // Para std::find
#include <SDL2/SDL_keycode.h>
#include "osd_input.h"
#include "util.h"
#include "../tipos.h"
#include "../directories.h"
#include "../target.h"
#include "../log.h"


#ifdef TARGET_PC
extern bool GLOBAL_QUIT;
#endif



//const std::vector<SDL_KeyCode> INPUT_ESPECIAL_CHARS {
//	SDLK_PERIOD, SDLK_SPACE, SDLK_UNDERSCORE, SDLK_MINUS, SDLK_LEFTPAREN, SDLK_RIGHTPAREN	
//};

constexpr std::string_view INPUT_ESPECIAL_CHARS = " -_!()[].,@#$~";
constexpr std::string_view NOT_VALID_CHARS = "/\\:*?\"<>|";
constexpr std::string_view HEXADECIMAL_CHARS = "0123456789abcdefABCDEF";

//void comprobarModificadores() {
//    SDL_Keymod modState = SDL_GetModState(); // Obtener el estado de los modificadores
//
//    // Comprobar cada modificador
//    if (modState & KMOD_CAPS) {
//        std::cout << "Caps Lock (BLOQMAYS) está activado." << std::endl;
//    }
//    if (modState & KMOD_LSHIFT) {
//        std::cout << "Left Shift (LSHIFT) está activado." << std::endl;
//    }
//    if (modState & KMOD_RSHIFT) {
//        std::cout << "Right Shift (RSHIFT) está activado." << std::endl;
//    }
//    if (modState & KMOD_LCTRL) {
//        std::cout << "Left Control (LCONTROL) está activado." << std::endl;
//    }
//    if (modState & KMOD_RCTRL) {
//        std::cout << "Right Control (RCONTROL) está activado." << std::endl;
//    }
//    if (modState & KMOD_LALT) {
//        std::cout << "Left Alt (ALT) está activado." << std::endl;
//    }
//    if (modState & KMOD_RALT) {
//        std::cout << "Right Alt (AltGr) está activado." << std::endl;
//    }
//}
//
//bool isMaysSelected() {
//	SDL_Keymod modState = SDL_GetModState();
//	return (modState & KMOD_CAPS) != 0  ||  (modState & KMOD_LSHIFT) != 0  ||  (modState & KMOD_RSHIFT) != 0 ;
//    //const Uint8* keyState = SDL_GetKeyboardState(NULL);
//	//return (keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT] || (modState & KMOD_CAPS) != 0);
//}

//--------------------------------------------------------------------------------


OSD_Input:: OSD_Input(OSD* _osd, u8 _col, u8 _row, u8 _maxLen, const std::string& initialValue, TColor _bgColor, TColor _fgColor) :
	osd(_osd), col(_col), row(_row), maxLen(_maxLen), text(initialValue), bg(_bgColor), fg(_fgColor)
{
	if (initialValue != "") {
		// descomponer initialValue en text y hidden
		string_decompose(initialValue, text, hidden);
	}
	cursor = text.size();
}


bool OSD_Input:: insert(char* c) {
	debug_osd("<%d>", *c);
	if (text.size() < maxLen) {
		if (onlyHex) {
			if (HEXADECIMAL_CHARS.find(*c) == std::string::npos) return false;
			*c = std::tolower(*c);
		}
		if (*c != ESCAPE_CHAR) {
			text.insert(cursor, 1, *c);
			hidden.insert(cursor, 1, ' ');
			cursor++;
		}
		else {
			if (onlyAscii) {
				if (*c >= 32  &&  *c < 127) {
					text.insert(cursor, 1, *c);
					c++;
					hidden.insert(cursor, 1, *c);
					cursor++;
				}
			}
			else {
				text.insert(cursor, 1, *c);
				c++;
				hidden.insert(cursor, 1, *c);
				cursor++;
			}
		}
		return true;
	}
	else return false;
}


bool OSD_Input:: deleteLeft() {
	if (cursor > 0) {
		--cursor;
		text.erase(cursor, 1);
		hidden.erase(cursor, 1);
		return true;
	}
	else return false;
}

bool OSD_Input:: deleteRight() {
	if (cursor <= text.size()) {
		text.erase(cursor, 1);
		hidden.erase(cursor, 1);
		return true;
	}
	else return false;
}

bool OSD_Input:: moveCursorLeft() {
	if (cursor > 0) {
		cursor--;
		return true;
	}
	else return false;
}

bool OSD_Input:: moveCursorRight() {
	if (cursor < maxLen  &&  cursor < text.size()) {
		cursor++;
		return true;
	}
	else return false;
}

bool OSD_Input:: moveCursorBegin() {
	if (cursor != 0) {
		cursor = 0;
		return true;
	}
	else return false;
}

bool OSD_Input:: moveCursorEnd() {
	if (cursor != text.size()) {
		cursor = text.size();
		return true;
	}
	else return false;
}



void OSD_Input:: printCursor() {
	osd->setColors(fg, bg);

	std::string chr;
	if (cursor >= text.size()) {
		chr = " ";
	}
	else {
		chr = text.at(cursor);
	}
	osd->writeTextC(col + cursor, row, chr);
}



void OSD_Input:: setExitOnTab(bool eot) { exitOnTab = eot; }

void OSD_Input:: setInputHex(bool h) { onlyHex = h; }

void OSD_Input:: setOnlyAscii(bool a) { onlyAscii = a; }



bool OSD_Input:: getAnswer(std::string& resp) {
	//debug_osd("OSD:: input() ini  %d,%d %d\n", col, row, maxLen);
	osd->setColors(bg, fg);
	osd->drawFilledRectangleC(col, row, maxLen+1, 1);
	osd->writeFastTextC(col, row, composed);
	printCursor();
	osd->updateUI();

	bool cambios = true;
	SDL_Keycode keycode;
	SDL_Event event;
	SDL_Scancode scancode;
	SDL_StartTextInput();

    while (true && !GLOBAL_QUIT) {
		SDL_PollEvent(&event);
		keycode = event.key.keysym.sym;
		scancode = SDL_GetScancodeFromKey(keycode);

		switch (event.type) {
			#ifdef TARGET_PC
			case SDL_QUIT:
				GLOBAL_QUIT = true; 
				return "";
				break;
			#endif

			case SDL_TEXTINPUT: 
				cambios = insert(event.text.text);
				break;

			case SDL_KEYDOWN: {
				switch (scancode) {
					case SDL_SCANCODE_RETURN: 
						SDL_StopTextInput();
						debug_osd("OSD:: text[%s] hidden[%s]\n", text.c_str(), hidden.c_str());
						resp = composed;
						return true;
						break;
					case SDL_SCANCODE_ESCAPE:
						SDL_StopTextInput();
						return false;
					case SDL_SCANCODE_LEFT:       cambios = moveCursorLeft();  break;
					case SDL_SCANCODE_RIGHT:      cambios = moveCursorRight(); break;
					case SDL_SCANCODE_HOME:       cambios = moveCursorBegin(); break;
					case SDL_SCANCODE_END:        cambios = moveCursorEnd();   break;
					case SDL_SCANCODE_BACKSPACE:  cambios = deleteLeft();      break;
					case SDL_SCANCODE_DELETE:     cambios = deleteRight();     break;
					//case SDL_SCANCODE_UP:
					//case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_TAB:
						if (exitOnTab) {
							SDL_StopTextInput();
							resp = composed;
							return true;
						}
				}
			}
		}
		if (cambios) {
			composed = string_compose(text, hidden);
			osd->setColors(bg, fg);
			osd->drawFilledRectangleC(col, row, maxLen+1, 1);
			osd->writeFastTextC(col, row, composed);
			printCursor();
			osd->updateUI();
			cambios = false;
		}
	}
	return false;
}
