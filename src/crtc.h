/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CRTC implementation                                                       |
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
#include "io_device.h"
#include "z80.h"
#include "gatearray.h"
#include "sna.h"


#define horizontal_displayed registros[1]
#define horizontal_sync_position registros[2]

#define CRTC_NUM_REGISTERS 18 // ??
#define CRTC_PALETTE_SIZE 17


class CRTC : public IO_Device
{
public:   
    Z80* cpu;
    GateArray* gatearray;
	BYTE crtcType = 0;

    //bool ajustado;
    BYTE registroSeleccionado = 0;
    BYTE registros[32]; // para acceder desde el raster, ¿?

    // "contadores" internos
   	bool hsync, vsync; // indica si se estan en zonas de sinconizacion
    bool pintar_caracter_horizontal, pintar_caracter_vertical; // si hay que pintar caracter
    //i8 c52; // contador para la interrupcion
    i8 hcc, vcc; // contadores de caracter horizontal y vertical
    u8 hsc, vsc; // contadores de pulsos de sincronismo horizontal y vertical
    u8 vlc; // contador de la linea de caracter (normalmente de 0 a 7)
    u8 vtac; // contador las lineas adicionales
    i16 vdur; // contador de la linea absoluta
    i16 vdur_min;
    u32 vma; // direccion de la memoria de video a la que se accede para pintarlo

    u8 paginas; // paginas que se muestran (cuando registros[12] & 0b00001100  ==  0b00001100)
    u8 bancoRam;
    u16 vma_base, vma_offset;
    u32 limite_pagina;

    u8 horizontal_total; // R0+1
    u8 lineas_caracter; // R9
    u8 vertical_total; // R6+1
    u8 lineas_adicionales; // num lineas adicionales
    u8 vertical_displayed; // lineas dentro del borde
    u8 vertical_sync_position; // R7
    u16 vma_origen; // direccion de comienzo de la memoria para empezar a pintar
    u16 mascara_incremento; // en funcion del numero de paginas
    u8 fin_hsync; //, fin_vsync; // para indicar que se ha terminado la sincronizacion horizontal o vertical

    u8 interlace_and_skew;

    
public:

    CRTC(Z80* cpu, GateArray* ga);

    void reset() override;
    bool OUT(WORD puerto, BYTE dato) override;
    bool IN(WORD puerto, BYTE* dato) override;
    void update(u8 numCiclosCpu) override;

    void actualizarVariablesInternas();

    void incLinea();

    BYTE getType() { return 0; }

	void setSnaData(SNA_CRTC* sna, u8 version);
	void getSnaData(SNA_CRTC* sna);

    void print();
};


