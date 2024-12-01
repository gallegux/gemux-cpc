/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CPC Keyboard                                                              |
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

#include <unordered_map>
#include "tipos.h"


constexpr u8 KEYBOARD_ROWS = 10;
constexpr u8 KEYS_PER_ROW  =  8;


class Keyboard 
{
    u8 selectedRow = 0;
    u8 keyRows[10];

    BYTE keyStatus[KEYBOARD_ROWS][KEYS_PER_ROW];
    
public:
    
    Keyboard();
    //~Keyboard();

    void selectRow(u8 fila);
	u8 getSelectedRow();  // devuelve el numero de fila seleccionada

    BYTE getRowStatus(u8 fila);
    BYTE getSelectedRowStatus(); // row status de la fila seleccionada

    void setKeyStatus(u16 tecla, bool estado);
};
