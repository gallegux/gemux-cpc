/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CPC structure implementation                                              |
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

/** 
Fichero de configuracion:

type=464|664|6128
monitor=BW|GREEN|ORANGE|COLOR
extMemPages=<nº paginas de 64k>
fdc=0|1  (solo aplica cuando es 464)
lowerRom=<fichero rom para la rom baja>
upperRom.0=<fichero rom para el slot 0>
...
upperRom.31=<fichero rom para el slot 31>
**/


#include <string>
#include "tipos.h"
#include "io.h"
#include "memory.h"
#include "z80.h"
#include "crtc.h"
#include "keyboard.h"
#include "psg.h"
#include "gatearray.h"
#include "monitor.h"
#include "ppi.h"
#include "fdc.h"
#include "disc_drive.h"
#include "cpc_models.h"
#include "target.h"
#include "log.h"



constexpr std::string_view TYPE_PROPERTY = "cpc_type";
constexpr std::string_view RAM_PROPERTY = "ram";
constexpr std::string_view LOWER_ROM_PROPERTY = "lower_rom";
constexpr std::string_view UPPER_ROM_PROPERTY = "upper_rom.";
constexpr std::string_view MON_PAL1_PROPERTY = "monitor_palette1";
constexpr std::string_view MON_PAL2_PROPERTY = "monitor_palette2";
constexpr std::string_view MON_SCANLINES = "monitor_scanlines";



class CPC
{
    
private:
	CPC_TYPE  cpcType;   // version de cpc
	IO_Bus io;
	Memory memory;
	Z80 cpu   {&memory, &io};

	// estos dispositivos siempre los tenemos
	Monitor    monitor;
	GateArray  gatearray {&cpu, &memory, &monitor};
	CRTC       crtc{&cpu, &gatearray};
	Keyboard   keyboard;
	PSG        psg {&keyboard};
	Tape       tape;
	PPI        ppi {&psg, &gatearray, &crtc, &tape};
	FDC*       fdc = nullptr;
	DiscDrive* discDrives[4] = {nullptr};

	u8 defaultCPC[3] = {0,4,5}; // cpc por defecto para los snas
    
public:
	static const CPC_MODEL STANDARD_MODELS[];
	static const std::string CPC_BASE[];
	//static const std::string CPC_TYPES[];

	#ifdef TARGET_PC
	static void print_models();
	#endif

	CPC();
	~CPC();

	BYTE getType() { return cpcType; }

	void reset();

	void setStandardModel(u8 nModel);
	void set464();
	void set664();
	void set6128();

	bool loadConfig(const std::string& fichero);
	bool saveConfig(const std::string& fichero);

	bool loadSna(const std::string& snaFile);
	bool saveSna(const std::string& snaFile);

    Z80*        getCpu()        { return &cpu; };
    Memory*     getMemory()     { return &memory; };
    GateArray*  getGateArray()  { return &gatearray; };
    CRTC*       getCrtc()       { return &crtc; };
    Keyboard*   getKeyboard()   { return &keyboard; };
    PPI*        getPPI()        { return &ppi; }
    PSG*        getPSG()        { return &psg; }
    Monitor*    getMonitor()    { return &monitor; }
    Tape*       getTape()       { return &tape; }
    
	void addFDC();
	bool hasFDC() { return fdc != nullptr; };
	FDC* getFDC() { return fdc; };
	bool removeFDC();

    DiscDrive* getDiscDrive(u8 unidad);

    // ejecuta la siguiente instruccion de la cpu
    void execute();
    u8 getInstructionCycles(); // devuelve m-cycles (no t-states) [1 m-cycle = 4 t-states]

};

/** 
class CPC464 : public CPC
{
public:
    CPC464() {}
};


class CPC664 : public CPC
{
public:
    CPC664() {
        debug("CPC664\n");
        addFDC();
    }
};


class CPC6128 : public CPC664
{
public:
    CPC6128() {
        debug("CPC6128\n");
        memory.addExtendedRamPages(1);
    }
};
**/
