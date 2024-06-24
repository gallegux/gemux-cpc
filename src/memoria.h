/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  RAM and ROM implementation                                                |
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
#include "tipos.h"


#define TAM_BANCO 0x4000
#define MAX_PAGINAS_RAM 8   // hasta 512k de ram
#define BANCOS_POR_PAGINA 4
#define MAX_ROMS 32


class Banco
{
    BYTE bytes[TAM_BANCO];

public:
    Banco();
    ~Banco();
    
    void reset();
    
    void writeByte(DIR offset, BYTE dato);
    void readByte(DIR offset, BYTE *dato);
    
    BYTE& operator[](DIR i);

    static bool load(std::string& fichero, Banco* banco);
    static Banco* load(const std::string& fichero);
};


/* 
por defecto: 
- la lower rom (slot 0) es el basic
- la upper rom (slot 7) es el amsdos

se puede cambiar la upper rom
*/

class Memoria
{
    BYTE ram_numBancos = 0;
    BYTE ram_numPaginas = 0;

    Banco* ram_base[4];
    Banco* ram_bancos[BANCOS_POR_PAGINA * MAX_PAGINAS_RAM];
    Banco* ram_bancosAcc[BANCOS_POR_PAGINA];

    Banco* rom_lower = nullptr;
    Banco* rom_upper = nullptr; // upper rom actual
    Banco* rom_upperRoms[MAX_ROMS];// = {nullptr};
    bool upperRomEnabled = false;
    bool lowerRomEnabled = true;

    BYTE bancoSeleccionado; // de 0 a 3
    DIR offset; // de 0000 a 3ffff
    
    void calcBancoOffset(DIR posicion);
    void resetRam(); // reset ram

public:
    Memoria();
    ~Memoria();

    void reset();

    void addExtendedRamPages(BYTE numPaginas);

    void setLowerRom(Banco* rom);
    void addUpperRom(BYTE slot, Banco* rom); // invocar al crear la maquina

    void setEnableUpperRom(bool e);
    void setEnableLowerRom(bool e);
    void selectUpperRom(BYTE slot); // que pasa cuando se selecciona una rom que no existe?
    
    void writeByte(DIR posicion, BYTE dato);
    void writeWord(DIR posicion, WORD dato);
    
    void readByte(DIR posicion, BYTE *dato);
    void readWord(DIR posicion, WORD *dato);

    void readByteVRam(DIR posicion, BYTE* dato);

    void setDefaultRamConfiguration();
    void configRam(BYTE numPagina, BYTE numConfiguracion);
    
    void grabarRam(std::string& fichero);
    
};

