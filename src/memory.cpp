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
#include <cstring> // Para memset
#include "log.h"
#include "tipos.h"
#include "memory.h"
#include "breakpoint.h"
#include "directories.h"
#include "util.h"


Banco::Banco() {}

Banco::~Banco() {}

void Banco::reset() {
	std::memset(bytes, 0, TAM_BANCO);
    //for (unsigned short int x = 0; x < TAM_BANCO; x++)  bytes[x] = 0;
}

inline void Banco::writeByte(DIR offset, BYTE dato) {
    bytes[offset] = dato;
}

inline void Banco::readByte(DIR offset, BYTE *dato) {
    *dato = bytes[offset];
}


void Banco:: write(std::fstream& fichero) {
    fichero.write(reinterpret_cast<char*>(bytes), TAM_BANCO);
}


void Banco:: loadBytes(std::fstream& fichero) {
	fichero.read(reinterpret_cast<char*>(bytes), TAM_BANCO);
}

bool Banco:: loadBytes(const std::string& fichero) {
	std::ifstream f {fichero, std::ios::binary};

	if (f) {
       	f.read(reinterpret_cast<char*>(bytes), TAM_BANCO);
		return true;
	}
	else {
		debug_mem("BANK:: loadBytes() %s no encontrado\n", fichero.c_str());
		return false;
	}
}


// static
bool Banco:: load(const std::string& fichero, Banco* banco) {
    std::ifstream file(fichero, std::ios::binary);
	return banco->loadBytes(fichero);
    //if (file) {
	//	if (checkFileSize(file, TAM_BANCO)) {
    //    	file.read(reinterpret_cast<char*>(banco->bytes), TAM_BANCO);
	//		return true;
	//	}
	//	else {
	//		debug_mem("Bank:: load(2) rom de tamaño no estándar");
    //        return false;
	//	}
    //}
    //else {
    //    debug_mem("Bank:: load(2) %s no cargado\n", fichero.c_str());
    //    return false;
    //}
}

// static
Banco* Banco:: load(const std::string& fichero) {
    std::ifstream file(fichero, std::ios::binary);

	Banco* banco = new Banco;
	bool ok = banco->loadBytes(fichero);
	
	if (!ok) {
		delete banco;
		banco = nullptr;
	}
	return banco;
//
//	if (ok)
//			file.read(reinterpret_cast<char*>(banco->bytes), TAM_BANCO);
//			return banco;
//		}
//        else {
//            debug_mem("Bank:: load(1) rom de tamaño no estándar");
//            return nullptr;
//        }
//    }
//    else {
//        debug_mem("Bank:: load(1) %s no cargado\n", fichero.c_str());
//        return nullptr;
//    }
}


//------------------------------------------------------------------------

Memory::Memory() {
    for (u8 i = 0; i < 4; i++)  ram_base[i] = new Banco;

    for (BYTE x = 0; x < MAX_ROMS; x++) rom_uppers[x] = nullptr;

    setDefaultRamConfiguration();
}

Memory::~Memory() {
    debug_mem("Memory:: ~memoria bancos_ram\n");
    for (BYTE x = 0; x < ram_numBancosExt; x++)  delete ram_bancosExt[x];

    debug_mem("Memory:: ~memoria lower_rom\n");
    if (rom_lower != nullptr) delete rom_lower;

    debug_mem("Memory:: ~memoria upper_roms\n");
    rom_upper = nullptr;
    for (BYTE x = 0; x < MAX_ROMS; x++) {
        if (rom_uppers[x] != nullptr)  {
            debug_mem("Memory:: ~memoria eliminar upper rom %d\n", x);
            delete rom_uppers[x];
        }
    }
    //debug_mem("Memory:: ~memoria fin\n");
}

void Memory::reset() {
    upperRomEnabled = true;
    lowerRomEnabled = true;
	rom_upper = nullptr;

    resetRam();
}

void Memory::resetRam() {
    setDefaultRamConfiguration();

    for (BYTE x = 0; x < BANCOS_POR_PAGINA; x++) ram_base[x]->reset();
    for (BYTE x = 0; x < ram_numBancosExt; x++) ram_bancosExt[x]->reset();
}


// elimina todos los bancos de memoria extendida
void Memory:: removeExtendedRam() {
	setDefaultRamConfiguration();   // para que ram_bancosAcc[x] no a punte a ram_bancosExt[x] que se pueda eliminar

	for (u8 x = 0; x < ram_numBancosExt; x++) {
		if (ram_bancosExt[x] != nullptr) {
			debug_mem("MEM:: eliminar banco %d\n", x);
			delete ram_bancosExt[x];
			ram_bancosExt[x] = nullptr;
		}
	}
	ram_numPaginasExt = ram_numBancosExt = 0;

	//for (u8 x = 0; x < MAX_ROMS; x++) {
	//	if (rom_upperRoms[x] != nullptr) {
	//		delete rom_upperRoms[x];
	//		rom_upperRoms[x] = nullptr;
	//	}
	//}
	//
	//delete rom_lower;
}


void Memory:: addExtendedRamPages(BYTE _numPaginas) {
	debug_mem("Memory:: addExtendedRamPages(%d)\n", _numPaginas);
    u8 inicio = ram_numPaginasExt * BANCOS_POR_PAGINA;
    u8 fin = (ram_numPaginasExt + _numPaginas) * BANCOS_POR_PAGINA;

    for (u8 x = inicio; x < fin; x++) {
        ram_bancosExt[ram_numBancosExt++] = new Banco;
    }
    ram_numPaginasExt += _numPaginas;
    {
        int memoriaExtendida = (ram_numPaginasExt * BANCOS_POR_PAGINA * TAM_BANCO)/1024;
        debug_mem("Memory:: memoria extendida: %dk (paginas=%d)\n", memoriaExtendida, ram_numPaginasExt);
    }
}


void Memory:: removeExtendedRamPages(u8 n) {
	u8 eliminar = n * BANCOS_POR_PAGINA;
	u8 numBanco = ram_numBancosExt-1;
	//debug_mem("MEM:: bancos_ext = %d\n", ram_numBancosExt);
	setDefaultRamConfiguration();
	
	while (eliminar > 0) {
		//debug_mem("MEM:: eliminar=%d  numBanco = %d\n", eliminar, numBanco);
		if (ram_bancosExt[numBanco] != nullptr) {
			delete ram_bancosExt[numBanco];
			ram_bancosExt[numBanco] = nullptr;
			ram_numBancosExt--;
			numBanco--;
			eliminar--;
		}
	}
	ram_numPaginasExt = ram_numBancosExt / 4;
}


u8 Memory:: getExtendedRamPages() { return ram_numPaginasExt; }


u16 Memory:: getRamSize() { return (ram_numPaginasExt+1) * 64; }


// return true si hay cambio
bool setRom(Banco** banco, std::string& romFilename, const std::string& newFilename) {
	if (romFilename != ""  ||  newFilename != "")
		debug_mem("MEM:: %s --> %s\n", romFilename.c_str(), newFilename.c_str());
	
	if (romFilename == newFilename) {
		// nada
		return false;
	}
	else if (romFilename == ""  &&  newFilename != "") {
		// poner
		debug_mem("MEM:: poner\n");
		*banco = Banco::load(newFilename);
		if (*banco != nullptr) {
			romFilename = newFilename;
			return true;
		}
		else {			
			romFilename = "";
			return false;
		}
	}
	else if (romFilename != ""  &&  newFilename == "") {
		// quitar
		debug_mem("MEM:: quitar\n");
		delete *banco;
		*banco = nullptr;
		return true;
	}
	else if (romFilename != newFilename) {
		// sustituir
		debug_mem("MEM:: sustituir\n");
		bool read = (*banco)->loadBytes(newFilename);
		if (read) {
			romFilename = newFilename;
		}
		else {
			delete *banco;
			*banco = nullptr;
			romFilename = "";
		}
		debug_mem("MEM:: 		sustituido %d\n", read);
		return true;
	}
	return true;
}



bool Memory:: setLowerRom(const std::string& newRomFile) {
	return setRom(&rom_lower, lowerRomFile, newRomFile);
	//bool ok = setRom(&rom_lower, lowerRomFile, newRomFile);
	//debug_mem("MEM:: lower_rom %d\n", rom_lower == nullptr);
	//return ok;
	
	//if (rom_lower != nullptr) {
	//	delete rom_lower;
	//	rom_lower = nullptr;
	//	lowerRomFile = "";
	//}
	//rom_lower = Banco::load(newRomFile);
	//if (rom_lower != nullptr) {
	//	lowerRomFile = newRomFile;
	//	return true;
	//}
	//return false;
}




bool Memory:: setUpperRom(u8 slot, const std::string& romFile) {
    if (slot < MAX_ROMS) {
		return setRom(&rom_uppers[slot], upperRomFiles[slot], romFile);
		//if (rom_uppers[slot] != nullptr) {
		//	delete rom_uppers[slot];
		//	rom_uppers[slot] = nullptr;
		//	upperRomFiles[slot] = "";
		//}
		//rom_uppers[slot] = Banco::load(romFile);
		//if (rom_uppers[slot] != nullptr) {
		//	//rom_uppers[slot] = rom;
	    //    upperRomFiles[slot] = romFile;
		//	debug_mem("MEM:: setUpperRom %d=%s\n", slot, romFile.c_str());
		//	return true;
		//}
		//else {
		//	debug_mem("MEM:: addUpperRom NO CARGADO %d=%s\n", slot, romFile.c_str());
		//}
    }
	else return false;
}


void Memory:: getUpperRomFile(u8 slot, std::string& romfile, bool *loaded) {
	if (slot < MAX_ROMS) {
		romfile = upperRomFiles[slot];
		if (rom_uppers[slot] == nullptr)  *loaded = true;
		else  *loaded = rom_uppers[slot] != nullptr  &&  upperRomFiles[slot] != "";
	}
	else {
		romfile = "";
		*loaded = true;
	}
}


std::string& Memory:: getLowerRomFile() { return lowerRomFile; }


// quita todos los bancos de rom cargados
void Memory:: removeRoms() {
	if (rom_lower != nullptr) {
		delete rom_lower;
		rom_lower = nullptr;
		lowerRomFile = "";
	}

	for (u8 i = 0; i < MAX_ROMS; i++) {
		if (rom_uppers[i] != nullptr) {
			delete rom_uppers[i];
			rom_uppers[i] = nullptr;
			upperRomFiles[i] = "";
		}
	}
	rom_upper = nullptr;
}


// quita un banco de rom
void Memory:: removeUpperRom(u8 slot) {
	if (slot < MAX_ROMS  &&  rom_uppers[slot] != nullptr) {
		if (rom_uppers[slot] == rom_upper) rom_upper = nullptr;

		delete rom_uppers[slot];
		rom_uppers[slot] = nullptr;
		upperRomFiles[slot] = "";
	}
}



void Memory:: setEnableUpperRom(bool e) { upperRomEnabled = e; }

void Memory:: setEnableLowerRom(bool e) { lowerRomEnabled = e; }

void Memory:: selectUpperRom(BYTE slot) {
    if (slot < MAX_ROMS && rom_uppers[slot] != nullptr) rom_upper = rom_uppers[slot];
    else rom_upper = rom_uppers[0];
}


void Memory::calcBancoOffset(DIR posicion) {
    bancoSeleccionado = posicion >> 14;
	offset = posicion & 0x3FFF;
}


void Memory::writeByte(DIR posicion, BYTE dato) {
    calcBancoOffset(posicion);
    ram_bancosAcc[bancoSeleccionado]->writeByte(offset, dato);
}

void Memory::writeWord(DIR posicion, WORD dato) {
    writeByte(  posicion, dato.b.l);
    writeByte(++posicion, dato.b.h);
}


void Memory::readByte(DIR posicion, BYTE *dato) {
    bp_test_read(posicion);
    calcBancoOffset(posicion);

    //debug("Memory:: readByte() %04X\n", posicion);
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
void Memory::readByteVRam(DIR posicion, BYTE *dato) {
    calcBancoOffset(posicion);
    ram_base[bancoSeleccionado]->readByte(offset, dato);
}



void Memory::readWord(DIR posicion, WORD *dato) {
    readByte(posicion, &dato->b.l );
	readByte(++posicion, &dato->b.h );
}


void Memory::setDefaultRamConfiguration() {
    for (u8 i = 0; i < 4; i++) ram_bancosAcc[i] = ram_base[i];
}


void Memory::configRam(BYTE numPagina, BYTE numConfiguracion) {
    // https://www.grimware.org/doku.php/documentations/devices/gatearray#regular.rmr.register
    debug_mem("Memory:: configRam() numPagina=%d numConfig=%02X\n", numPagina, numConfiguracion);

    setDefaultRamConfiguration();
    if (ram_numPaginasExt == 0) return;
    else if (numPagina >= ram_numPaginasExt) numPagina = 0;

    BYTE numBanco1pagina = numPagina*BANCOS_POR_PAGINA; // primer banco de la pagina: 4,8,12,...

    switch (numConfiguracion) {
        case 0:     break;
        case 1:     ram_bancosAcc[3] = ram_bancosExt[numBanco1pagina+3];   break;
        case 2:
            ram_bancosAcc[0] = ram_bancosExt[numBanco1pagina + 0];
            ram_bancosAcc[1] = ram_bancosExt[numBanco1pagina + 1];
            ram_bancosAcc[2] = ram_bancosExt[numBanco1pagina + 2];
            ram_bancosAcc[3] = ram_bancosExt[numBanco1pagina + 3];
            break;
        case 3:
            ram_bancosAcc[1] = ram_base[3];
            ram_bancosAcc[3] = ram_bancosExt[numBanco1pagina + 3];
            break;
        default:
            ram_bancosAcc[1] = ram_bancosExt[numBanco1pagina + (numConfiguracion & 3) ];
            break;
    }
}



void Memory:: saveRam(std::fstream& fichero) {
	// grabar ram base
	for (u8 i = 0; i < BANCOS_POR_PAGINA; i++) {
			debug_sna("banco base %d\n", i);
			ram_base[i]->write(fichero);
			//debug_sna("sna -> %llX\n", static_cast<long long>(fichero.tellp()) );
	}
	// ram extendida
	for (u8 i = 0; i < ram_numBancosExt; i++) {
			debug_sna("banco ext %d\n", i);
			ram_bancosExt[i]->write(fichero);
			//debug_sna("sna -> %llX\n", static_cast<long long>(fichero.tellp()) );
	}
}


void Memory:: loadRam(std::fstream& fichero) {
	// cargar ram base
	for (u8 i = 0; i < BANCOS_POR_PAGINA; i++) {
			debug_sna("SNA:: load banco base %d\n", i);
			ram_base[i]->loadBytes(fichero);
			//debug_sna("sna -> %llX\n", static_cast<long long>(fichero.tellp()) );
	}
	// ram extendida
	for (u8 i = 0; i < ram_numBancosExt; i++) {
			debug_sna("SNA:: load banco ext %d\n", i);
			ram_bancosExt[i]->loadBytes(fichero);
			//debug_sna("sna -> %llX\n", static_cast<long long>(fichero.tellp()) );
	}
}


void Memory:: grabarRam(std::string& fichero) {
    std::ofstream  f = std::ofstream (fichero, std::ios::out | std::ios::binary);
    debug_mem("Memory:: garbarRam() fichero=%s\n", fichero.c_str());
    
    for (u8 i = 0; i < 4; i++) {
       f.write( reinterpret_cast<char*>(ram_bancosAcc[i]), 0x4000 );
    }

    f.close();
}
