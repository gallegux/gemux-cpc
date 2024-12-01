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
#include <fstream>
#include "tipos.h"


// no separo la gestion de la RAM y la ROM

constexpr u16 TAM_BANCO = 0x4000;    // cada banco de memoria es de 16k


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

    void write(std::fstream& fichero);
	void loadBytes(std::fstream& fichero);
	bool loadBytes(const std::string& fichero);

    static bool load(const std::string& fichero, Banco* banco);
    static Banco* load(const std::string& fichero);
};


/* 
por defecto: 
- la lower rom (slot 0) es el basic
- la upper rom (slot 7) es el amsdos

se puede cambiar la upper rom
*/

class Memory
{
public:
	static constexpr u8  MAX_PAGINAS_RAM = 8;   // hasta 512k de ram
	static constexpr u8  BANCOS_POR_PAGINA = 4; // cada pagina es de 64k
	static constexpr u8  MAX_BANCOS = BANCOS_POR_PAGINA * MAX_PAGINAS_RAM;
	static constexpr u8  MAX_ROMS = 32;

private:
    BYTE ram_numBancosExt = 0; // numero de bancos de la memoria extendida (ram_numPaginasExt*4)
    BYTE ram_numPaginasExt = 0; // numero de paginas de la memoria extendida

    Banco* ram_base[4];
    Banco* ram_bancosExt[BANCOS_POR_PAGINA * MAX_PAGINAS_RAM];
    Banco* ram_bancosAcc[BANCOS_POR_PAGINA]; // bancos accesibles (0x0000->0xFFFF)

    Banco* rom_lower = nullptr;
    Banco* rom_upper = nullptr; // upper rom actual, va cambiando
    Banco* rom_uppers[MAX_ROMS];// = {nullptr};

	std::string lowerRomFile = "";
	std::string upperRomFiles[MAX_ROMS] = {""};

    bool upperRomEnabled = false;
    bool lowerRomEnabled = true;

    BYTE bancoSeleccionado; // de 0 a 3
    DIR offset; // de 0000 a 3ffff
    
    void calcBancoOffset(DIR posicion);
    void resetRam(); // reset ram


public:
    Memory();
    ~Memory();

    void reset();
	void removeExtendedRam(); // quita todos los bancos de memoria ram

    void addExtendedRamPages(BYTE numPaginas);
    u8   getExtendedRamPages();
	void removeExtendedRamPages(u8 n);

    //void setLowerRom(Banco* rom);
    bool setLowerRom(const std::string& romFile);
    //void setUpperRom(BYTE slot, Banco* rom); // invocar al crear la maquina
    bool setUpperRom(u8 slot, const std::string& romFile); // invocar al crear la maquina

	std::string& getLowerRomFile();
	void getUpperRomFile(u8 slot, std::string& romfile, bool *loaded);

	void removeRoms();
	void removeUpperRom(u8 slot);


    void setEnableUpperRom(bool e);
    void setEnableLowerRom(bool e);
    void selectUpperRom(BYTE slot);
    
    void writeByte(DIR posicion, BYTE dato);
    void writeWord(DIR posicion, WORD dato);
    
    void readByte(DIR posicion, BYTE *dato);
    void readWord(DIR posicion, WORD *dato);

    void readByteVRam(DIR posicion, BYTE* dato); // lee un byte de la primera pagina de memoria

    void setDefaultRamConfiguration();
    void configRam(BYTE numPagina, BYTE numConfiguracion);

	// para sna
    u16 getRamSize();
	void saveRam(std::fstream& fichero);
	void loadRam(std::fstream& fichero);
    
    void grabarRam(std::string& fichero);
    
};

