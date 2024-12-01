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

#pragma once

// https://www.cpcwiki.eu/index.php/Format:SNA_snapshot_file_format


#include "tipos.h"
#include "log.h"
#include "target.h"



typedef struct {
    WORD af, bc, de, hl;
    BYTE R, I;					// registros I R, Rb7 es ek bit 7, I vector de interrupciones
    bool IFF1, IFF2;
    WORD ix, iy;
    WORD sp, pc;
    bool im = false;
    WORD af2, bc2, de2, hl2;
	bool intReq;

	void load(std::fstream&f);
	void save(std::fstream& f);

	#if TARGET_PC
	void print();
	#endif
} SNA_Z80;


typedef struct {
	BYTE selectedPen;
	BYTE palette[17];
	BYTE multiConfiguration; // mode,high-rom,low-rom
	BYTE ramConfiguration; // parece que no se utiliza
	BYTE romSelection; // supongo que se refiere a la upper-rom
	BYTE syncDelayCounter;
	BYTE r52;

	void load(std::fstream& f);
	void save(std::fstream& f);
	void print();
} SNA_GA;


typedef struct {
	BYTE selectedRegister;
	BYTE registerData[18];

	BYTE vcc, vlc, vsc, vtac, hcc, hsc, vma, vdur;
	bool vsync, hsync;
	BYTE vsyncDelayCounter;
	BYTE crtcType;

	void load(std::fstream& f);
	void save(std::fstream& f);
	void print();
} SNA_CRTC;


typedef struct {
	BYTE portA;
	BYTE portB;
	BYTE portC;
	BYTE control;

	void load(std::fstream& f);
	void save(std::fstream& f);
	void print();
} SNA_PPI;


typedef struct {
	BYTE selectedRegister;
	BYTE registerData[16];

	void load(std::fstream& f);
	void save(std::fstream& f);
	void print();
} SNA_PSG;


typedef struct {
	bool led;
	BYTE currentTracks[4]; // MAX_DRIVES

	void load(std::fstream& f);
	void save(std::fstream& f);
	void print();
} SNA_FDC;



typedef struct
{
	BYTE idFile[8] = {'M','V',' ','-',' ','S','N','A'};
	BYTE snaVersion = 3;
	BYTE cpcType;
	u16 memoryDumpSize;  // en Kb

	SNA_Z80 sna_z80;
	SNA_GA sna_ga;
	SNA_CRTC sna_crtc;
	SNA_PPI sna_ppi;
	SNA_PSG sna_psg;
	SNA_FDC sna_fdc;

	void load(std::fstream& fichero);
	void save(std::fstream& fichero);
	void print();
} SNA;


