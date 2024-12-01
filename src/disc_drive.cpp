/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Disc drive implementation                                                 |
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
#include <filesystem>
#include "log.h"
#include "dsk.h"
#include "disc_drive.h"
#include "fdc.h"



DiscDrive::DiscDrive(u8 _unidad, FDC* _fdc) { 
    this->unidad = _unidad; 
    this->fdc = _fdc; 
    this->disco = nullptr;

    debug_disquetera("DRIVE:: Unidad de disco %d\n", this->unidad);
}


DiscDrive::~DiscDrive() {
    debug_disquetera("DRIVE:: ~DiscDrive(%d)\n", unidad);
    eject();
}


//bool DiscDrive:: insert(std::filesystem::path& fichero) {
//	std::string d { fichero.string() };
//	return insert(d);
//}


bool DiscDrive:: insert(std::string& fichero) {
    eject();
    
    disco = new DSK;
    bool ok = disco->open(fichero);
    if (ok) {
        fdc->setDisk(unidad, disco);
        debug_disquetera("DRIVE:: [%d] insert: %s\n", unidad, fichero.c_str());

        //osd_setMessage("Drive " + UNIDADES[unidad] + ": " + fichero + " inserted");
    }
    else {
        delete disco;
        debug_disquetera("DRIVE:: [%d] insert -> fichero no valido\n", unidad);
        //osd_setMessage("Drive " + UNIDADES[unidad] + ": Not valid file");
    }
    return ok;
}


void DiscDrive:: eject() {
    if (disco != nullptr) {
        debug_disquetera("DRIVE:: [%d] - eject\n", unidad);
        //osd_setMessage("Drive " + UNIDADES[unidad] + ": Ejected");
        fdc->setDisk(unidad, nullptr);
        disco->close();
        delete disco;
        disco = nullptr;
    }
}


void DiscDrive:: setWriteProtection(bool p) {
    if (disco != nullptr) {
        disco->setProtected(p);
        debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, disco->isProtected());
        //osd_setMessage("Drive " + UNIDADES[unidad] + ": " + PROTECCION[disco->isProtected()]);
    }
}

bool DiscDrive:: getWriteProtection() {
    bool p = (disco != nullptr) ? disco->isProtected() : false;
    debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, p);
    return p;
}

bool DiscDrive:: flipWriteProtection() {
    debug_disquetera("DRIVE:: [%d] cambiarProteccionEscritura()\n", unidad);
    if (disco != nullptr) {
        disco->flipProtected();
        debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, disco->isProtected());
        return disco->isProtected();
    }
    else return false;
}

bool DiscDrive:: hasDSK() {
    return disco != nullptr;
}
