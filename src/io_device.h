/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Generic device implementation                                                        |
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
#include "compilation_options.h"


class IO_Device
{
    
public:

    // devuelve true si el puerto corresponde al dispositivo
    virtual bool OUT(WORD puerto, BYTE dato) = 0;
    
    // si el puerto corresponde al dispositivo devuelve true, y escribe en *dato
    // si el puerto no corresponde devuelve false
    virtual bool IN(WORD puerto, BYTE* dato) = 0;

    virtual void reset() = 0;

    // actualizar el dispositivo, se envian los ciclos de la cpu que han transcurrido
    virtual void update(u8 ciclos) = 0; 

};


class IAttendedInterruptReceiver
{
public:
    virtual void notifyInterruptAttended() = 0;
};


#ifdef debug_ints_borde
class IDisplayInterrupt
{
public:
    virtual void startInt() = 0;
    virtual void endInt() = 0;
};
#endif
