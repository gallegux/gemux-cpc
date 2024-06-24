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

/**
https://www.cpcwiki.eu/index.php/Standard_Memory_Expansions
https://cpctech.cpc-live.com/docs/rampage.html
https://www.cpcwiki.eu/index.php/Gate_Array#ROM_configuration_selection
https://www.cpcwiki.eu/index.php/Upper_ROM_Bank_Number

la memoria se divide en paginas, cada pagina tiene 4 bancos de 16kb
**/


#include <fstream>
#include <stdio.h>
#include <string>
#include "log.h"
#include "tipos.h"
#include "memoria.h"
#include "breakpoint.h"


Banco::Banco() {}

Banco::~Banco() {}

void Banco::reset() {
    for (unsigned short int x = 0; x < TAM_BANCO; x++)  bytes[x] = 0;
}

inline void Banco::writeByte(DIR offset, BYTE dato) {
    bytes[offset] = dato;
}

inline void Banco::readByte(DIR offset, BYTE *dato) {
    *dato = bytes[offset];
}


bool Banco:: load(std::string& fichero, Banco* banco) {
    std::ifstream file(fichero, std::ios::binary);

    if (file) {
        file.read(reinterpret_cast<char*>(banco->bytes), TAM_BANCO);

        if (file.tellg() != TAM_BANCO) {
            debug_mem("Banco:: load() rom de tamaño no estándar");
        }
        return true;
    }
    else {
        debug_mem("Banco:: load() %s no cargado\n", fichero.c_str());
        return false;
    }
}

Banco* Banco:: load(const std::string& fichero) {
    std::ifstream file(fichero, std::ios::binary);

    if (file) {
        Banco* banco = new Banco;
        file.read(reinterpret_cast<char*>(banco->bytes), TAM_BANCO);

        if (file.tellg() != TAM_BANCO) {
            debug_mem("Banco:: load() rom de tamaño no estándar");
            delete banco;
            banco = nullptr;
        }
        file.close();
        return banco;
    }
    else {
        debug_mem("Banco:: load() %s no cargado\n", fichero.c_str());
        return nullptr;
    }
}

//------------------------------------------------------------------------

Memoria::Memoria() {
    for (u8 i = 0; i < 4; i++) {
        ram_base[i] = new Banco;
    }

    for (BYTE x = 0; x < MAX_ROMS; x++) rom_upperRoms[x] = nullptr;

    setDefaultRamConfiguration();
}

Memoria::~Memoria() {
    debug_mem("Memoria:: ~memoria bancos_ram\n");
    for (BYTE x = 0; x < ram_numBancos; x++)  delete ram_bancos[x];

    debug_mem("Memoria:: ~memoria lower_rom\n");
    if (rom_lower != nullptr) delete rom_lower;

    debug_mem("Memoria:: ~memoria upper_roms\n");
    rom_upper = nullptr;
    for (BYTE x = 0; x < MAX_ROMS; x++) {
        if (rom_upperRoms[x] != nullptr)  {
            debug_mem("Memoria:: ~memoria eliminar upper rom %d\n", x);
            delete rom_upperRoms[x];
        }
    }
    debug_mem("Memoria:: ~memoria fin\n");
}

void Memoria::reset() {
    upperRomEnabled = true;
    lowerRomEnabled = true;

    resetRam();
}

void Memoria::resetRam() {
    for (BYTE x = 0; x < ram_numBancos; x++) ram_bancos[x]->reset();

    setDefaultRamConfiguration();
}


void Memoria:: addExtendedRamPages(BYTE _numPaginas) {
    u8 inicio = ram_numPaginas * BANCOS_POR_PAGINA;
    u8 fin = (ram_numPaginas + _numPaginas) * BANCOS_POR_PAGINA;

    for (u8 x = inicio; x < fin; x++) {
        ram_bancos[ram_numBancos++] = new Banco;
    }
    ram_numPaginas += _numPaginas;
    {
        int memoriaExtendida = (ram_numPaginas * BANCOS_POR_PAGINA * TAM_BANCO)/1024;
        debug("Memoria:: memoria extendida: %dk (paginas=%d)\n", memoriaExtendida, ram_numPaginas);
    }
}

void Memoria:: setLowerRom(Banco* rom) {
    rom_lower = rom;
}


void Memoria:: addUpperRom(BYTE slot, Banco* rom) {
    if (slot < MAX_ROMS) {
        rom_upperRoms[slot] = rom;
    }
}

void Memoria:: setEnableUpperRom(bool e) { upperRomEnabled = e; }

void Memoria:: setEnableLowerRom(bool e) { lowerRomEnabled = e; }

void Memoria:: selectUpperRom(BYTE slot) {
    if (slot < MAX_ROMS && rom_upperRoms[slot] != nullptr) rom_upper = rom_upperRoms[slot];
    else rom_upper = rom_upperRoms[0];
}


void Memoria::calcBancoOffset(DIR posicion) {
    bancoSeleccionado = posicion >> 14;
	offset = posicion & 0x3FFF;
}


void Memoria::writeByte(DIR posicion, BYTE dato) {
    calcBancoOffset(posicion);
    ram_bancosAcc[bancoSeleccionado]->writeByte(offset, dato);
}

void Memoria::writeWord(DIR posicion, WORD dato) {
    writeByte(  posicion, dato.b.l);
    writeByte(++posicion, dato.b.h);
}


void Memoria::readByte(DIR posicion, BYTE *dato) {
    bp_test_read(posicion);
    calcBancoOffset(posicion);

    //debug("Memoria:: readByte() %04X\n", posicion);
    if (upperRomEnabled  &&  bancoSeleccionado == 3  &&  rom_upper != nullptr) {
        //debug("de la upper rom\n");
        rom_upper->readByte(offset, dato);
        return;
    }
    if (lowerRomEnabled  &&  bancoSeleccionado == 0  &&  rom_lower != nullptr) {
        //debug("de la lower rom\n");
        rom_lower->readByte(offset, dato);
        return;
    }
    ram_bancosAcc[bancoSeleccionado]->readByte(offset, dato);
}

// la memoria de video solo puede estar en los primeros 64K!
void Memoria::readByteVRam(DIR posicion, BYTE *dato) {
    calcBancoOffset(posicion);
    ram_base[bancoSeleccionado]->readByte(offset, dato);
}



void Memoria::readWord(DIR posicion, WORD *dato) {
    readByte(posicion, &dato->b.l );
	readByte(++posicion, &dato->b.h );
}


void Memoria::setDefaultRamConfiguration() {
    for (u8 i = 0; i < 4; i++) ram_bancosAcc[i] = ram_base[i];
}


void Memoria::configRam(BYTE numPagina, BYTE numConfiguracion) {
    // https://www.grimware.org/doku.php/documentations/devices/gatearray#regular.rmr.register
    debug_mem("Memoria:: configRam() pagina64k=%d numConfig=%d\n", numPagina, numConfiguracion);

    setDefaultRamConfiguration();
    if (ram_numPaginas == 0) return;
    else if (numPagina >= ram_numPaginas) numPagina = 0;

    BYTE numBanco1pagina = numPagina*BANCOS_POR_PAGINA; // primer banco de la pagina: 4,8,12,...

    switch (numConfiguracion) {
        case 0:     break;
        case 1:     ram_bancosAcc[3] = ram_bancos[numBanco1pagina+3];   break;
        case 2:
            ram_bancosAcc[0] = ram_bancos[numBanco1pagina + 0];
            ram_bancosAcc[1] = ram_bancos[numBanco1pagina + 1];
            ram_bancosAcc[2] = ram_bancos[numBanco1pagina + 2];
            ram_bancosAcc[3] = ram_bancos[numBanco1pagina + 3];
            break;
        case 3:
            ram_bancosAcc[1] = ram_base[3];
            ram_bancosAcc[3] = ram_bancos[numBanco1pagina + 3];
            break;
        default:
            ram_bancosAcc[1] = ram_bancos[numBanco1pagina + (numConfiguracion & 3) ];
            break;
    }
}


void Memoria:: grabarRam(std::string& fichero) {
    std::ofstream  f = std::ofstream (fichero, std::ios::out | std::ios::binary);
    debug_mem("Memoria:: garbarRam() fichero=%s\n", fichero.c_str());
    
    for (u8 i = 0; i < 4; i++) {
       f.write( reinterpret_cast<char*>(ram_bancosAcc[i]), 0x4000 );
    }

    f.close();
}
