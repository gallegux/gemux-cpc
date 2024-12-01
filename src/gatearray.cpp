/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Gatearry implementation                                                   |
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
#include "log.h"
#include "tipos.h"
#include "gatearray.h"
#include "monitor.h"
#include "z80.h"
#include "compilation_options.h"



GateArray:: GateArray (Z80* cpu, Memory* memoria, Monitor* monitor) {
    //debug_ga("GateArray\n");
    this->cpu = cpu;
    this->memoria = memoria;
    this->monitor = monitor;

    reset();
}

void GateArray:: reset() {
    screenMode = 0x01;
    memoria->setEnableLowerRom(true);
    r52 = 0x0;
    vsync = false;

    ramConfiguration = 0;
}

void GateArray:: update(u8 ciclos) {}

//void GateArray:: setMonitor(Monitor* m) {
//    monitor = m;
//}


void GateArray:: notifyInterruptAttended() {
    //debug_ga("GA:: notificacion interrupcion atendida\n");
    reset_R52_bit5();
}


// 7F
bool GateArray:: OUT(WORD puerto, BYTE dato) {
    u8 tipodato = dato & 0b11000000;

    if (tipodato != 0xC0  &&  puerto.b.h == 0x7F) {
        switch (tipodato) {
            case 0b00000000: { // 7FCx seleccionar tinta, 7f00,7f10
                // Select pen
                tintaSeleccionada = (dato & 0x10) ? 16 : dato & 0x0F; //16=borde
                //debug_ga("GA. tinta_seleccionada=%d\n", tintaSeleccionada);
                return true;
            }
            case 0b01000000: { // 7F4x asignar color a tinta
                // dato & 0x1F es un color hardware
                // Select colour for selected pen
                coloresHardware[tintaSeleccionada] = 0x1F & dato;
                //cambioColor[tintaSeleccionada] = 0x1F & dato; por si se ac
#ifdef debug_ints_borde
                if (tintaSeleccionada == HW_BORDER) colorBordeInt = 0x1F & dato;
#endif
                //debug_ga("GA. tinta=%d color=%02X \n", tintaSeleccionada, coloresHardware[tintaSeleccionada]);
                return true;
            }
            case 0b10000000: { // RMR 7F8x
                // 7f81 ->  mode 1, upper enabled,  lower enabled
                // 7f85 ->  mode 1, upper enabled,  lower disabled
                // 7f87 ->  mode 2, upper disabled, lower enabled
                // 7f89 ->  mode 1, upper disabled, lower enabled
                // 7f8d ->  mode 1, upper disabled, lower disabled
                // Select screen mode, rom configuration and interrupt control
                //screenMode = dato & 0b00000011;
                configureModeAndRoms(dato);
                //cambioMode = dato & 0b00000011;
                //bool lowerRomDisabled = dato & 0b00000100;
                //bool upperRomDisabled = dato & 0b00001000;
                //
                ////debug_ga("GA:: lower_rom_dis=%d upper_rom_dis=%d screen_mode=%d \n", lowerRomDisabled, upperRomDisabled, screenMode);
                //memoria->setEnableLowerRom(!lowerRomDisabled);
                //memoria->setEnableUpperRom(!upperRomDisabled);

                if (dato & 0x10) { // Interrupt generation control
                    r52 = 0;
					cpu->requestInterrupt(); // ¿o quitar int pendiente si la hubiera?
                    debug_ga("GA:: R52 reset\n"); // ¿r52 reset o vcc reset?
                }

                return true;
            }
        }
    }

    // puerto: de 78 a 7F: memory extension -> seleccionar pagina de la memoria ram extendida de 4MB
    // para 512MB creo que solo hace falta 0x7F00
    /* bit  7,6   = 3 -> gate array funcion 3
     * bits 5,4,3 -> 64k bank number (0..7); always 0 on an unexpanded cpc6128, 0-7 on standard memory expansions
     * bits 2,1,0 -> ram config (0..7)
     * las expansiones standard solo usan el puerto 0x78, el resto del 0x79 al 0x7f
     */
        /* las expansiones superiores a 512k amplian eso por encima del estandar. los lsb del numero de banco de 64k se mantienen 
         * en bits de datos d5-d3, los msb del numero de banco de 64k estan en bits de direcion a10-a8. normalmente en forma invertida
         * (por lo que se accede al primer bloque de 512k a traves del puerto 7fxx, al siguiente a traves de 7fxx, etc.
         */
    if (tipodato == 0xC0  &&  puerto.b.h >= 0x78  &&  puerto.b.h <= 0x7F) {
        u8 pagina = (puerto.b.h & 0x07);
        pagina = 7 - pagina;
        pagina = pagina << 3;
        pagina |= (dato & 0b00111000) >> 3;
        ramConfiguration = dato & 0b00000111;
        memoria->configRam(pagina, ramConfiguration);
        return true;

        //if (puerto.b.h == 0x78) {  // primeros 512k
        //    pagina = (dato & 0b00111000) >> 3;
        //}
        //else if (puerto.b.h >= 0x79  &&  puerto.b.h <= 0x7F) { // mas memoria extendida
        //    pagina = (puerto.b.h & 0x07) << 3; // msb
        //    pagina |= (dato & 0b00111000) >> 3; // lsb
        //}
        //u8 numConfig = dato & 0x07;
        //debug_ga(" puerto=%02X  pagina64k=%d %02X  config_ram=%d\n", puerto.b.h, pagina, pagina, numConfig);
        //memoria->config(pagina, numConfig);
        //return true;
    }

    // seleccionar banco de rom para la upper rom
    if (puerto.b.h == 0xDF) {
        memoria->selectUpperRom(dato);
        debug_ga("GA:: seleccion de rom %d\n", dato);
        return true;
    }

    return false;
}



void GateArray:: configureModeAndRoms(BYTE dato) {
    cambioMode = dato & 0b00000011;
    lowerRomDisabled = dato & 0b00000100;
    upperRomDisabled = dato & 0b00001000;
    
    //debug_ga("GA:: lower_rom_dis=%d upper_rom_dis=%d screen_mode=%d \n", lowerRomDisabled, upperRomDisabled, screenMode);
    memoria->setEnableLowerRom(!lowerRomDisabled);
    memoria->setEnableUpperRom(!upperRomDisabled);
}


// seguro que debe ser asi?
void GateArray:: configureRam(u8 configuracion) {
    memoria->configRam(0, configuracion);
}



bool GateArray:: IN(WORD puerto, BYTE* dato) {
    return false;
}


void GateArray:: nextLine() {
    //debug_ga("next line\n");
    monitor->nextLine();
    modoVideoLinea = screenMode;  // el modo de video se puede cambiar cada linea
}

void GateArray:: nextFrame(i16 vdur_min) {
    //debug_ga("GA.  next frame\n");
    monitor->nextFrame(vdur_min);
}


void GateArray:: memoriaVideo(u16 vma, u8 numPaginas, u8 bancoRam) {
    BYTE b;
    memoria->readByteVRam(vma, &b);
    //debug_ga("<%04X=%02X>", vma, b);
    
    //if (numPaginas == 1) 
    //else memoria->readByte(*vma, &b);

    calcSubpixelsByte(b); // arrayTintas = numeros de tinta a usar
    // cada entrada del array contiene el color firmware
    /*debug_ga("<");
    for (u8 i = 0; i < 8; i++) {
        debug_ga("%02X ",arrayTintas[i]);
    }
    debug_ga("-> ");*/

    for (u8 i = 0; i < 8; i++) {
        arrayTintas[i] = coloresHardware[arrayTintas[i]];
        //debug_ga("%02X ",arrayTintas[i]);
    }
    //debug_ga(">");

    monitor->update(arrayTintas);
}

void GateArray:: sync(bool hsync, bool vsync) {
    monitor->update(hsync, vsync);
}

void GateArray:: borde() {
    monitor->update(coloresHardware[HW_BORDER]);
}


// dado un byte y un modo, devuelve un array de tamaño 8, donde cada elemento tiene el nº de tinta
void GateArray:: calcSubpixelsByte(BYTE b) {
    BYTE x;
	switch (modoVideoLinea) {
        case 2:
            arrayTintas[0] = (b & 0b10000000) ? 1 : 0;
            arrayTintas[1] = (b & 0b01000000) ? 1 : 0;
            arrayTintas[2] = (b & 0b00100000) ? 1 : 0;
            arrayTintas[3] = (b & 0b00010000) ? 1 : 0;
            arrayTintas[4] = (b & 0b00001000) ? 1 : 0;
            arrayTintas[5] = (b & 0b00000100) ? 1 : 0;
            arrayTintas[6] = (b & 0b00000010) ? 1 : 0;
            arrayTintas[7] = (b & 0b00000001) ? 1 : 0;
            break;

        case 1:
            /*
            x = ((b & 0b10000000) >> 7) | ((b & 0b00001000) >> 2);
            arrayTintas[0] = arrayTintas[1] = x;
            x = ((b & 0b01000000) >> 6) | ((b & 0b00000100) >> 1);
            arrayTintas[2] = arrayTintas[3] = x;
            x = ((b & 0b00100000) >> 5) | (b & 0b00000010);
            arrayTintas[4] = arrayTintas[5] = x;
            x = ((b & 0b00010000) >> 4) | ((b & 0b00000001) << 1);
            arrayTintas[6] = arrayTintas[7] = x;
            */
            arrayTintas[0] = arrayTintas[1] = ((b & 0b10000000) >> 7) | ((b & 0b00001000) >> 2);
            arrayTintas[2] = arrayTintas[3] = ((b & 0b01000000) >> 6) | ((b & 0b00000100) >> 1);
            arrayTintas[4] = arrayTintas[5] = ((b & 0b00100000) >> 5) | (b & 0b00000010);
            arrayTintas[6] = arrayTintas[7] = ((b & 0b00010000) >> 4) | ((b & 0b00000001) << 1);
            break;

        case 0:
            /*
            // a0 a1 a2 a3
            x = ((b & 0x80) >> 7) | ((b & 0x08) >> 2) | ((b & 0x20) >> 3) | ((b & 0x02) << 2);
            arrayTintas[0] = arrayTintas[1] = arrayTintas[2] = arrayTintas[3] = x;
            // b0 b1 b2 b3
            x = ((b & 0x40) >> 6) | ((b & 0x04) >> 1) | ((b & 0x10) >> 2) | ((b & 0x01) << 3);
            arrayTintas[4] = arrayTintas[5] = arrayTintas[6] = arrayTintas[7] = x;
            */
            // a0 a1 a2 a3
            arrayTintas[0] = arrayTintas[1] = arrayTintas[2] = arrayTintas[3] =
                ((b & 0x80) >> 7) | ((b & 0x08) >> 2) | ((b & 0x20) >> 3) | ((b & 0x02) << 2);
            // b0 b1 b2 b3
            arrayTintas[4] = arrayTintas[5] = arrayTintas[6] = arrayTintas[7] = 
                ((b & 0x40) >> 6) | ((b & 0x04) >> 1) | ((b & 0x10) >> 2) | ((b & 0x01) << 3);
            break;
            
        case 3:
            /*
            arrayTintas[0] = arrayTintas[1] = arrayTintas[4] = arrayTintas[5] = 0;
            x = ((b & 0x08) >> 2) | ((b & 0x80) >> 7); // a1 a0
            arrayTintas[2] = arrayTintas[3] = x;
            x = ((b & 0x40) >> 6) | ((b & 0x04) >> 1); 
            arrayTintas[6] = arrayTintas[7] = x;
            */
            arrayTintas[0] = arrayTintas[1] = arrayTintas[4] = arrayTintas[5] = 0;
            // a1 a0
            arrayTintas[2] = arrayTintas[3] = ((b & 0x08) >> 2) | ((b & 0x80) >> 7);
            // b0 b1
            arrayTintas[6] = arrayTintas[7] = ((b & 0x40) >> 6) | ((b & 0x04) >> 1); 

            break;

        default:
            break;
	}
}


//void GateArray:: updateColors() {
//    for (u8 c = 0; c < 17; c++) {
//        if (cambioColor[c] != 255) {
//            coloresHardware[c] = cambioColor[c];
//            cambioColor[c] = 255;
//        }
//    }
//}

void GateArray:: updateMode() {
    if (cambioMode != 255) {
        screenMode = cambioMode;
        cambioMode = 255;
    }
}


void GateArray:: incR52() {
    r52++;
    if (r52 > 51) {
        r52 = 0;
        //debug_ga("GA::  solicitar_interrupcion!\n");
		cpu->requestInterrupt();
    }
}

u8 GateArray:: getR52() {
    return r52;
}

void GateArray:: reset_R52_bit5() {
    r52 = r52 & 0x1F;
}

bool GateArray:: get_R52_bit5() {
    return r52 & 0b00100000;
}


#ifdef debug_ints_borde
void GateArray:: startInt() {
    colorBordeInt = coloresHardware[HW_BORDER];
    coloresHardware[HW_BORDER] = 0;
}

void GateArray:: endInt() {
    coloresHardware[HW_BORDER] = colorBordeInt;
}
#endif