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

#include <stdio.h>
#include <string>
#include <fstream>
#include "log.h"
#include "tipos.h"
#include "cpc.h"
#include "memory.h"
#include "io.h"
#include "crtc.h"
#include "z80.h"
#include "keyboard.h"
#include "psg.h"
#include "gatearray.h"
#include "ppi.h"
#include "monitor.h"
#include "properties.h"
#include "util.h"
#include "log.h"
#include "sna.h"
#include "compilation_options.h"
#include "directories.h"




CPC::CPC() {
    debug_cpc("CPC\n");

    cpu.setAttendedInterruptReceiver(&gatearray);
#ifdef debug_ints_borde
    cpu.setDisplayInt(&gatearray);
#endif
    
    io.addDevice( &crtc );
    io.addDevice( &gatearray );
    io.addDevice( &ppi );

    reset();
}


CPC::~CPC() {
    for (u8 u = 0; u < 4; u++)
        if (discDrives[u] != nullptr)  
            delete discDrives[u];

    if (fdc != nullptr) delete fdc;
}



void CPC:: setStandardModel(u8 nModel) {
	CPC_MODEL model = STANDARD_MODELS[nModel];
	cpcType = (CPC_TYPE) model.cpcType;

	if (hasFDC() && cpcType == CPC464) {

		removeFDC();
	}
	else if (!hasFDC() && (cpcType == CPC6128 || cpcType == CPC664)) { 
		// poner fdc
		addFDC();
	}

	memory.removeExtendedRam(); // quita toda la mem ext

	std::string romsDir {ROMS_DIR};
	romsDir.append("/");
	debug_cpc("CPC:: roms_dir [%s]\n", romsDir.c_str());
	//debug_cpc("CPC: sustituir rom 1\n");
	memory.setLowerRom(romsDir + model.lowerRowFile);
	//debug_cpc("CPC: sustituir rom 2\n");
	memory.setUpperRom(0, romsDir + model.upperRom0File);
	//debug_cpc("CPC: sustituir rom 3\n");
	memory.setUpperRom(7, romsDir + model.upperRom7File);

	for (u8 i = 0; i < Memory::MAX_ROMS; i++) {
		if (i != 0  &&  i != 7)  memory.setUpperRom(i, "");
	}

	if (cpcType == CPC6128)  memory.addExtendedRamPages(1); 
}


void CPC:: set464() {  
	setStandardModel(DEFAULT_464);
	/*cpcType = CPC464;
	memory.setLowerRom( Banco::load("roms/OS_464_UK.ROM") );
	memory.setUpperRom(0, Banco::load("roms/BASIC_464_UK.ROM"));*/
}

void CPC:: set664() { 
	setStandardModel(DEFAULT_664);
	/*cpcType = CPC664;
	addFDC(); 
	memory.setLowerRom( Banco::load("roms/OS_664_UK.ROM") );
	memory.setUpperRom(0, Banco::load("roms/BASIC_664_UK.ROM"));
	memory.setUpperRom(7, Banco::load("roms/AMSDOS.ROM"));*/
}

void CPC:: set6128() { 
	setStandardModel(DEFAULT_6128);
	/*cpcType = CPC6128;
	addFDC(); 
	memory.addExtendedRamPages(1); 
	memory.setLowerRom( Banco::load("roms/OS_6128_UK.ROM") );
	memory.setUpperRom(0, Banco::load("roms/BASIC_6128_UK.ROM"));
	memory.setUpperRom(7, Banco::load("roms/AMSDOS.ROM"));*/
}


void CPC:: addFDC() {
    if (fdc == nullptr) {
        fdc = new FDC;

        for (u8 u = 0; u < 4; u++) {
            discDrives[u] = new DiscDrive(u, fdc);
        }
        io.addDevice(fdc);
        debug_cpc("FDC anadida\n");
    }
}


bool CPC:: removeFDC() {
	if (fdc != nullptr) {
		discDrives[0]->eject();
		discDrives[1]->eject();

        for (u8 u = 0; u < 4; u++) {
			delete discDrives[u];
            discDrives[u] = nullptr;
        }

		io.removeDevice(fdc);

		delete fdc;
		fdc = nullptr;

		return true;
	}
	else return false;
}




bool CPC:: loadConfig(const std::string& fichero)
{
    debug_cpc("CPC:: loadConfig fichero=%s\n", fichero.c_str());
    bool datoObtenido;
    int numero;
    std::string cadena;
    Properties configuracion;
    
    datoObtenido = configuracion.load(fichero);

    if (datoObtenido) {
        configuracion.print();
    }
    else {
        debug_cpc("CPC:: loadConfig:: fichero \"%s\" no encontrado\n", fichero.c_str());
        return false;
    }

	if (configuracion.getNumber(TYPE_PROPERTY, &numero))  cpcType = static_cast<CPC_TYPE>(numero);

    // color monitor
    if (configuracion.getNumber(MON_PAL1_PROPERTY, &numero))  monitor.setPalette(numero);
    if (configuracion.getNumber(MON_PAL2_PROPERTY, &numero))  monitor.setPaletteVariant(numero);
	// scanlines
	if (configuracion.getNumber(MON_SCANLINES, &numero)) monitor.useScanlines(numero);

    // memoria extra
    if (configuracion.getNumber(RAM_PROPERTY, &numero)) {
        numero = std::min(static_cast<int>(Memory::MAX_PAGINAS_RAM), numero);
        numero = std::max(0, numero);
	    memory.addExtendedRamPages(static_cast<u8>(numero));
    }

    // fdc
    if (cpcType == CPC664  ||  cpcType == CPC6128)  addFDC();

    // lower rom
    if (configuracion.getString(LOWER_ROM_PROPERTY, cadena)) {
		memory.setLowerRom(cadena);
    }

    // upper roms
    std::string prefijo { UPPER_ROM_PROPERTY };
    std::string ficheroRom;

    for (u8 i = 0; i < Memory::MAX_ROMS; i++) {
        ficheroRom = prefijo + std::to_string(i);
        datoObtenido = configuracion.getString(ficheroRom, cadena); // s=nombre del fichero
        if (datoObtenido) {
            //debug_cpc("CPC:: upper_rom: %d,%s\n", i, cadena.c_str());
            memory.setUpperRom(i, cadena);
        }
    }
    return true;
}


bool CPC:: saveConfig(const std::string& fichero) {
	std::fstream f {fichero, std::ios::out};

    debug_cpc("cpc.loadConfig:: fichero=%s\n", fichero.c_str());
    bool obtenido;
    int n;
    std::string s;
    Properties configuracion;
    
	configuracion.setNumber(TYPE_PROPERTY, cpcType);
	configuracion.setNumber(RAM_PROPERTY, memory.getExtendedRamPages());
	configuracion.setNumber(MON_PAL1_PROPERTY, monitor.getPalette());
	configuracion.setNumber(MON_PAL2_PROPERTY, monitor.getPaletteVariant());
	configuracion.setNumber(MON_SCANLINES, monitor.getScanlines());
	configuracion.setString(LOWER_ROM_PROPERTY, memory.getLowerRomFile());

	std::string prefijo { UPPER_ROM_PROPERTY };
    std::string propiedad;

	for (u8 i = 0; i < Memory::MAX_ROMS; i++) {
		bool loaded;
		memory.getUpperRomFile(i, s, &loaded);  // r indica si se ha modificado el valor de s
		if (s != "") {
			propiedad = prefijo + std::to_string(i);
			configuracion.setString(propiedad, s);
		}
	}
	configuracion.save(fichero);
    return true;
}


void CPC:: reset() {
    cpu.reset();
    memory.reset();
    io.reset();
}



void CPC:: execute() {
    cpu.executeNextInstruction();
    io.update(cpu.getInstructionCycles());
}


DiscDrive* CPC:: getDiscDrive(u8 _unidad) { 
    if (_unidad < 4) return discDrives[_unidad]; 
    else return nullptr;
}



bool CPC:: loadSna(const std::string& snaFile) {
	debug_sna("SNA:: %s\n", snaFile.c_str());
	std::fstream fichero {snaFile, std::ios::in | std::ios::binary};
	SNA sna;
	sna.load(fichero);
	sna.print();

	//if (sna.cpcType == CPC464) set464();
	//if (sna.cpcType == CPC664) set664();
	//if (sna.cpcType == CPC6128) set6128();

	debug_sna("cpu\n");
	cpu.setSnaData(&sna.sna_z80, sna.snaVersion);
	debug_sna("crtc\n");
	crtc.setSnaData(&sna.sna_crtc, sna.snaVersion);
	debug_sna("ppi\n");
	ppi.setSnaData(&sna.sna_ppi);
	debug_sna("psg\n");
	psg.setSnaData(&sna.sna_psg);
	debug_sna("mem\n");

	//u8 ramExtPages = (sna.memoryDumpSize - 64) / 64;
	//memory.addExtendedRamPages(ramExtPages);
	memory.loadRam(fichero);

	debug_sna("ga\n");
	gatearray.setSnaData(&sna.sna_ga, sna.snaVersion);

	if (hasFDC() && sna.snaVersion == 3)  fdc->setSnaData(&sna.sna_fdc);

	fichero.close();
	debug_sna("SNA:: fin\n");
	return true;
}


bool CPC:: saveSna(const std::string& snaFile) {
	SNA sna;	
	std::fstream fichero {snaFile, std::ios::out | std::ios::binary};

	cpu.getSnaData(&sna.sna_z80);
	gatearray.getSnaData(&sna.sna_ga);
	crtc.getSnaData(&sna.sna_crtc);
	ppi.getSnaData(&sna.sna_ppi);
	psg.getSnaData(&sna.sna_psg);

	if (hasFDC())  fdc->getSnaData(&sna.sna_fdc);

	sna.cpcType = cpcType;
	sna.memoryDumpSize = memory.getExtendedRamPages() * 64 + 64;

	sna.print();
	sna.save(fichero);

	debug_sna("SNA:: grabar ram\n");
	memory.saveRam(fichero);

	fichero.close();
	debug_sna("SNA:: fin\n");

	return true;
}


