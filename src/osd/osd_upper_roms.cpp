/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD upper roms selection                                                  |
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
#include "osd.h"
#include "osd_select_file.h"
#include "osd_upper_roms.h"
#include "util.h"
#include "util_windows.h"
#include "../tipos.h"
#include "../monitor.h"  // para OSD_COLS y OSD_ROWS
#include "../directories.h"
#include "../log.h"



OSD_UpperRomItem:: OSD_UpperRomItem(u8 _slot, const std::string& _file, bool _loaded) :
	slot(_slot), file(_file), loaded(_loaded) {}

std::string OSD_UpperRomItem:: toString() {
	std::string s;

	if (slot < 10) s.push_back(' ');
	s.append( std::to_string(static_cast<int>(slot)) );
	s.push_back(':');

	if (loaded) {
		s.push_back(' ');
		s.append(file);
	}
	return s;
}


std::string& OSD_UpperRomItem:: getFile() { return file; }

void OSD_UpperRomItem:: setFile(std::string f) { file = f; }

bool OSD_UpperRomItem:: isLoaded() { return loaded; }

void OSD_UpperRomItem:: setLoaded(bool l) { loaded = l; }




OSD_UpperRomSelection:: OSD_UpperRomSelection(std::string title, OSD* osd) :
	OSD_BigSelect(title, osd)  {}



std::string& OSD_UpperRomSelection:: getUpperRom(u8 slot) { 
	OSD_SelectionItem* item = items.at(slot);
	OSD_UpperRomItem* romItem = reinterpret_cast<OSD_UpperRomItem*>(item);

	return romItem->getFile();
}


void OSD_UpperRomSelection:: setUpperRom(u8 slot, const std::string& romFile, bool loaded) {
	OSD_UpperRomItem* ur = new OSD_UpperRomItem(slot, romFile, loaded);
	items.push_back(ur);
}



void OSD_UpperRomSelection:: writeItem(bool selected) {
	if (selected)
		osd->setColors(WINDOW_SELECTED_ITEM_BG, WINDOW_SELECTED_ITEM_FG);
	else
		osd->setColors(WINDOW_ITEM_BG, WINDOW_ITEM_FG);

	OSD_SelectionItem* wi = items.at(cursor);
	OSD_UpperRomItem* ri = reinterpret_cast<OSD_UpperRomItem*>(wi);

	if (!ri->loaded) osd->setForeground(ROM_NOT_LOADED);

	osd->drawFilledRectangleC(winCol, winRow + item1row + cursor - firstInWindow, winWidth, 1);
	osd->writeFastTextC(winCol+1, winRow + item1row + cursor - firstInWindow, ri->toString());
}


void OSD_UpperRomSelection:: printWindow() {
	OSD_TitledWindow::printWindow();
	printItemsPage();
	clearRestPage();
	printHelp();
	printCursorPosition();
}


void OSD_UpperRomSelection:: k_return() {  // changeRom() {
	OSD_SelectionItem* wi = items.at(cursor);
	OSD_UpperRomItem* fi = reinterpret_cast<OSD_UpperRomItem*>( items.at(cursor) );

	OSD_FileSelection w {"Select a Upper ROM image", osd, ROMS_DIR, ROMS_EXT};
	w.setHeight(true, 20, true);
	w.setWidth(50);
	osd->push(w);
	w.show();
	
	if (w.getExitOk()) {
		std::filesystem::path file = w.getSelectedFile();
		debug_osd("OSD:: upper_rom [%s]\n", file.string().c_str());
		osd->pop();
		fi->setFile(file.string());
		fi->setLoaded(true);
		writeItem(true);
	}
	else osd->pop();

	osd->updateUI();
}


void OSD_UpperRomSelection:: k_F8() {  // removeRom() {
	OSD_SelectionItem* wi = items.at(cursor);
	OSD_UpperRomItem* fi = reinterpret_cast<OSD_UpperRomItem*>(wi);
	
	fi->setLoaded(false);
	fi->setFile("");
	fi->setLoaded(true);

	writeItem(true);
	osd->updateUI();
}


void OSD_UpperRomSelection:: k_F10() {  // accept
	exitF10 = true;
	finish();
}


bool OSD_UpperRomSelection:: getExitOk() { return exitF10; }


void OSD_UpperRomSelection:: printHelp() {
	printStatus("RETURN Change | F8 Delete | F10 Accept");
	//std::string text { "RETURN Change | F8 Delete | F10 Accept" };
	//clearStatusRow();
	//osd->writeFastTextC(winCol+1, statusRow, text);
}


