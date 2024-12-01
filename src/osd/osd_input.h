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


#pragma once

#include <cctype> // std::to_upper(caracter)
#include <string>
#include <algorithm>  // Para std::find
#include <SDL2/SDL_keycode.h>
#include "osd.h"
#include "util.h"


class OSD_Input {
/* tengo dos cadenas, una principal y otra oculta. en la principal guardo los caracteres ascii (del 32 al 126) y el 
   caracter de ESCAPE (-61), y en la oculta se guarda el caracter extendido en la misma posicion que el de escape */
	OSD* osd;
	std::string text;
	std::string hidden;
	std::string composed;
	bool onlyHex = false; // solo caracteres hexadecimales
	bool onlyAscii = true;
	u8 cursor = 0;
	u8 maxLen;
	u8 col, row; // posicion en pantalla en la que aparece el cursor
	TColor bg;
	TColor fg;
	bool exitOnTab = false; // salir del input si se presiona TAB, flecha_arriba o flecha_abajo

	bool insert(char* c);  // devuelve si fue insertado
	bool deleteLeft();
	bool deleteRight();
	void printCursor();
	bool moveCursorLeft();
	bool moveCursorRight();
	bool moveCursorBegin();
	bool moveCursorEnd();

public:
	OSD_Input(OSD* osd, u8 _col, u8 _row, u8 _maxLen, const std::string& initialValue, TColor _bgColor, TColor _fgColor);
	bool getAnswer(std::string& resp);
	void setExitOnTab(bool eot);
	void setInputHex(bool h);
	void setOnlyAscii(bool a);
	
};
