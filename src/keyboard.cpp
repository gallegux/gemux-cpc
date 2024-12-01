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


#include <stdio.h>
#include <unordered_map>
#include "tipos.h"
#include "keyboard.h"
#include "log.h"


u8 BIT_TECLA[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


u8 getNumTeclaEnFila(u16 b) {
    switch (b) {
        case 0x80:  return 7;
        case 0x40:  return 6;
        case 0x20:  return 5;
        case 0x10:  return 4;
        case 0x08:  return 3;
        case 0x04:  return 2;
        case 0x02:  return 1;
        default:    return 0;
    }
    return 0;
}


Keyboard::Keyboard() {
    //debug_kb("Keyboard\n");

    for (u8 y = 0; y < KEYBOARD_ROWS; y++)
        for (u8 x = 0; x < KEYS_PER_ROW; x++)
            keyStatus[y][x] = false;
}


void Keyboard:: selectRow(u8 fila) {
    debug_kb("KEYB::  select_row=%d\n", fila);
    selectedRow = fila & 0x0F;
}


u8 Keyboard:: getSelectedRow() {
	return selectedRow;
}


void Keyboard:: setKeyStatus(u16 teclaCPC, bool estado) {
        //debug_kb("Keyboard::  set_Key_Status %04X,%d\n", teclaCPC, estado);
    //try {
        //u16 teclaCPC = tablaTeclado_ES[codigoTeclaSDL];
        u16 numFila = (teclaCPC & 0x0F00) >> 8;
        u16 bitTecla = teclaCPC & 0x00FF;
        u8 numTecla = getNumTeclaEnFila(bitTecla);
        keyStatus[numFila][numTecla] = (estado) ? bitTecla : 0;  // en estado guardamos su bit a 0 1
        debug_kb("KEYB:: set_key_status   tecla_cpc=%04X=%d  fila=%d maskTecla=%d numTecla=%d\n", 
			teclaCPC, estado,  numFila, bitTecla, numTecla);
    //}
    //catch (const std::out_of_range& ex) {
    //    // Manejar la excepción si la clave no existe en el mapa
    //}
}

BYTE Keyboard:: getRowStatus(u8 fila) {
    BYTE v = 0;

    for (u8 i = 0; i < KEYS_PER_ROW; i++) {
        v |= keyStatus[fila][i];
    }

    if (v!=0) debug_kb("KEYB::  getRowStatus(%d)=%02X  pulsada=%d\n", fila, (BYTE) ~v, v!=0);
    return  ~v;
}


BYTE Keyboard:: getSelectedRowStatus() {
	return getRowStatus(selectedRow);
//    BYTE v = 0;
//
//    for (u8 i = 0; i < TECLADO_TECLAS_FILA; i++) {
//        v |= estadoTeclas[filaSeleccionada][i];
//    }
//
//    //debug_teclado("KEYB::  getRowStatus()=%02X  pulsada=%d\n", (BYTE) ~v, v!=0);
//    return  ~v;
}


