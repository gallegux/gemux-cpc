/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD window file selection                                                 |
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
#include "osd_window.h"
#include "../tipos.h"
#include "../target.h"


class OSD_FileItem : public OSD_SelectionItem
{
public:
	std::string description;
	bool isDir;

	OSD_FileItem(const std::string& desc, bool dir);
	std::string toString() override;
};

class OSD_FileSelection : public OSD_BigSelect
{
public:
	OSD_FileSelection(const std::string& title, OSD* osd, std::filesystem::path dir, std::string_view ext);
	void setCreateOption(bool c);
	//void setFunctionCreate( bool (*_functionCreate)(const std::string& f) );
	std::filesystem::path getSelectedFile();
	std::filesystem::path getCurrentDir();
	//bool getExitOk() override;

protected:
	void printWindow();
	bool createOption;
	std::filesystem::path dir;
	std::string ext;
	virtual void printHelp();

private:
	std::string filter = "";
	//bool (*functionCreate)(const std::string& f) = nullptr;
	u16  insertNewFile(const std::filesystem::path& filename); // true si se creo
	void goToCursor();
	bool createFile(const std::filesystem::path& filename); // true si se creo
	void printPathDir();
	void writeItem(bool selected);
	void getFiles();
	//void pre_run() override;
	void k_return() override;
	void k_F2() override;
	void k_F3() override;
	void k_F8() override;
	#ifdef TARGET_WINDOWS
	void k_F9() override;
	#endif
	u16 getInsertPosition(OSD_FileItem* fi);
};



class OSD_SaveFile : public OSD_FileSelection
{
public:
	OSD_SaveFile(const std::string& title, OSD* osd, std::filesystem::path dir, std::string_view ext);
	bool getExitOk() override;
	std::filesystem::path getNewFile();

protected:
	void printHelp() override;

private:
	std::filesystem::path newFile;

	void k_F2() override;
	void k_F4() override;
};

