/*******************************************************************************
    GEMUX-CPC - Amstrad CPC emulator
    PPI implementation

    Copyright (c) 2024 Gallegux (gallegux@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <http://www.gnu.org/licenses/>.

    If you use this code, please attribute the original source by mentioning
    the author and providing a link to the original repository.
*******************************************************************************/

#include <stdio.h>
#include "tipos.h"
#include "ppi.h"
#include "unidad_cinta.h"
#include "log.h"

// https://www.cpcwiki.eu/index.php/8255

#define PRINTER_BUSY 0x40
#define AMSTRAD 0x0E
#define REFRESH_RATE_50HZ 0x10

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

PPI:: PPI(PSG* _psg, GateArray* _gatearray, CRTC* _crtc, UnidadCinta* _unidadCinta) {
    debug_ppi("PPI\n");
    psg = _psg;
    gatearray = _gatearray;
    crtc = _crtc;
    unidadCinta = _unidadCinta;

    reset();
}


void PPI:: reset() {
    puertoA_dir = PPI_OUTPUT;  // input/output
    puertoA_val = 0;

    puertoB_dir = PPI_INPUT;
    puertoB_val = 0;
    
    puertoCh_dir = PPI_INPUT;
    puertoCl_dir = PPI_INPUT;
    
    puertoC_val = 0;
}


void PPI:: update(u8 ciclos) {
    unidadCinta->update(ciclos);
    psg->update(ciclos);
}


bool PPI:: OUT(WORD puerto, BYTE dato) {
    switch (puerto.b.h) {
        case 0xF4:
            if (puerto.b.h >= 41 && puerto.b.h <= 49)
                debug_ppi("PPI::  OUT %02X, %02X\n", puerto.b.h, dato);
            if (puertoA_dir == PPI_OUTPUT) puertoA_val = dato; // colocar el dato en el puerto A
            return true;

        case 0xF5:
            debug_ppi("PPI::  OUT %02X, %02X\n", puerto.b.h, dato); 
            puertoB_val = dato;
            
            return true;

        case 0xF6: {
            if (puerto.b.h >= 41 && puerto.b.h <= 49)
                debug_ppi("PPI::  OUT %02X, %02X\n", puerto.b.h, dato); 

            if (puertoA_dir == PPI_OUTPUT) {
                if (dato & 0x10) unidadCinta->setMotorStatus(true); 
                // Cassette Write data = dato  & 0x20;
            }

            u8 funcionPSG = dato & PSG_FUNCTION_MASK;
            if (puertoA_dir == PPI_OUTPUT  &&  funcionPSG == PSG_SELECT_REGISTER) {
                if (puertoA_val != PSG_REGISTRO_TECLADO)
                    debug_ppi("PPI:: seleccionar_registro %d\n", puertoA_val);
                psg->select(puertoA_val);
            }
            else if (puertoA_dir == PPI_OUTPUT  &&  funcionPSG == PSG_WRITE_SELECTED_REGISTER) {
                debug_ppi("PPI:: escribir_registro %d\n", puertoA_val);
                psg->write(/*dato,*/ puertoA_val);
            }
            else if (puertoA_dir == PPI_INPUT  &&  funcionPSG == PSG_READ_SELECTED_REGISTER) {
                psg->read(dato/*fila*/, &puertoA_val);
                if (psg->selected() != PSG_REGISTRO_TECLADO) debug_ppi("PPI:: leer_registro %d\n", puertoA_val);
            }
            else {
                //debug_ppi("PPI:: inactivo\n");
                psg->inactive();
            }



            return true;
        }

        case 0xF7:
            //debug_ppi("PPI::  OUT %02X, %02X\n", puerto.b.h, dato); FLUSH;
            if ( (dato & 0xFF) == 0x80) { // bit 7=1
                // reset
                // CAUTION: Writing to PIO Control Register (with Bit7 set), automatically resets PIO Ports A,B,C to 00h each! ??????????
                puertoA_val = puertoB_val = puertoC_val = 0;
                puertoA_dir = puertoB_dir = puertoCh_dir = puertoCl_dir = PPI_OUTPUT;
            }
            else if (dato > 0x80) { // bit 7=1
                /* In the CPC only Bit 4 is of interest, all other bits are always having the same value. 
                In order to write to the PSG sound registers, a value of 82h must be written to this register. 
                In order to read from the keyboard (through PSG register 0Eh), a value of 92h must be written to this register. */
                puertoA_dir  = (dato & 0x10) ? PPI_INPUT : PPI_OUTPUT;
                puertoB_dir  = (dato & 0x02) ? PPI_INPUT : PPI_OUTPUT;
                puertoCh_dir = (dato & 0x08) ? PPI_INPUT : PPI_OUTPUT;
                puertoCl_dir = (dato & 0x01) ? PPI_INPUT : PPI_OUTPUT;
            }
            else { // bit 7 == 0
                // para poner un bit especifico del puerto C a 1 o 0, o dicho de otra manera,
                // para configurar uno de los puertos
                u8 numeroBit = (dato & 0b00001110)> 1;
                u8 mascaraBit = dato & 1;
                mascaraBit = mascaraBit << numeroBit;

                if (numeroBit) { // set
                    puertoC_val |= mascaraBit;
                }
                else { // reset
                    mascaraBit = ~mascaraBit;
                    puertoC_val &= mascaraBit;    
                }
            }
            return true;
    }

    return false;
}




bool PPI:: IN(WORD puerto, BYTE* dato) {
    //debug_ppi("PPI.  IN %04X\n", puerto.w);
    switch (puerto.b.h) {
        case 0xF4:
            //debug_ppi("PPI. in F4\n"); 
            if (puertoA_dir == PPI_INPUT) { //  &&  puertoA_rw == PPI_READ) {
                switch (psg->selected()) {
                    case 14:
                        *dato = puertoA_val;
                        if (psg->registerValue(7) & R7_PORT_A_DIRECTION == R7_PORT_A_INPUT)
                            *dato |= psg->registerValue(14);
                        break;
                    case 15:
                        if (psg->registerValue(7) & R7_PORT_B_DIRECTION != R7_PORT_B_INPUT)
                            *dato = psg->registerValue(15);
                        break;
                    default:
                        *dato = psg->registerValue(psg->selected());
                        break;
                }                
            }
            return true;

        case 0xF5:
            if (puertoB_dir == PPI_INPUT) {
                // https://www.cpcwiki.eu/index.php/8255
                *dato = PRINTER_BUSY            // bit 6
                    | AMSTRAD                   // bits 3,2,1
                    | REFRESH_RATE_50HZ;  // bit 8

                if (gatearray->getVSync()) *dato |= /*(*dato) |*/ 0x01;
                if (unidadCinta->getBit()) *dato |= 0x80;
            }
            else {
                *dato = puertoB_val;
            }

            //if (puertoA_dir == PPI_INPUT  &&  (*dato & PSG_FUNCTION_MASK) == PSG_READ_SELECTED_REGISTER) {
            //    debug("PPI:: IN puerto F5 dentro del IF\n");
            //    psg->read(*dato, &puertoA_val);
            //}

            //debug_ppi("PPI:: in F5 = %02X\n", *dato);
            return true;

        case 0xF6:
            *dato = puertoC_val;
            if (puertoCh_dir == PPI_INPUT) {
                *dato &= 0x0F;

                if (puertoC_val & 0xC0 == 0xC0) *dato |= 0x80;  // change to psg write register
                else *dato |= 0xC0;
                *dato |= 0x20; // tape write
                if (unidadCinta->getMotorStatus()) *dato |= 0x10;
            }
            if (puertoCl_dir == PPI_OUTPUT) {
                *dato |= 0x0F;
            }

            return true;
    }
    return false;
}


char direccion(bool d) {
    return (d == PPI_INPUT) ? 'i' : 'o';
}

void PPI:: print() {
    debug_ppi("<%02X %02X %02X %c %c %c%c>", puertoA_val, puertoB_val, puertoC_val, 
        direccion(puertoA_dir), direccion(puertoB_dir), direccion(puertoCh_dir), direccion(puertoCl_dir));
}