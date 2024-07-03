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

#pragma once

#include <string>
#include "log.h"
#include "fdc.h"
#include "dsk.h"


// clase orientada a la interfaz de usuario

class UnidadDisco
{
    u8 unidad = 255;
    DSK* disco;
    FDC* fdc;

public:

    UnidadDisco(u8 unidad, FDC* fdc);
    ~UnidadDisco();

    bool insertar(std::string& fichero);
    void expulsar();
    void setProteccionEscritura(bool p);
    bool getProteccionEscritura();
    bool cambiarProteccionEscritura();
    bool hayDisco();
    DSK* getDisco();
};

