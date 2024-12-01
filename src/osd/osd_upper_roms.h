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


#pragma once

#include "osd_window.h"
#include "../tipos.h"



class OSD_UpperRomItem : public OSD_SelectionItem
{
public:
	u8 slot;
	std::string file;
	bool loaded;

	OSD_UpperRomItem(u8 slot, const std::string& _file, bool _loaded);
	std::string toString() override;
	bool isLoaded();
	void setLoaded(bool l);
	std::string& getFile();
	void setFile(const std::string romFile);
};


class OSD_UpperRomSelection : public OSD_BigSelect
{
public:
	OSD_UpperRomSelection(const std::string title, OSD* osd);

	bool getExitOk() override;
	void setUpperRom(u8 slot, const std::string& romFile, bool loaded);
	std::string& getUpperRom(u8 slot);

private:
	bool exitF10 = false;
	
	void printWindow() override;
	void writeItem(bool selected) override;
	void printHelp();

	void k_return() override;  // change rom
	void k_F8()     override;  // remove rom
	void k_F10()    override;
};

