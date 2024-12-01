/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD window option selection                                               |
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
#include "util.h"
#include "../tipos.h"
#include "../monitor.h"  // para OSD_COLS y OSD_ROWS
#include "../log.h"



OSD_IntStringOption:: OSD_IntStringOption(u8 id, const std::string& desc) : 
	optionId(id), description(desc) {}


std::string OSD_IntStringOption:: toString() { return description; }



void OSD_SimpleSelect:: addOption(u8 id, const std::string& desc) {
	OSD_IntStringOption* o = new OSD_IntStringOption(id, desc);
	addItem(o);
}


u8 OSD_SimpleSelect:: getValueSelected() {
	OSD_SelectionItem* wi = getSelected();
	OSD_IntStringOption* o = reinterpret_cast<OSD_IntStringOption*>(wi);
	return o->optionId;
}


