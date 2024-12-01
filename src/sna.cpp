/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Snapshot file data implementation                                         |
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


// https://www.cpcwiki.eu/index.php/Format:SNA_snapshot_file_format

#include <string>
#include <fstream>
#include "sna.h"
#include "z80.h"
#include "gatearray.h"
#include "crtc.h"
#include "fdc.h"
#include "ppi.h"
#include "psg.h"
#include "log.h"



void SNA_Z80:: load(std::fstream& f) {
	f.seekg(0x11, std::ios::beg);
	f.read(reinterpret_cast<char*>(&af), 2);
	f.read(reinterpret_cast<char*>(&bc), 2);
	f.read(reinterpret_cast<char*>(&de), 2);
	f.read(reinterpret_cast<char*>(&hl), 2);
	f.read(reinterpret_cast<char*>(&R), 1);
	f.read(reinterpret_cast<char*>(&I), 1);
	f.read(reinterpret_cast<char*>(&IFF1), 1);
	f.read(reinterpret_cast<char*>(&IFF2), 1);
	f.read(reinterpret_cast<char*>(&ix), 2);
	f.read(reinterpret_cast<char*>(&iy), 2);
	f.read(reinterpret_cast<char*>(&sp), 2);
	f.read(reinterpret_cast<char*>(&pc), 2);
	f.read(reinterpret_cast<char*>(&im), 1);
	f.read(reinterpret_cast<char*>(&af2), 2);
	f.read(reinterpret_cast<char*>(&bc2), 2);
	f.read(reinterpret_cast<char*>(&de2), 2);
	f.read(reinterpret_cast<char*>(&hl2), 2);
	f.seekg(0xB4, std::ios::beg);
	f.read(reinterpret_cast<char*>(&intReq), 1);
}

void SNA_Z80:: save(std::fstream& f) {
	f.seekp(0x11, std::ios::beg);
	f.write(reinterpret_cast<char*>(&af), 2);
	f.write(reinterpret_cast<char*>(&bc), 2);
	f.write(reinterpret_cast<char*>(&de), 2);
	f.write(reinterpret_cast<char*>(&hl), 2);
	f.write(reinterpret_cast<char*>(&R), 1);
	f.write(reinterpret_cast<char*>(&I), 1);
	f.write(reinterpret_cast<char*>(&IFF1), 1);
	f.write(reinterpret_cast<char*>(&IFF2), 1);
	f.write(reinterpret_cast<char*>(&ix), 2);
	f.write(reinterpret_cast<char*>(&iy), 2);
	f.write(reinterpret_cast<char*>(&sp), 2);
	f.write(reinterpret_cast<char*>(&pc), 2);
	f.write(reinterpret_cast<char*>(&im), 1);
	f.write(reinterpret_cast<char*>(&af2), 2);
	f.write(reinterpret_cast<char*>(&bc2), 2);
	f.write(reinterpret_cast<char*>(&de2), 2);
	f.write(reinterpret_cast<char*>(&hl2), 2);
	f.seekp(0xB4, std::ios::beg);
	f.write(reinterpret_cast<char*>(&intReq), 1);
}

void SNA_Z80:: print() {
	debug_sna("SNA:: Z80:  AF=%04X BC=%04X DE=%04X HL=%04X  IX=%04X IY=%04X  "
				"SP=%04X PC=%04X  AF'=%04X BC'=%04X DE'=%04X HL'=%04X  "
				"R=%02X I=%02X IM=%d IFF=%d,%d  ir=%d\n",
		af.w, bc.w, de.w, hl.w,  ix.w, iy.w,  sp.w, pc.w,  af2.w, bc2.w, de2.w, hl2.w,  
		R, I, im, IFF1, IFF2, intReq);
}



void SNA_GA:: load(std::fstream& f) {
	f.seekg(0x2E, std::ios::beg);
	f.read(reinterpret_cast<char*>(&selectedPen), 1);
	f.read(reinterpret_cast<char*>(&palette), 17);
	f.read(reinterpret_cast<char*>(&multiConfiguration), 1);
	f.read(reinterpret_cast<char*>(&ramConfiguration), 1);
	f.seekg(0x55, std::ios::beg);
	f.read(reinterpret_cast<char*>(&romSelection), 1);

	f.seekg(0xB2, std::ios::beg);
	f.read(reinterpret_cast<char*>(&syncDelayCounter), 1);
	f.read(reinterpret_cast<char*>(&r52), 1);
}

void SNA_GA:: save(std::fstream& f) {
	f.seekp(0x2E, std::ios::beg);
	f.write(reinterpret_cast<char*>(&selectedPen), 1);
	f.write(reinterpret_cast<char*>(&palette), 17);
	f.write(reinterpret_cast<char*>(&multiConfiguration), 1);
	f.write(reinterpret_cast<char*>(&ramConfiguration), 1);
	f.seekp(0x55, std::ios::beg);
	f.write(reinterpret_cast<char*>(&romSelection), 1);

	f.seekp(0xB2, std::ios::beg);
	f.write(reinterpret_cast<char*>(&syncDelayCounter), 1);
	f.write(reinterpret_cast<char*>(&r52), 1);
}

void SNA_GA:: print() {
	debug_sna("SNA:: GA:  pen=%d cfg=%02X ramCfg=%02X romSelect=%d palette=", 
			selectedPen, multiConfiguration, ramConfiguration, romSelection);
	for (u8 i = 0; i < 17; i++) debug("%d,", palette[i]);
	debug_sna("\n");
}



void SNA_CRTC:: load(std::fstream& f) {
	f.seekg(0x42, std::ios::beg);
	f.read(reinterpret_cast<char*>(&selectedRegister), 1);
	f.read(reinterpret_cast<char*>(&registerData), 18);
	f.seekg(0xA4, std::ios::beg);
	f.read(reinterpret_cast<char*>(&crtcType), 1);
	f.seekg(0xA9, std::ios::beg);
	f.read(reinterpret_cast<char*>(&hcc), 1); // horizontal character counter register
	f.seekg(0xAB, std::ios::beg);
	f.read(reinterpret_cast<char*>(&vcc), 1); // character-line counter register
	f.read(reinterpret_cast<char*>(&vlc), 1); // raster line counter??
	f.read(reinterpret_cast<char*>(&vtac), 1); // vertical total adjust counter register
	f.read(reinterpret_cast<char*>(&hsc), 1); // horizontal sync width counter
	f.read(reinterpret_cast<char*>(&vsc), 1); // vertical sync width counter

	u16 stateFlags ;
	f.read(reinterpret_cast<char*>(&stateFlags), 2); // 
	vsync = (stateFlags & 0x0001) ? true : false;
	hsync = (stateFlags & 0x0002) ? true : false;
	vtac = (stateFlags & 0x0080) ? 1 : 0;            //?????????????????????
}

void SNA_CRTC:: save(std::fstream& f) {
	f.seekp(0x42, std::ios::beg);
	f.write(reinterpret_cast<char*>(&selectedRegister), 1);
	f.write(reinterpret_cast<char*>(&registerData), 18);
	f.seekp(0xA4, std::ios::beg);
	f.write(reinterpret_cast<char*>(&crtcType), 1);
	f.seekp(0xA9, std::ios::beg);
	f.write(reinterpret_cast<char*>(&hcc), 1); // horizontal character counter register
	f.seekp(0xAB, std::ios::beg);
	f.write(reinterpret_cast<char*>(&vcc), 1); // character-line counter register
	f.write(reinterpret_cast<char*>(&vlc), 1); // raster line counter??
	f.write(reinterpret_cast<char*>(&vtac), 1); // vertical total adjust counter register
	f.write(reinterpret_cast<char*>(&hsc), 1); // horizontal sync width counter
	f.write(reinterpret_cast<char*>(&vsc), 1); // vertical sync width counter

	u16 stateFlags = 0;
	if (vsync) stateFlags |= 0x0001;
	if (hsync) stateFlags |= 0x0002;
	if (vtac != 0) stateFlags |= 0x0080;               //?????????????????????
	f.write(reinterpret_cast<char*>(&stateFlags), 2); // 
}

void SNA_CRTC:: print() {
	debug_sna("SNA:: CRTC: reg=%d data=", selectedRegister);
	for (u8 i = 0; i < CRTC_PALETTE_SIZE; i++) debug_sna("%d,", registerData[i]);
	debug_sna("\n");
}



void SNA_PPI:: load(std::fstream& f) {
	f.seekg(0x56, std::ios::beg);
	f.read(reinterpret_cast<char*>(&portA), 1);
	f.read(reinterpret_cast<char*>(&portB), 1);
	f.read(reinterpret_cast<char*>(&portC), 1);
	f.read(reinterpret_cast<char*>(&control), 1);
}

void SNA_PPI:: save(std::fstream& f) {
	f.seekp(0x56, std::ios::beg);
	f.write(reinterpret_cast<char*>(&portA), 1);
	f.write(reinterpret_cast<char*>(&portB), 1);
	f.write(reinterpret_cast<char*>(&portC), 1);
	f.write(reinterpret_cast<char*>(&control), 1);
}

void SNA_PPI:: print() {
	debug_sna("SNA:: PPI:  A=%02X B=%02X C=%02X control=%02X\n", portA, portB, portC, control);
}



void SNA_PSG:: load(std::fstream& f) {
	f.seekg(0x5A, std::ios::beg);
	f.read(reinterpret_cast<char*>(&selectedRegister), 1);
	f.read(reinterpret_cast<char*>(&registerData), 16);
}

void SNA_PSG:: save(std::fstream& f) {
	f.seekp(0x5A, std::ios::beg);
	f.write(reinterpret_cast<char*>(&selectedRegister), 1);
	f.write(reinterpret_cast<char*>(&registerData), 16);
}

void SNA_PSG:: print() {
	debug_sna("SNA:: PSG:  reg=%d data=", selectedRegister);
	// 16=PSG_NUM_REGISTERS
	for (u8 i = 0; i < PSG_NUM_REGISTERS; i++) debug_sna("%d,", registerData[i]);
	debug_sna("\n");
}



void SNA_FDC:: load(std::fstream& f) {
	f.seekg(0x9C, std::ios::beg);
	f.read(reinterpret_cast<char*>(&led), 1);
	f.read(reinterpret_cast<char*>(&currentTracks), 4);
}

void SNA_FDC:: save(std::fstream& f) {
	f.seekp(0x9C, std::ios::beg);
	f.write(reinterpret_cast<char*>(&led), 1);
	f.write(reinterpret_cast<char*>(&currentTracks), 4);
}

void SNA_FDC:: print() {
	debug_sna("SNA:: FDC  led=%d ", led);
	for (u8 i = 0; i < MAX_DRIVES; i++) debug_sna("%d,", currentTracks[i]);
	debug_sna("\n");
}




void SNA:: load(std::fstream& fichero) {
	fichero.seekg(0, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(&idFile), 8);

	fichero.seekg(0x10, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(&snaVersion), 1);

	fichero.seekg(0x6D, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(&cpcType), 1);

	fichero.seekg(0x6B, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(&memoryDumpSize), 2);

	sna_z80.load(fichero);
	sna_ga.load(fichero);
	sna_crtc.load(fichero);
	sna_ppi.load(fichero);
	sna_psg.load(fichero);
	sna_fdc.load(fichero);

	fichero.seekg(0x100, std::ios::beg);
}

void SNA:: save(std::fstream& fichero) {
	// creamos los 0x100 primeros bytes
	fichero.seekp(0, std::ios::beg);
	{
		char zz[0x100] = {0};
		fichero.write(reinterpret_cast<char*>(&zz), 0x100);
	}

	fichero.seekp(0, std::ios::beg);
	fichero.write(reinterpret_cast<char*>(&idFile), 8);

	fichero.seekp(0x10, std::ios::beg);
	debug_sna("SNA:: version=%d\n", snaVersion);
	fichero.write(reinterpret_cast<char*>(&snaVersion), 1);

	fichero.seekp(0x6B, std::ios::beg);
	fichero.write(reinterpret_cast<char*>(&memoryDumpSize), 2);

	fichero.seekp(0x6D, std::ios::beg);
	fichero.write(reinterpret_cast<char*>(&cpcType), 1);

	sna_z80.save(fichero);
	sna_ga.save(fichero);
	sna_crtc.save(fichero);
	sna_ppi.save(fichero);
	sna_psg.save(fichero);
	sna_fdc.save(fichero);

	fichero.seekp(0x100, std::ios::beg); // dejamos el cursor listo para grabar la memoria
}

void SNA:: print() {
	debug_sna("SNA::  version=%d  cpcType=%d  ram=%d\n", snaVersion, cpcType, memoryDumpSize);
	sna_z80.print();
	sna_ga.print();
	sna_crtc.print();
	sna_ppi.print();
	sna_psg.print();
	sna_fdc.print();
}


// -------------------------------------------

void Z80:: setSnaData(SNA_Z80* sna, u8 version) {
	regAF = sna->af;
	regBC = sna->bc;
	regDE = sna->de;
	regHL = sna->hl;
	regAF2 = sna->af2;
	regBC2 = sna->bc2;
	regDE2 = sna->de2;
	regHL2 = sna->hl2;
	regIX = sna->ix;
	regIY = sna->iy;
	regSP = sna->sp;
	regPC = sna->pc;

	IM = sna->im;
	R = sna->R;
	I = sna->I;

	IFF1_interrupcionesHabilitadas = sna->IFF1;
	IFF2_estadoAntIFF1 = sna->IFF2;

	if (version > 1) {
    	interrupcionSolicitada = sna->intReq;
	}
}

void Z80:: getSnaData(SNA_Z80* sna) {
	sna->af = regAF;
	sna->bc = regBC;
	sna->de = regDE;
	sna->hl = regHL;
	sna->af2 = regAF2;
	sna->bc2 = regBC2;
	sna->de2 = regDE2;
	sna->hl2 = regHL2;
	sna->ix = regIX;
	sna->iy = regIY;
	sna->sp = regSP;
	sna->pc = regPC;

	sna->im = IM;
	sna->R = R;
	sna->I = I;

	sna->IFF1 = IFF1_interrupcionesHabilitadas;
	sna->IFF2 = IFF2_estadoAntIFF1;

    sna->intReq = interrupcionSolicitada;
}

// -------------------------------------------

void GateArray:: setSnaData(SNA_GA* sna, u8 version) {
	tintaSeleccionada = sna->selectedPen;

	memcpy(coloresHardware, sna->palette, GA_PALETTE_SIZE);

	memoria->selectUpperRom(sna->romSelection);

	configureModeAndRoms(sna->multiConfiguration);

    // cambiar la configuracion de ram!!
    memoria->configRam(0, selectedRom = sna->ramConfiguration);

	if (version == 3) {
    	r52 = sna->r52;
		sna->syncDelayCounter; // ?
	}
}

void GateArray:: getSnaData(SNA_GA* sna) {
	sna->selectedPen = tintaSeleccionada & 0x1F; 
    // This byte is the last value written to this port. Bit 7,6,5 should be set to "0". 
    
	memcpy(sna->palette, coloresHardware, GA_PALETTE_SIZE);
    /*for (u8 c = 0; c < GA_PALETTE_SIZE; c++) {
        sna->palette[c] = coloresHardware[c];;// & 0x17;
        // these bytes should have bit 7=bit 6=bit 5="0". 
        // Bits 4..0 define the colour using the hardware colour code
    }*/

    sna->multiConfiguration = 0x80; // For CPCEMU compatibility, bit 7 should be set to "1" and bit 6 and bit 5 set to "0".
    if (cambioMode != 255) sna->multiConfiguration |= cambioMode;
	else                   sna->multiConfiguration |= screenMode;

	if (lowerRomDisabled)  sna->multiConfiguration |= 0b00000100;
	if (upperRomDisabled)  sna->multiConfiguration |= 0b00001000;
    
	sna->romSelection = selectedRom;
    sna->ramConfiguration = ramConfiguration;

	sna->r52 = r52;
}

// -------------------------------------------

void CRTC:: setSnaData(SNA_CRTC* sna, u8 version) {
    registroSeleccionado = sna->selectedRegister;

    memcpy(registros, sna->registerData, CRTC_NUM_REGISTERS);
	actualizarVariablesInternas();

	if (version == 3) {
		vcc = sna->vcc;
		vlc = sna->vlc; 
		vsc = sna->vsc; 
		vtac = sna->vtac; 
		hcc = sna->hcc;
		hsc = sna->hsc;

		vsync = sna->vsync;
		hsync = sna->hsync;
		sna->vsyncDelayCounter; // ??

		crtcType = sna->crtcType; 
	}
}

void CRTC:: getSnaData(SNA_CRTC* sna) {
	sna->selectedRegister = registroSeleccionado;

	memcpy(sna->registerData, registros, CRTC_NUM_REGISTERS);

	sna->vcc = vcc;
	sna->vlc = vlc; 
	sna->vsc = vsc; 
	sna->vtac = vtac; 
	sna->hcc = hcc;
	sna->hsc = hsc;

	sna->vsync = vsync;
	sna->hsync = hsync;
	sna->vsyncDelayCounter; // ???? que variable es??
	sna->crtcType = crtcType; // ?? poner el tipo de crtc
}

// -------------------------------------------

void PPI:: setSnaData(SNA_PPI* sna) {
	puertoA_val = sna->portA;
	puertoB_val = sna->portB;
	puertoC_val = sna->portC;

	setControlValue(sna->control);

	debug_sna("SNA:: PPI: A=%02X B=%02X C=%02X CONTROL=%02X  a=%d b=%d  ch=%d cl=%d  ga=%d gb=%d\n",
				puertoA_val, puertoB_val, puertoC_val, sna->control,
				puertoA_dir, puertoB_dir, puertoCh_dir, puertoCl_dir, grupoA_modo, grupoB_modo);
}


void PPI:: getSnaData(SNA_PPI* sna) {
	sna->portA = puertoA_val;
	sna->portB = puertoB_val;
	sna->portC = puertoC_val;
	sna->control = getControlValue();

	debug_sna("SNA:: PPI: A=%02X B=%02X C=%02X CONTROL=%02X  a=%d b=%d  ch=%d cl=%d  ga=%d gb=%d\n",
				puertoA_val, puertoB_val, puertoC_val, sna->control,
				puertoA_dir, puertoB_dir, puertoCh_dir, puertoCl_dir, grupoA_modo, grupoB_modo);
}

// -------------------------------------------

void PSG:: setSnaData(SNA_PSG *sna) {
	sna->selectedRegister = registroSeleccionado;

	memcpy(registros, sna->registerData, PSG_NUM_REGISTERS);
}

void PSG:: getSnaData(SNA_PSG *sna) {
	registroSeleccionado = sna->selectedRegister;

	memcpy(sna->registerData, registros, PSG_NUM_REGISTERS);
}

// -------------------------------------------

