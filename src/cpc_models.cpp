/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CPC models and its rom files                                              |
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
#include "cpc_models.h"
#include "tipos.h"
#include "cpc.h"
#include "target.h"


//const std::string CPC:: CPC_TYPES[] {"CPC464", "CPC664", "CPC6128"};


const CPC_MODEL CPC:: STANDARD_MODELS[] = {
	{ CPC464, "CPC464 English", "OS_464_UK.rom", "BASIC_464_UK.rom", ""},
	{ CPC464, "CPC464 Spanish", "OS_464_ES.rom", "BASIC_464_UK.rom", ""},
	{ CPC464, "CPC464 Danish",  "OS_464_DK.rom", "BASIC_464_UK.rom", ""},
	{ CPC464, "CPC464 French",  "OS_464_FR.rom", "BASIC_464_UK.rom", ""},

	{ CPC664, "CPC664 English", "OS_664_UK.rom", "BASIC_664_UK.rom", "AMSDOS.rom"},

	{ CPC6128, "CPC6128 English", "OS_6128_UK.rom", "BASIC_6128_UK.rom", "AMSDOS.rom"},
	{ CPC6128, "CPC6128 Spanish", "OS_6128_ES.rom", "BASIC_6128_ES.rom", "AMSDOS.rom"},
	{ CPC6128, "CPC6128 Danish",  "OS_6128_DK.rom", "BASIC_6128_DK.rom", "AMSDOS.rom"},
	{ CPC6128, "CPC6128 French",  "OS_6128_FR.rom", "BASIC_6128_FR.rom", "AMSDOS.rom"},
};


const std::string CPC:: CPC_BASE[] = { "CPC464", "CPC664", "CPC6128" };


#ifdef TARGET_PC
void CPC:: print_models() {
	printf("Values for -v argument:\n");
	for (u8 x = 0; x < CPC_MODELS_COUNT; x++) {
		printf("\t%d: %s\n", x, STANDARD_MODELS[x].description.c_str());
	}
}
#endif
