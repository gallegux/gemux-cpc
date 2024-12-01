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


#include <string>
#include <fstream>
#include "dsk.h"
#include "standard_disks.h"



static void createStandardData(const std::string& f) {; // 40 pistas, 9 sectores, 1 cara
	DSK::create(f, 40, 1, 9, 2, 0xE5, 0x4E, 0xC1);
}


static void createStandardSystem(const std::string& f) {; // 40 pistas, 9 sectores, 1 cara
	DSK::create(f, 40, 1, 9, 2, 0xE5, 0x4E, 0x41);
}


static void create35Data(const std::string& f) {; // 80 pistas, 9 sectores, 2 caras
	DSK::create(f, 80, 2, 9, 2, 0xE5, 0x4E, 0xC1);
}


static void create35System(const std::string& f) { // 80 pistas, 9 sectores, 2 caras
	DSK::create(f, 80, 2, 9, 2, 0xE5, 0x4E, 0x41);
}


const DSK_FORMAT DSK_FORMATS[NUMBER_DSK_FORMATS] = {
	{ "3\" data format (178 KB)"   , 40, 1, 9, 2, 0xE5, 0x4E, 0xC1},
	{ "3\" system format (169 KB)" , 40, 1, 9, 2, 0xE5, 0x4E, 0xC1},
	{ "3\" unformatted"            , 40, 1, 9, 2, 0, 0, 0}
//	{ "3.5\" single side (716 Kb)" , 80, 1, 9, 2, 0xE5, 0x4E, 0xC1},
//	{ "3.5\" double side (356 Kb)" , 80, 2, 9, 2, 0xE5, 0x4E, 0xC1}
};
