/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Printer implementation                                                    |
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

// la idea es generar una imagen


#include "tipos.h"
#include "io_device.h"
#include "printer.h"
#include "log.h"


Printer:: Printer() {
    debug("Printer\n");
    reset();
}


void Printer:: reset() {

}


bool Printer:: OUT(WORD puerto, BYTE dato) {
    if (puerto.b.h == 0xE7) {
        return true;
    }
    return false;
}


bool Printer:: IN(WORD puerto, BYTE* dato) {
    if (puerto.b.h == 0xE7) {
        return true;
    }
    return false;
}
