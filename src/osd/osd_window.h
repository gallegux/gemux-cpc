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


#pragma once


#include <string>
#include <vector>
#include "osd.h"
#include "../tipos.h"
#include "../log.h"



constexpr TColor WINDOW_ALERT_BG = {250,0,0};
constexpr TColor WINDOW_ALERT_FG = {250,250,250};
constexpr TColor WINDOW_TITLE_BG = {200,50,100};
constexpr TColor WINDOW_TITLE_FG = {250,250,250};
constexpr TColor WINDOW_SUBTITLE_BG = {138,138,138};
constexpr TColor WINDOW_SUBTITLE_FG = {0,0,0};
constexpr TColor WINDOW_ITEM_BG = {240,240,240};
constexpr TColor WINDOW_ITEM_FG = {10,10,10};
constexpr TColor WINDOW_SELECTED_ITEM_BG = {20,200,240};
constexpr TColor WINDOW_SELECTED_ITEM_FG = {0,0,0};
constexpr TColor WINDOW_STATUS_BG = {138,138,138};
constexpr TColor WINDOW_STATUS_FG = {0,0,0};
constexpr TColor WINDOW_SHADOW = {0,0,0};
constexpr TColor WINDOW_INPUT_BG = {255,255,128};
constexpr TColor WINDOW_INPUT_FG = {0,0,0};
constexpr TColor ROM_NOT_LOADED = {128,0,0};
constexpr u8 SEARCH_LEN = 33;
constexpr u8 INPUT_NEW_FILE_LEN = 35;
constexpr u8 NUM_POKES = 16;
constexpr u8 WINDOW_MAX_WIDTH = 58;
constexpr u8 WINDOW_MAX_HEIGHT = 33;


class OSD_Window 
{
protected:
	OSD* osd;
	u8 winCol = 0;
	u8 winRow = 0;
	u8 winWidth = 0;
	u8 winHeight = 0;
	bool finishRun;
	bool exitOk = false;

public:
	OSD_Window(OSD* osd);

	void setWidth(u8 height);
	void setHeight(u8 height);
	void getBounds(u8* col, u8* row, u8* width, u8* height) const;
	void show();
	virtual bool getExitOk();
	virtual void calculateSizePosition();
	virtual u8 getWidth() = 0;
	virtual u8 getHeight() = 0;

protected:
	void calculateWidth();
	void calculateHeight();
	virtual void printWindow() = 0;   // primer pintado de la ventana

	virtual void run();
	void finish();

	virtual void moveUp();
	virtual void moveDown();
	virtual void rePag();
	virtual void advPag();
	virtual void moveBegin();
	virtual void moveEnd();

	virtual void k_esc();
	virtual void k_return();
	virtual void k_F2();
	virtual void k_F3();
	virtual void k_F4();
	virtual void k_F8();
	virtual void k_F9();
	virtual void k_F10();

	virtual void k_Y();
	virtual void k_N();
};



class OSD_PlainWindow : public OSD_Window
{
private:
	TColor bg, fg;
	std::vector<std::string> texts;

public:
	OSD_PlainWindow(OSD* osd, TColor bg, TColor fg);
	void appendTextLine(const std::string& text);

	u8 getWidth() override;
	u8 getHeight() override;
	void printWindow() override;
};



class OSD_YesNoQuestionWindow : public OSD_PlainWindow
{
public:
	OSD_YesNoQuestionWindow(OSD* osd);
	void k_N() override;
	void k_Y() override;
};


class OSD_AlertWindow : public OSD_PlainWindow
{
public:
	OSD_AlertWindow(OSD* osd);
};




class OSD_TitledWindow : public OSD_Window
{
protected:
	u8 subtitleRow;  // fila relativa para el subtitulo
	u8 statusRow; // fila relativa para la barra de estado/ayuda/etc
	u8 item1row;  // fila relativa para el contenido de la ventana
	u8 itemsRows = 0; // = bodyRows
	std::string title;
	std::string subtitle;
	std::string status;
	bool useSubtitle = false;
	bool useStatus = false;
	bool sizeCalculated = false;

	OSD_TitledWindow(const std::string& _title, OSD* _osd);
	
	void printSubtitle(const std::string subtitle);
	void printStatus(const std::string& status);
	void clearSubtitleRow();
	void clearStatusRow();
	void clearRow(u8 row, TColor color);

	virtual u8 getItemRows();   // = getBodyRows
	u8 getHeight() override;
	void printWindow();   // primer pintado de la ventana

public:
	void setSubtitle(const std::string& text);
	void setStatus(const std::string& text);
	void setHeight(bool useSubtitle, u8 windowBodyRows, bool useStatus); // para que se ajuste segun el nº de elementos -> itemsRows=0
};





class OSD_SelectionItem
{
public:
	virtual std::string toString() = 0;
};




class OSD_Selection : public OSD_TitledWindow
{
public:
	bool getExitOk() override;

protected:

	i32 cursor = 0;
	std::vector<OSD_SelectionItem*> items;
	OSD_SelectionItem* itemSelected = nullptr;

	OSD_Selection(const std::string& _title, OSD* _osd); 
	~OSD_Selection();

	void addItem(OSD_SelectionItem* item);
	void clearItems(); // libera recursos usados por los items

	OSD_SelectionItem* getSelected();

	virtual void k_return();
};



class OSD_BigSelect : public OSD_Selection
{

protected:
	i16 firstInWindow = 0;

	OSD_BigSelect(const std::string& _title, OSD* _osd); // : OSD_Selection(_title, _osd) {}
	u8 getWidth() override;
	u8 getHeight() override;

	//void printWindow() override;
	void moveUp()    override;
	void moveDown()  override;
	void rePag()     override;
	void advPag()    override;
	void moveBegin() override;
	void moveEnd()   override;

	virtual u8 getItemRows();
	void clearItemRow(u8 itemRow);
	virtual void writeItem(bool selected);
	//virtual void writeCursorItem(bool selected, u16 cursor, u8 cursorRow);
	void printCursorPosition();  // escribe en la barra de estado el nº de fichero/total
	void printItemsPage(); // escribe una pagina entera a partir de firstInWindow
	void clearRestPage();  // borra el contenido de la pagina que no se utiliza
};




class OSD_LittleSelect : public OSD_Selection
{
protected:
	u8 cursorAnt = 0;

	void writeItem(u8 cursor, bool selected);

	void moveUp()    override;
	void moveDown()  override;
	void rePag()     override;
	void advPag()    override;
	void moveBegin() override;
	void moveEnd()   override;
	void updateCursorSelection();

	void printWindow()   override;
	u8 getItemRows() override;
	u8 getWidth() override;
	
public:
	OSD_LittleSelect(const std::string& _title, OSD* _osd); 

};


//----------------------------------------------------------------------

class OSD_IntStringOption : public OSD_SelectionItem
{
public:
	std::string description;
	int optionId;

	OSD_IntStringOption(u8 id, const std::string& desc);
	std::string toString() override;
};


class OSD_SimpleSelect : public OSD_LittleSelect
{
protected:
	//u8 getWidth() override;
public:
	OSD_SimpleSelect(const std::string& _title, OSD* _osd);  // : OSD_LittleSelect(_title, _osd) {}

	void addOption(u8 id, const std::string& desc);
	u8 getValueSelected();
};

//----------------------------------------------------------------------------
