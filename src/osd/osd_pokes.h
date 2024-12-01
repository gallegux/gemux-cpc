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


#pragma once

#include "osd_window.h"
#include "../tipos.h"


class OSD_PokeItem : public OSD_SelectionItem
{
public:
	u8 number;
	u16 address = 0;
	u8 value = 0;

	OSD_PokeItem(u8 _number);

	void setAddress(const std::string& _address);
	void setValue(const std::string& _value);

	std::string toString() override;
};


class OSD_PokeSelection : public OSD_LittleSelect
{
public:
	OSD_PokeSelection(const std::string title, OSD* osd);
	u8 getNumPokes();
	void getPoke(u8 numPoke, u16* address, BYTE* value);
	bool getExitOk() override;

protected:
	void k_return()  override; // change poke
	void k_F8()      override;    // removePoke();
	void k_F10()     override;    // aceptar();

private:
	bool exitF10 = false;

};

