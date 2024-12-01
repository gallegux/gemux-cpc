/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD pokes introduction                                                    |
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


#include <string>
#include <vector>
#include <algorithm>
#include "osd_input.h"
#include "osd_pokes.h"
#include "util.h"
#include "util_windows.h"
#include "../tipos.h"
#include "../monitor.h"  // para OSD_COLS y OSD_ROWS
#include "../util.h"
#include "../directories.h"
#include "../log.h"



OSD_PokeItem:: OSD_PokeItem(u8 _number) : number(_number) {}


std::string OSD_PokeItem:: toString() {
	std::string s;
	if (number < 10)  s.push_back(' ');
	s.append( std::to_string(number) );
	s.append(":  ");

	if (address != 0) {
		s.append( toHex(address, 4) );
		s.append(" : ");
		s.append( toHex(value, 2) );
	}
	debug_osd("OSD:: POKE %d [%s]\n", number, s.c_str());
	return s;
}


void OSD_PokeItem:: setAddress(const std::string& str) {
	address = std::stoi(str, nullptr, 16);
}

void OSD_PokeItem:: setValue(const std::string& str) {
	value = std::stoi(str, nullptr, 16);
}



OSD_PokeSelection:: OSD_PokeSelection(std::string title, OSD* osd) :
	OSD_LittleSelect(title, osd)  {
	OSD_PokeItem* pi;

	for (u8 i = 0; i < NUM_POKES; i++) {
		pi = new OSD_PokeItem(i+1);
		addItem(pi);
	}

	//itemsRows = NUM_POKES;
	setHeight(false, NUM_POKES, true);
	setStatus("RETURN Modify | F8 Remove | F10 Accept");
	setWidth(40);
}



void OSD_PokeSelection:: getPoke(u8 numPoke, u16* address, BYTE* value) {
	*address = 0;
	*value = 0;

	if (numPoke < NUM_POKES) {
		OSD_SelectionItem* i = items.at(numPoke);
		OSD_PokeItem* pi = reinterpret_cast<OSD_PokeItem*>(i);
		//debug_osd("OSD:: address=%d value=%d\n", pi->address, pi->value);
		if (pi->address != 0) {
			*address = pi->address;
			*value = pi->value;
		}
	}
}



void OSD_PokeSelection:: k_return() {
	u8 r = winRow + item1row + cursor;
	u8 c = winCol + 6;

	OSD_SelectionItem* i = items.at(cursor);
	OSD_PokeItem* pi = reinterpret_cast<OSD_PokeItem*>(i);
	std::string x;
	bool primeraEdicion = false;
	
	if (pi->address != 0)  x = toHex(pi->address, 4);
	else  primeraEdicion = true;
	
	OSD_Input inputAddress {osd, c, r, 4, x, WINDOW_INPUT_BG, WINDOW_INPUT_FG};
	inputAddress.setInputHex(true);

	if (inputAddress.getAnswer(x)) {  // salida con tab o ret
		if (x == "") writeItem(cursor, true);
		else {
			pi->address = std::stoi(x, nullptr, 16);
			debug_osd("ADDRESS: %04X  %d\n", pi->address, pi->address);
			if (pi->address == 0)  
				k_F8();
			else {
				writeItem(cursor, true);
				osd->updateUI();

				// preguntar value
				x = (primeraEdicion) ? "" : toHex(pi->value, 2);
				OSD_Input inputvalue {osd, c+7, r, 2, x, WINDOW_INPUT_BG, WINDOW_INPUT_FG};
				inputvalue.setInputHex(true);

				if (inputvalue.getAnswer(x)) {
					pi->value = std::stoi(x, nullptr, 16);
					if (cursor < itemsRows-1) moveDown();
					else writeItem(cursor, true);
				}
				else  {
					pi->address = 0;
					pi->value = 0;
				}
			}
		}
	}
	else writeItem(cursor, true);

	osd->updateUI();
}


void OSD_PokeSelection:: k_F8() {
	OSD_SelectionItem* i = items.at(cursor);
	OSD_PokeItem* pi = reinterpret_cast<OSD_PokeItem*>(i);

	pi->address = 0;
	pi->value = 0;

	writeItem(cursor, true);
	osd->updateUI();
}


void OSD_PokeSelection:: k_F10() {
	exitF10 = true;
	finish();
}


bool OSD_PokeSelection:: getExitOk() { return exitF10; }

