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
#include "log.h"
#include "dsk.h"
#include "disquetera.h"
#include "fdc.h"



UnidadDisco::UnidadDisco(u8 _unidad, FDC* _fdc) { 
    this->unidad = _unidad; 
    this->fdc = _fdc; 
    this->disco = nullptr;

    debug_disquetera("DRIVE:: Unidad de disco %d\n", this->unidad);
}


UnidadDisco::~UnidadDisco() {
    debug_disquetera("DRIVE:: ~UnidadDisco(%d)\n", unidad);
    expulsar();
}


bool UnidadDisco:: insertar(std::string& fichero) {
    expulsar();
    
    disco = new DSK;
    bool ok = disco->open(fichero);
    if (ok) {
        fdc->setDisk(unidad, disco);
        debug_disquetera("DRIVE:: [%d] insertar: %s\n", unidad, fichero.c_str());
    }
    else {
        delete disco;
        debug_disquetera("DRIVE:: [%d] insertar -> fichero no valido\n", unidad);
    }
    return ok;
}


void UnidadDisco:: expulsar() {
    if (disco != nullptr) {
        debug_disquetera("DRIVE:: [%d] - expulsar\n", unidad);

        fdc->setDisk(unidad, nullptr);
        disco->close();
        delete disco;
        disco = nullptr;
    }
}


void UnidadDisco:: setProteccionEscritura(bool p) {
    if (disco != nullptr) {
        disco->setProtected(p);
        debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, disco->isProtected());
    }
}

bool UnidadDisco:: getProteccionEscritura() {
    bool p = (disco != nullptr) ? disco->isProtected() : false;
    debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, p);
    return p;
}

bool UnidadDisco:: cambiarProteccionEscritura() {
    debug_disquetera("DRIVE:: [%d] cambiarProteccionEscritura()\n", unidad);
    if (disco != nullptr) {
        disco->flipProtected();
        debug_disquetera("DRIVE:: [%d] proteccion = %d\n", unidad, disco->isProtected());
        return disco->isProtected();
    }
    else return false;
}

bool UnidadDisco:: hayDisco() {
    return disco != nullptr;
}
