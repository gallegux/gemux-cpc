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

#include <stdio.h>
#include "tipos.h"
#include "ppi.h"
#include "tape.h"
#include "log.h"

// https://www.cpcwiki.eu/index.php/8255



/**
Register selection
------------------
To write data to the PSG, PPI Port A must be operating as output. (See the document on the 8255 to see how to do this)
To select a register, write the register number into PPI Port A, then set bit 7 and bit 6 of Port C to "1".
The register will now be selected and the user can read from or write to it.
The register will remain selected until another is chosen.

Writing to the selected PSG register
------------------------------------
To write data to the PSG, PPI Port A must be operating as output. (See the document on the 8255 to see how to do this)
To write data into the selected PSG register:
write data to PPI Port A, set bit 7 to "1" and bit 6 to "0" of PPI Port C The data will then be written into the register.

Reading from the selected PSG register
--------------------------------------
To read data from the PSG, PPI Port A must be operating as input. (See the document on the 8255 to see how to do this)
To read data from the selected register:
set bit 7 to "0" and bit 6 to "1" of PPI Port C read data from PPI Port A, The register data is available at PPI Port A.
*/

PPI:: PPI(PSG* _psg, GateArray* _gatearray, CRTC* _crtc, Tape* _tape) {
    psg = _psg;
    gatearray = _gatearray;
    crtc = _crtc;
    tape = _tape;

    reset();
}


void PPI:: reset() {
    puertoA_val = puertoB_val = puertoC_val = 0;
    puertoA_dir = puertoB_dir = puertoCh_dir = puertoCl_dir = PPI_INPUT;
	grupoA_modo = grupoB_modo = 0;
	levelToWrite = false;
}


void PPI:: update(u8 ciclos) {
    tape->update(ciclos, levelToWrite);
    psg->update(ciclos);

	//debug_ppi("PPI:: dA=%d dB=%d dCH=%d dCL=%d\n", puertoA_dir, puertoB_dir, puertoCh_dir, puertoCl_dir);
}



bool PPI:: OUT(WORD puerto, BYTE dato) {
	
    switch (puerto.b.h) {
//-----------------------------------------------------------------------------
		case 0xF4:
			//if (dato < 0x41 || dato > 0x49) 
			//debug_ppi("PPI:: OUT %02X, %02X\n", puerto.b.h, dato);
			if (puertoA_dir == PPI_OUTPUT)  puertoA_val = dato; // colocar el dato en el puerto A
            return true;
//-----------------------------------------------------------------------------
		case 0xF5:
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! OUT F5\n");
			if (puertoB_dir == PPI_OUTPUT)  puertoB_val = dato;
			return true;
//-----------------------------------------------------------------------------
		case 0xF6: 
			// invocando a este OUT con la fila, y luego un IN #F4, se utilza la escritura en la pantalla del OS
			//debug_ppi("PPI:: OUT %02X, %02X\n", puerto.b.h, dato);
			puertoC_val = dato;
			//if (dato == 0x58) debug_ppi("PPI:: OUT F6=58  CH_dir=%d  CL_dir=%d\n", puertoCh_dir, puertoCl_dir);
			modifyPortC();		
			return true;
//-----------------------------------------------------------------------------
		case 0xF7:
			//debug_ppi("PPI:: OUT %02X, %02X\n", puerto.b.h, dato);
			if (dato >= 0x80) { // bit 7=1
				// reset
				// CAUTION: Writing to PIO Control Register (with Bit7 set), automatically resets PIO Ports A,B,C to 00h each! ??????????
				puertoA_val = puertoB_val = puertoC_val = 0;
				setControlValue(dato);
			}
			else { // bit 7 == 0  para poner un bit especifico del puerto C a 1 o 0,
				u8 numeroBit = (dato & 0b00001110) >> 1;
				u8 mascaraBit = 1 << numeroBit;
				//u8 mascaraBit = (dato & 1) << numeroBit; //algunos juegos cargan con esto ¿?

				//if (numeroBit == 4)  debug_ppi("PPI:: mascara=%02X  C=%02X  ", mascaraBit, puertoC_val);
				//debug_ppi("PPI:: bit=%d  num_bit=%d  mascara=%02X  C=%02X -> ", dato & 1, numeroBit, mascaraBit, puertoC_val);
				if (dato & 1) /* set   */  puertoC_val |= mascaraBit; 
				else          /* reset */  puertoC_val &= (~mascaraBit);   

				if (numeroBit == 4) debug_ppi("PPI:: motor=%d\n", dato & 1);
				//debug_ppi("%02X  \n", puertoC_val);
				modifyPortC();
			}
			return true;
//-----------------------------------------------------------------------------
    }

    return false;
}


void PPI:: modifyPortC() {
	//debug_ppi(" %d,%d> ", puertoCh_dir == PPI_OUTPUT, (puertoC_val & CASSETTE_WRITE_DATA)!=0);
	//debug_ppi("PPI:: modify_port_c  %d %d\n", puertoCh_dir, puertoCl_dir);
	if (puertoCh_dir == PPI_OUTPUT) {
		levelToWrite = (puertoC_val & CASSETTE_WRITE_DATA);
		bool casseteMotorControl = (puertoC_val & CASSETTE_MOTOR_ON) != 0;
		tape->setMotorStatus(casseteMotorControl); 
		//debug_ppi("PPI:: modify_port_c  motor=%d\n", casseteMotorControl);

		switch (puertoC_val & PSG_FUNCTION_MASK) {
			case PSG_SELECT_REGISTER:
				/*if (puertoA_dir == PPI_OUTPUT)*/psg->selectRegister(puertoA_val);
				break;
			case PSG_WRITE_SELECTED_REGISTER:
				/*if (puertoA_dir == PPI_OUTPUT)*/psg->writeSelectedRegister(puertoA_val);
				break;
			//case PSG_READ_SELECTED_REGISTER:
			//	if (puertoA_dir == PPI_INPUT) psg->readSelectedRegister(puertoC_val/*fila*/, &puertoA_val);
			//	break;
			// este codigo deja de tener sentido ya que que en IN_F4 se llega a hacer lo mismo
		}
	}		
	if (puertoCl_dir == PPI_OUTPUT) {
		psg->getKeyboard()->selectRow(puertoC_val);
				
		//switch (puertoC_val & PSG_FUNCTION_MASK) {
		//	case PSG_SELECT_REGISTER:
		//		if (puertoA_dir == PPI_OUTPUT) psg->selectRegister(puertoA_val);
		//		break;
		//	case PSG_WRITE_SELECTED_REGISTER:
		//		if (puertoA_dir == PPI_OUTPUT) psg->writeSelectedRegister(puertoA_val);
		//		break;
		//	case PSG_READ_SELECTED_REGISTER:
		//		//if (dato == 0x58) debug_ppi("PPI:: read_selected_register %02X = ", puertoC_val & 0x0F);
		//		//if (puertoC_val == 0x58) debug_ppi("PPI:: \n");
		//		if (puertoA_dir == PPI_INPUT) psg->readSelectedRegister(puertoC_val/*fila*/, &puertoA_val);
		//		//if (dato == 0x58) debug_ppi("%02X \n", puertoA_val);
		//		break;
		//}
	}
}



bool PPI:: IN(WORD puerto, BYTE* dato) {
    
    switch (puerto.b.h) {
//-----------------------------------------------------------------------------
        case 0xF4:
			/*
			0Eh - External Dataregister Port A
			This register receives data from the CPC keyboard (or joystick), for more information read the chapter 
			about the CPC Keyboard Matrix. This register can be also used as output port by setting bit 6 of the 
			PSG control register to 1 (that would allow to use the six data pins of the joystick connector to 
			output data to external hardware).
			0Fh - External Dataregister Port B
			This register is not used in CPC computers. In detail, a AY-3-8910 sound chip would have external 
			connectors for this register, so that it could be used as a further IO port, but the CPC's sound chip
			(AY-3-8912, in 28 pin package) doesn't have such connectors, even though the register still does exist
			internally. The Aleste 520EX (russian CPC clone) is a special case: does have a 8910 chip, 
			with PSG Port B being used as 8bit printer port data.
			*/
			if (puertoA_dir == PPI_INPUT) {
				if ((puertoC_val & PSG_FUNCTION_MASK) == PSG_READ_SELECTED_REGISTER) {
					psg->readSelectedRegister(dato);
				}
				else  *dato = 0xFF;
			}
			else  *dato = puertoA_val;
            return true;
//-----------------------------------------------------------------------------
        case 0xF5:
			if (puertoB_dir == PPI_INPUT) {
            // https://www.cpcwiki.eu/index.php/8255
				*dato =   PRINTER_BUSY            // bit 6
						| REFRESH_RATE_50HZ     // bit 4
						| AMSTRAD               // bits 3,2,1
						;
				if (gatearray->getVSync())  *dato |= VSYNC_ACTIVE;  // bit 0
				if (tape->getLevel())       *dato |= CAS_IN_HIGH_LEVEL; // bit 7
			}
			else *dato = puertoB_val;

            return true;
//-----------------------------------------------------------------------------
        case 0xF6:
			//debug_ppi("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PPI. IN F6\n");
            // segun la documentacion no deberia invocarse (pero se invoca)
            // I/O address  A9	A8  Description	Read/Write status   Used Direction  Used for
            // &F6xx	    1	0   Port C Data	Read/Write	        Out	            KeybRow/CasOut/PSG
            if (puertoCh_dir == PPI_INPUT) {
	            *dato = puertoC_val & 0x0F; // se conservan los bits 3210
				*dato |= puertoC_val & PSG_FUNCTION_SELECTION;
				//*dato |= CASSETTE_WRITE_DATA;
				if (tape->getMotorStatus()) *dato |= CASSETTE_MOTOR_ON;
			}
			else *dato = 0xFF; // si no sale Read error b

            return true;
//-----------------------------------------------------------------------------
        case 0xF7:
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IN F7\n");
//-----------------------------------------------------------------------------
    }
    return false;
}



BYTE PPI:: getControlValue() {
    BYTE control = 0;

	if (puertoA_dir == PPI_INPUT)  control |= PPI_PORT_A_MASK;
	if (puertoB_dir == PPI_INPUT)  control |= PPI_PORT_B_MASK;
	if (puertoCh_dir == PPI_INPUT) control |= PPI_PORT_Ch_MASK;
	if (puertoCl_dir == PPI_INPUT) control |= PPI_PORT_Cl_MASK;

	control |= (grupoB_modo << 2);
	control |= (grupoA_modo << 5);

    return control;
}


void PPI:: setControlValue(BYTE control) {
   	puertoA_dir  = (control & PPI_PORT_A_MASK) ? PPI_INPUT : PPI_OUTPUT;
	puertoB_dir  = (control & PPI_PORT_B_MASK) ? PPI_INPUT : PPI_OUTPUT;
	puertoCh_dir = (control & PPI_PORT_Ch_MASK) ? PPI_INPUT : PPI_OUTPUT;
	puertoCl_dir = (control & PPI_PORT_Cl_MASK) ? PPI_INPUT : PPI_OUTPUT;

	grupoB_modo = (control && PPI_GROUP_B_MASK) >> 2;                
	grupoA_modo = (control && PPI_GROUP_A_MASK) >> 5;

	#ifdef debug_cdt
	if (grupoA_modo != 0  ||  grupoB_modo != 0) 
		debug("PPI:: gA=%d gB=%d\n", grupoA_modo, grupoB_modo);
	#endif
}



#define strDir(x) (x == PPI_INPUT ? 'i' : 'o')

void PPI:: print() {
    debug_ppi("<%02X %02X %02X %c %c %c%c>", puertoA_val, puertoB_val, puertoC_val, 
        strDir(puertoA_dir), strDir(puertoB_dir), strDir(puertoCh_dir), strDir(puertoCl_dir));
}



