/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Standard disks creation                                                   |
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


#include <fstream>

static void createStandardData(const std::string& f); // 40 pistas, 9 sectores, 1 cara
static void createStandardSystem(const std::string& f); // 40 pistas, 9 sectores, 1 cara
static void createStandardUnformatted(const std::string& f); // 40 pistas, 9 sectores, 1 cara

static void create35Data(const std::string& f); // 80 pistas, 2 caras, 9 sectores, 512k
static void create35Unformatted(const std::string& f); // 80 pistas, 2 caras, 9 sectores, 512k


typedef struct {
	std::string description;
	u8 tracks;
	u8 sides;
	u8 sectors;
	u8 size;     // 1 = 256; 2 = 512;  real size = 0x40 << size
	BYTE fillerByte;
	BYTE gap;
	BYTE firstSector;
	//u8 info;	// 1 solo amsdos, 2 parados, 4 romdos
} DSK_FORMAT;


constexpr u8 NUMBER_DSK_FORMATS = 3;

extern const DSK_FORMAT DSK_FORMATS[NUMBER_DSK_FORMATS];

