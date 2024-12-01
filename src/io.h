/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Generic input/output implementation                                       |
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

#include "tipos.h"
#include "io.h"
#include "io_device.h"



class IO_Bus
{
    
    IO_Device* dispositivos[10];
    u8 numDispositivos = 0;

public:
	static constexpr u8 MAX_DISPOSITIVOS = 5;

    IO_Bus();
    
    void reset();

    bool addDevice(IO_Device* d);
    bool removeDevice(IO_Device* d);
    
    void OUT(WORD puerto, BYTE dato); // la cpu escribe en el bus
    
    BYTE IN(WORD puerto); // el z80 lee del puerto

    void update(u8 ciclos);

};
