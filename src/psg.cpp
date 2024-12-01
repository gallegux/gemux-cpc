/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  PSG implementation                                                        |
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


#include <cstring> // para memcpy
#include <stdio.h>
#include <SDL2/SDL_mixer.h>
#include "tipos.h"
#include "psg.h"
#include "keyboard.h"
#include "sna.h"
#include "log.h"


PSG:: PSG(Keyboard* _teclado) {
    //debug_psg("PSG\n");
    teclado = _teclado;
}

Keyboard* PSG:: getKeyboard() { return teclado; }


void PSG:: update(u8 ciclos) {
    //debug("PSG:: update()\n");
}

//BYTE PSG:: getRegisterValue(u8 numRegistro) {
//    return registros[numRegistro];
//}

/**
https://www.cpcwiki.eu/index.php/How_to_access_the_PSG_via_PPI

Register selection
------------------
To write data to the PSG, PPI Port A must be operating as output. (See the document on the 8255 to see how to do this)
To select a register, write the register number into PPI Port A, then set bit 7 and bit 6 of Port C to "1".
The register will now be selected and the user can read from or write to it.
The register will remain selected until another is chosen.
== pasos: A=[modo_output], A=num_registro, C=C0 --> y el registro queda seleccionado

Writing to the selected PSG register
------------------------------------
To write data to the PSG, PPI Port A must be operating as output. (See the document on the 8255 to see how to do this)
To write data into the selected PSG register:
write data to PPI Port A, set bit 7 to "1" and bit 6 to "0" of PPI Port C The data will then be written into the register.
== pasos: A=[modo_output], A=num_registro, C=C0, A=dato, C=80

Reading from the selected PSG register
--------------------------------------
To read data from the PSG, PPI Port A must be operating as input. (See the document on the 8255 to see how to do this)
To read data from the selected register:
set bit 7 to "0" and bit 6 to "1" of PPI Port C read data from PPI Port A, The register data is available at PPI Port A.
== pasos: A=[modo_output], A=num_registro, C=C0, A=[modo_input], C=04, y el dato esta en A listo para un IN
*/

void PSG:: selectRegister(u8 numRegistro) {
    //inactivo = false;
    registroSeleccionado = numRegistro;
    //if (numRegistro == 14) debug_psg("PSG::  registro_seleccionado=%d\n", registroSeleccionado);
}

u8 PSG:: getSelectedRegister() {
    return registroSeleccionado;
}


void PSG:: writeR7(BYTE dato) {
    registros[7] = dato; // mantenemos el dato para el sna
    toneA_flag = dato & R7_CHANNEL_A_TONE; // 1=off 0=on
    toneB_flag = dato & R7_CHANNEL_B_TONE; // 1=off 0=on
    toneC_flag = dato & R7_CHANNEL_C_TONE; // 1=off 0=on
    noiseA_flag = dato & R7_CHANNEL_A_NOISE; // 1=off 0=on
    noiseB_flag = dato & R7_CHANNEL_B_NOISE; // 1=off 0=on
    noiseC_flag = dato & R7_CHANNEL_C_NOISE; // 1=off 0=on
	// los bits 6 y 7 siempre estan a 0
}


// en el registro del teclado no se escribe desde codigo, solo se lee
void PSG:: writeSelectedRegister(BYTE dato) {
//	if (!active) {
//		active = true;
		if (registroSeleccionado < PSG_NUM_REGISTERS) {
			//debug_psg("PSG::  write R%d=%02X\n", registroSeleccionado, dato);
			//registros[registroSeleccionado] = *portA_value;
			//if (registroSeleccionado == PSG_REGISTRO_TECLADO)  teclado->seleccionarFila(puertoA_val);
			if (registroSeleccionado == 7) writeR7(dato);
			else if (registroSeleccionado == PSG_KEYBOARD_REGISTER) teclado->selectRow(dato & 0x0F);
			//else if (registroSeleccionado == PSG_REGISTRO_TECLADO) {
			//	registros[registroSeleccionado] = dato;
			//	teclado->selectRow(dato & 0x0F);
			//}
			else registros[registroSeleccionado] = dato;
		}
//	}
}


void PSG:: readSelectedRegister(BYTE *puertoA_val) {
	//if (!active) {
		//active = true;
		if (registroSeleccionado >= PSG_NUM_REGISTERS) return;

		if (registroSeleccionado == PSG_KEYBOARD_REGISTER) {
			*puertoA_val = teclado->getSelectedRowStatus();
			//debug_psg("PSG:: readSelectedRegister1  fila=%d  ret=%02X\n", teclado->getSelectedRow(), *puertoA_val);
			return;
		}
		// en el caso del teclado este metodo es invocado despues de readSelectedRegister(BYTE, BYTE*)
		// con lo que registros[registroSeleccionado] ya tiene el valor
		*puertoA_val = registros[registroSeleccionado];
	//}
}


void PSG:: readSelectedRegister(BYTE dato, BYTE *puertoA_val) {
//	if (!active) {
//		active = true;
		if (registroSeleccionado >= PSG_NUM_REGISTERS) return;

		if (registroSeleccionado == PSG_KEYBOARD_REGISTER) {
			*puertoA_val = teclado->getRowStatus(dato & 0x0F);
			debug_psg("PSG:: readSelectedRegister2  fila=%d  ret=%02X\n", dato & 0x0F, *puertoA_val);
		}
		else {
			// TODO, por si el chip AY devuelve 
			*puertoA_val = registros[registroSeleccionado]; 
		}
//	}
}

// hasta que punto es necesario implementar esta funcion?
// posiblemente, despues de hacer una operacion haya que hacer un inactive() para poder hacer otra operacion.
void PSG:: setInactive() { active = false; }


void PSG:: print() {
    debug_psg("PSG:: ");
    for (u8 i = 0; i < PSG_NUM_REGISTERS; i++) {
        debug_psg(" R%02X=%02X", i, registros[i]);
    }
    debug_psg("\n");
}



void PSG:: init() {}


void PSG:: reset() {
	active = false;

    for (int i = 0; i < 16; i++) registros[i] = 0;
    writeR7(0xFF);
    registroSeleccionado = 0xFF;
}

