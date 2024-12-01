/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  PPI implementation                                                        |
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
#include "psg.h"
#include "crtc.h"
#include "tape.h"
#include "gatearray.h"
#include "sna.h"



class PPI : public IO_Device
{

public:
	static constexpr BYTE PRINTER_BUSY      = 0x40;
	static constexpr BYTE AMSTRAD           = 0x0E;
	static constexpr BYTE REFRESH_RATE_50HZ = 0x10;

	static constexpr BYTE CAS_IN_HIGH_LEVEL = 0x80;
	static constexpr BYTE VSYNC_ACTIVE      = 0x01;

	static constexpr BYTE CASSETTE_MOTOR_ON       = 0x10;
	static constexpr BYTE CASSETTE_WRITE_DATA     = 0x20;
	static constexpr BYTE PSG_FUNCTION_SELECTION  = 0xC0;

	static constexpr bool PPI_INPUT  = true;
	static constexpr bool PPI_OUTPUT = false;
	static constexpr bool PPI_READ   = true;
	static constexpr bool PPI_WRITE  = false;

	// para el registro de control
	static constexpr BYTE PPI_PORT_A_MASK  = 0x10;
	static constexpr BYTE PPI_PORT_B_MASK  = 0x02;
	static constexpr BYTE PPI_PORT_Ch_MASK = 0x08;
	static constexpr BYTE PPI_PORT_Cl_MASK = 0x01;

	static constexpr BYTE PPI_GROUP_B_MASK = 0x04;
	static constexpr BYTE PPI_GROUP_A_MASK = 0x60;


    GateArray* gatearray;
    CRTC* crtc;
    PSG* psg;
    Tape* tape;

    bool puertoA_dir;  // input/output
    BYTE puertoA_val;

    bool puertoB_dir;
    BYTE puertoB_val;
    
    bool puertoCh_dir; // I/O de los 4 bits mas altos
    bool puertoCl_dir; // I/O de los 4 bits mas bajos
    BYTE puertoC_val;
    
	// modo de los puertos, posiblemente no hagan falta
	/* Group Mode 1 (Strobed Input/Output) and Group Mode 2 (Bi-Directional Bus), 
	   as far as I know, are not used by any program. Group Mode 0 (Basic Input/Output) is always used.*/
    u8 grupoB_modo;
	u8 grupoA_modo; // valores de 0 a 2

	bool levelToWrite; // level a escribir en la cinta
    
public:

    PPI(PSG* _psg, GateArray* _gatearray, CRTC* _crtc, Tape* _tape);

    bool OUT(WORD puerto, BYTE dato) override;
    bool IN(WORD puerto, BYTE* dato) override;
    void reset() override;
    void update(u8 ciclos) override;
  
    void print();

    void modifyPortC();

    BYTE getControlValue();
    void setControlValue(BYTE control);

	void setSnaData(SNA_PPI* sna);
	void getSnaData(SNA_PPI* sna);

};
