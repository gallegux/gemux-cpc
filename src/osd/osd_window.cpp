/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD window generic implementation                                         |
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
#include "osd_window.h"
#include "../tipos.h"
#include "../target.h"
#include "../monitor.h"  // para OSD_COLS y OSD_ROWS
#include "../util.h"
#include "../log.h"


#ifdef TARGET_PC
extern bool GLOBAL_QUIT;
#endif

//=================================================================================================

OSD_Window:: OSD_Window(OSD* _osd) : osd(_osd) {}

void OSD_Window:: setWidth(u8 w)  { winWidth = w; }

void OSD_Window:: setHeight(u8 h) { winHeight = h; }

void OSD_Window:: getBounds(u8* _col, u8* _row, u8* _width, u8* _height) const {
	*_col = winCol;
	*_row = winRow;
	*_width = winWidth;
	*_height = winHeight;
}

void OSD_Window:: calculateWidth() {
	if (winWidth == 0) winWidth = getWidth();
}

void OSD_Window:: calculateHeight() {
	if (winHeight == 0) winHeight = getHeight();
}


void OSD_Window:: calculateSizePosition() {
	calculateWidth();
	calculateHeight();
	if (winCol == 0) /*if (centerHorz) */winCol = (OSD_COLS - winWidth) / 2;
	if (winRow == 0) /*if (centerVert) */winRow = (OSD_ROWS - winHeight) / 2;
	debug_osd("OSD:: window.calculate  col=%d row=%d  w=%d h=%d\n", winCol, winRow, winWidth, winHeight);
}

void OSD_Window:: show() {
	calculateSizePosition();
	printWindow();
	osd->updateUI();
	run();
}

bool OSD_Window:: getExitOk() { return exitOk; }

void OSD_Window:: run() {
	SDL_Event event;
	SDL_Scancode scancode;
	finishRun = false;
	
	#ifdef TARGET_PC
    while (!GLOBAL_QUIT  &&  !finishRun) {
	#else
	while (!finishRun) {
	#endif
		SDL_PollEvent(&event);
		scancode = SDL_GetScancodeFromKey(event.key.keysym.sym);

		switch (event.type) {

			#ifdef TARGET_PC
			case SDL_QUIT:
				GLOBAL_QUIT = true; 
				break;
			#endif

			case SDL_KEYDOWN:
				switch (scancode) {
					case SDL_SCANCODE_UP:        moveUp();     break;
					case SDL_SCANCODE_DOWN:      moveDown();   break;
					case SDL_SCANCODE_HOME:      moveBegin();  break;
					case SDL_SCANCODE_END:       moveEnd();    break;
					case SDL_SCANCODE_PAGEUP:    rePag();      break;
					case SDL_SCANCODE_PAGEDOWN:  advPag();     break;
					case SDL_SCANCODE_F2:        k_F2();       break;
					case SDL_SCANCODE_F3:        k_F3();       break;
					case SDL_SCANCODE_F4:        k_F4();       break;
					case SDL_SCANCODE_F8:        k_F8();       break;
					case SDL_SCANCODE_F9:        k_F9();       break;
					case SDL_SCANCODE_F10:       k_F10();      break;
					case SDL_SCANCODE_RETURN:    k_return();   break;
					case SDL_SCANCODE_ESCAPE:    finish();     break;
					case SDL_SCANCODE_Y:         k_Y();        break;
					case SDL_SCANCODE_N:         k_N();        break;
				}
		}
	}
}

void OSD_Window:: finish() { finishRun = true; }

void OSD_Window:: moveUp() {}
void OSD_Window:: moveDown() {}
void OSD_Window:: rePag() {}
void OSD_Window:: advPag() {}
void OSD_Window:: moveBegin() {}
void OSD_Window:: moveEnd() {}
void OSD_Window:: k_return() {}
void OSD_Window:: k_F2() {}
void OSD_Window:: k_F3() {}
void OSD_Window:: k_F4() {}
void OSD_Window:: k_F8() {}
void OSD_Window:: k_F9() {}
void OSD_Window:: k_F10() {}
void OSD_Window:: k_Y() {}
void OSD_Window:: k_N() {}
void OSD_Window:: k_esc() { finishRun = true; }

//=================================================================================================


OSD_PlainWindow:: OSD_PlainWindow(OSD* _osd, TColor _bg, TColor _fg) : OSD_Window(_osd), bg(_bg), fg(_fg) {}

void OSD_PlainWindow:: appendTextLine(const std::string& text) {
	texts.push_back(text);
}

void OSD_PlainWindow:: printWindow() {
	osd->setColors(bg, fg);
	osd->drawFilledRectangleC(winCol, winRow, winWidth, winHeight); FLUSH

	u8 r = 1;
	for (std::string& text: texts) {
		osd->writeFastTextC(winCol+1, winRow + r++, text);
	}
	osd->drawWindowFrameC(winCol, winRow, winWidth, winHeight, WINDOW_ALERT_BG, WINDOW_SHADOW);
}

u8 OSD_PlainWindow:: getWidth() {
	u8 maxlen = 0;

	for (std::string& text: texts) {
		if (text.size() > maxlen) maxlen = text.size();
	}
	return maxlen+2;
}

u8 OSD_PlainWindow:: getHeight() { return texts.size() + 2; }


//=================================================================================================

OSD_TitledWindow:: OSD_TitledWindow(const std::string& _title, OSD* _osd) :
	OSD_Window(_osd), title(_title) {
		//debug_osd("OSD:: osd_titled_window const\n");
	}


void OSD_TitledWindow:: setSubtitle(const std::string& s) { useSubtitle = true; subtitle = s; }

void OSD_TitledWindow:: setStatus(const std::string& s) { useStatus = true; status = s; }

void OSD_TitledWindow:: setHeight(bool _useSubtitle, u8 _windowBodyRows, bool _useStatus) {
	useSubtitle = _useSubtitle;
	useStatus = _useStatus;
	itemsRows = _windowBodyRows;

	subtitleRow = useSubtitle ? 1 : 0;
	item1row    = subtitleRow + 1; // (useSubtitle ? 1 : 0);
	statusRow   = item1row + itemsRows; //1 + (useSubtitle ? 1 : 0) + itemsRows;

	OSD_Window::setHeight(statusRow+1);
	debug_osd("OSD:: setHeight item1row=%d statusRow=%d\n", item1row, statusRow);
}

u8 OSD_TitledWindow:: getItemRows() { return itemsRows; }


void OSD_TitledWindow:: printSubtitle(const std::string subtitle) {
	//std::string s = alignLeft(subtitle);
	clearSubtitleRow();
	osd->setForeground(WINDOW_SUBTITLE_FG);
	osd->writeFastTextC(winCol+1, winRow+subtitleRow, subtitle);
}

void OSD_TitledWindow:: printStatus(const std::string& status) {
	//std::string s = alignLeft(status);
	clearStatusRow();
	osd->setForeground(WINDOW_STATUS_FG);
	osd->writeFastTextC(winCol+1, winRow+statusRow, status);
}



u8 OSD_TitledWindow:: getHeight() {
	u8 h = getItemRows() + 1;
	item1row = 1;
	if (useSubtitle) {
		subtitleRow = 1;
		h++;
		item1row++;
	}
	if (useStatus) {
		h++;
		statusRow = h-1;
	}
	debug_osd("OSD:: title_window.get_heigth   height=%d item1row=%d\n", h, item1row);
	return h;
}



void OSD_TitledWindow:: printWindow() {
	// dibjua titulos y marco
	centerText(title, winWidth);
	osd->setColors(WINDOW_TITLE_BG, WINDOW_TITLE_FG);
	osd->writeTextC(winCol, winRow, title);

	if (useSubtitle) {
		if (subtitle != "")  printSubtitle(subtitle);
		else clearSubtitleRow();
	}
	if (useStatus) {
		if (status != "")  printStatus(status);
		else clearStatusRow();
	}
	osd->drawWindowFrameC(winCol, winRow, winWidth, winHeight, WINDOW_TITLE_BG, WINDOW_SHADOW);
}



void OSD_TitledWindow:: clearRow(u8 row, TColor color) {
	osd->setBackground(color);
	osd->drawFilledRectangleC(winCol, winRow+row, winWidth, 1);
}

void OSD_TitledWindow:: clearSubtitleRow() {
	clearRow(subtitleRow, WINDOW_SUBTITLE_BG);
}

void OSD_TitledWindow:: clearStatusRow() {
	clearRow(statusRow, WINDOW_SUBTITLE_BG);
}


//=================================================================================================


OSD_Selection:: OSD_Selection(const std::string& _title, OSD* _osd) : OSD_TitledWindow(_title, _osd) {
	debug_osd("OSD:: select const\n");
}

OSD_Selection:: ~OSD_Selection() { 
	//debug_osd("OSD:: ~OSD_Selection\n");
	clearItems(); 
}


bool OSD_Selection:: getExitOk() { return itemSelected != nullptr; }


OSD_SelectionItem* OSD_Selection:: getSelected() {
	OSD_SelectionItem* wi = itemSelected;
	itemSelected = nullptr;
	return wi;
}


void OSD_Selection:: k_return() {
	itemSelected = items.at(cursor);
	debug_osd("OSD:: selected [%s]\n", itemSelected->toString().c_str());
	finish();
}


void OSD_Selection:: addItem(OSD_SelectionItem* item) { items.push_back(item); }


void OSD_Selection:: clearItems() { 
	u16 size = items.size();
	for (auto* item : items) delete item;
	items.clear(); 
}


//=================================================================================================


OSD_LittleSelect:: OSD_LittleSelect(const std::string& _title, OSD* _osd) : OSD_Selection(_title, _osd) {
	//debug_osd("OSD:: little_select const. cursorAnt=%d\n", cursorAnt);
}

u8 OSD_LittleSelect:: getWidth() {
	size_t sz = title.size();
	//debug(" sz=%d  items=%d \n ", sz, items.size());
	for (u8 i = 0; i < items.size(); i++) {
		OSD_SelectionItem* it = items.at(i);
		sz = std::max(items[i]->toString().size(), sz);
	}
	if (useStatus  &&  status != "")  sz = std::max(sz, status.size());
	if (useSubtitle  &&  subtitle != "")  sz = std::max(sz, subtitle.size());
	return sz + 2; // +2 por el caracter extra a izq y der
}


u8 OSD_LittleSelect:: getItemRows() { return items.size(); }


void OSD_LittleSelect:: printWindow() {
	OSD_TitledWindow::printWindow();
	debug_osd("OSD:: OSD_LittleSelect:: printWindow()  items=%d\n", items.size());
	for (u8 i = 0; i < items.size(); i++)  writeItem(i, cursor == i);
	debug_osd("OSD:: OSD_LittleSelect:: printWindow()  items=%d\n", items.size());
}



void OSD_LittleSelect:: writeItem(u8 nItem, bool selected) {
	if (selected)
		osd->setColors(WINDOW_SELECTED_ITEM_BG, WINDOW_SELECTED_ITEM_FG);
	else
		osd->setColors(WINDOW_ITEM_BG, WINDOW_ITEM_FG);
	//debug("OSD:: little_select writeItem  %d %d %d\n", winRow, item1row, nItem);
	osd->drawFilledRectangleC(winCol, winRow + item1row + nItem, winWidth, 1);
	osd->writeFastTextC(winCol+1, winRow + item1row + nItem, items.at(nItem)->toString());
}


void OSD_LittleSelect:: moveUp()    {
	if (cursor > 0) cursor--;
	updateCursorSelection();
}

void OSD_LittleSelect:: moveDown()  {
	if (cursor < items.size()-1) cursor++;
	updateCursorSelection();
}

void OSD_LittleSelect:: rePag()     {
	cursor = 0;
	updateCursorSelection();
}

void OSD_LittleSelect:: advPag()    {
	cursor = items.size() - 1;
	updateCursorSelection();
}

void OSD_LittleSelect:: moveBegin() {
	cursor = 0;
	updateCursorSelection();
}

void OSD_LittleSelect:: moveEnd()   {
	cursor = items.size() - 1;
	updateCursorSelection();
}

void OSD_LittleSelect:: updateCursorSelection() {
	if (cursorAnt != cursor) {
		writeItem(cursorAnt, false);
		writeItem(cursor, true);
		osd->updateUI();
		cursorAnt = cursor;
	}
}


//=================================================================================================


OSD_BigSelect:: OSD_BigSelect(const std::string& _title, OSD* _osd) : OSD_Selection(_title, _osd) {
	debug_osd("OSD:: big_select const\n");
}



u8 OSD_BigSelect:: getWidth() {
	return (winWidth == 0) ? OSD_COLS-10 : winWidth; // anchura por defecto
}

u8 OSD_BigSelect:: getHeight() {
	return (winHeight == 0) ? OSD_ROWS-6 : winHeight; // altura por defecto
}



u8 OSD_BigSelect:: getItemRows() {
	return itemsRows;
}

void OSD_BigSelect:: clearItemRow(u8 itemRow) {
	osd->drawFilledRectangleC(winCol, winRow + item1row + itemRow, winWidth, 1);
}


void OSD_BigSelect:: writeItem(bool selected) {
	if (selected)
		osd->setColors(WINDOW_SELECTED_ITEM_BG, WINDOW_SELECTED_ITEM_FG);
	else
		osd->setColors(WINDOW_ITEM_BG, WINDOW_ITEM_FG);

	u16 row = winRow + item1row + (cursor - firstInWindow);
	osd->drawFilledRectangleC(winCol,   row, winWidth, 1);
	osd->writeFastTextC      (winCol+1, row, items.at(cursor)->toString());
}


void OSD_BigSelect:: printItemsPage() {
	u16 totalFiles = items.size();
	std::string fichero;
	i32 cursorCopy = cursor;
	cursor = firstInWindow;

	for (u8 contadorFilas = 0; 
			contadorFilas < itemsRows  &&  cursor < totalFiles; 
			contadorFilas++, cursor++)
	{
		writeItem(cursor == cursorCopy);
	}
	cursor = cursorCopy;
}


void OSD_BigSelect:: clearRestPage() {
	if (items.size() < itemsRows) {
		osd->setBackground(WINDOW_ITEM_BG);
		osd->drawFilledRectangleC(winCol, winRow + item1row + items.size(), winWidth, itemsRows - items.size());
	}
}



void OSD_BigSelect:: moveUp() {
	if (cursor > 0) {
		if (cursor - firstInWindow > 0) {
			writeItem(false);
			cursor--;
			writeItem(true);
		}
		else { // scroll
			cursor--;
			firstInWindow--;
			printItemsPage();
		}
		printCursorPosition();
		osd->updateUI();
	}
}


void OSD_BigSelect:: moveDown() {
	if (cursor < items.size()-1) {
		if (cursor - firstInWindow < itemsRows - 1) {
			writeItem(false);
			cursor++;
			writeItem(true);
		}
		else { // scroll
			cursor++;
			firstInWindow++;
			printItemsPage();
		}
		printCursorPosition();
		osd->updateUI();
	}
}


void OSD_BigSelect:: rePag() {
	if (firstInWindow == 0) { // 1a pagina
		writeItem(false);
		cursor = 0;
		writeItem(true);
	}
	else { // cambiar pagina
		//debug_osd("OSD:: first_in_window=%d  cursor=%d  itemRows=%d\n", firstInWindow, cursor, itemRows);
		firstInWindow -= itemsRows;
		if (firstInWindow < 0) firstInWindow = 0;
		cursor -= itemsRows;
		if (cursor < 0) cursor = 0;
		//debug_osd("OSD:: first_in_window=%d  cursor=%d  itemRows=%d\n", firstInWindow, cursor, itemRows);
		printItemsPage();
	}
	printCursorPosition();
	osd->updateUI();
}


void OSD_BigSelect:: advPag() {
	u16 totalItems = items.size();
	u16 totalItems_1 = totalItems - 1;

	if (cursor < totalItems_1) { // hay que mover
		if (totalItems > itemsRows) { // caso general
			cursor += itemsRows;
			if (cursor >= totalItems) 
				cursor = totalItems_1;
			firstInWindow += itemsRows;
			if (firstInWindow >= totalItems - itemsRows) 
				firstInWindow = totalItems - itemsRows;
			printItemsPage();
		}
		else { // cambiar cursor en la misma pagina (solo hay 1)
			writeItem(false);
			cursor = totalItems_1;
			writeItem(true);
		}
		printCursorPosition();
		osd->updateUI();
	}
}


void OSD_BigSelect:: moveBegin() {
	if (firstInWindow == 0) {
		if (cursor != 0) {
			writeItem(false);
			cursor = 0;
			writeItem(true);
			printCursorPosition();
			osd->updateUI();
		}
	}
	else { // caso general
		firstInWindow = 0;
		cursor = 0;
		printItemsPage();
		printCursorPosition();
		osd->updateUI();
	}
}


void OSD_BigSelect:: moveEnd() {
	u16 totalItems = items.size();
	u16 totalItems_1 = totalItems - 1;

	if (cursor < totalItems_1) { // hay que mover
		if (totalItems > itemsRows) { // caso general
			cursor = totalItems_1;
			firstInWindow = totalItems - itemsRows;
			printItemsPage();
		}
		else {
			writeItem(false);
			//firstInWindow = 0;
			cursor = totalItems_1;
			writeItem(true);
		}
		printCursorPosition();
		osd->updateUI();
	}
}



void OSD_BigSelect:: printCursorPosition() {
	std::string s = std::to_string(cursor+1) + "/" + std::to_string(items.size());
	osd->setColors(WINDOW_STATUS_BG, WINDOW_STATUS_FG);
	s.insert(0, 9 - s.size(), ' ');
	osd->drawFilledRectangleC(winCol+winWidth-10, winRow+statusRow, 9, 1);
	osd->writeFastTextC(winCol+winWidth-10, winRow+statusRow, s);
}


//=================================================================================================


OSD_SimpleSelect:: OSD_SimpleSelect(const std::string& _title, OSD* _osd) : OSD_LittleSelect(_title, _osd) {
	//debug_osd("OSD:: simple_select const. \n");
}



//=================================================================================================

OSD_YesNoQuestionWindow:: OSD_YesNoQuestionWindow(OSD* osd) : OSD_PlainWindow(osd, WINDOW_ALERT_BG, WINDOW_ALERT_FG) {}

void OSD_YesNoQuestionWindow:: k_N() { finish(); };
void OSD_YesNoQuestionWindow:: k_Y() { exitOk = true; finish(); }


//=================================================================================================

OSD_AlertWindow:: OSD_AlertWindow(OSD* osd) : OSD_PlainWindow(osd, WINDOW_ALERT_BG, WINDOW_ALERT_FG) {}


