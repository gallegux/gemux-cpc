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


#pragma once


#include "tipos.h"
#include "io_device.h"
#include "keyboard.h"
#include "tape.h"
#include "sna.h"


// http://cpcwiki.eu/index.php/PSG

constexpr BYTE PSG_FUNCTION_MASK           = 0b11000000;
constexpr BYTE PSG_INACTIVE                = 0;
constexpr BYTE PSG_READ_SELECTED_REGISTER  = 0b01000000; // 1
constexpr BYTE PSG_WRITE_SELECTED_REGISTER = 0b10000000; // 2
constexpr BYTE PSG_SELECT_REGISTER         = 0b11000000; // 3

constexpr u8 PSG_NUM_REGISTERS = 16;

constexpr u8 PSG_KEYBOARD_REGISTER = 0x0E;

// Registro 7, http://cpcwiki.eu/index.php/PSG#07h_-_Mixer_Control_Register
#define PSG_R7 registros[7]
constexpr BYTE R7_CHANNEL_A_TONE   = 0x01;    // 1=off 0=on
constexpr BYTE R7_CHANNEL_B_TONE   = 0x02;    // 1=off 0=on
constexpr BYTE R7_CHANNEL_C_TONE   = 0x04;    // 1=off 0=on
constexpr BYTE R7_CHANNEL_A_NOISE  = 0x08;    // 1=off 0=on
constexpr BYTE R7_CHANNEL_B_NOISE  = 0x10;    // 1=off 0=on
constexpr BYTE R7_CHANNEL_C_NOISE  = 0x20;    // 1=off 0=on
constexpr BYTE R7_PORT_A_DIRECTION = 0x40;    // 1=output 0=input (siempre 0 en el cpc)
constexpr BYTE R7_PORT_B_DIRECTION = 0x80;    // 1=output 0=input  (not used in cpc)
constexpr BYTE R7_PORT_A_INPUT  = 0;
constexpr BYTE R7_PORT_A_OUTPUT = 0x40;
constexpr BYTE R7_PORT_B_INPUT  = 0;
constexpr BYTE R7_PORT_B_OUTPUT = 0x80;


typedef enum {
    MONO = 0, ABC, ACB, BAC, BCA, CAB, CBA, ST_CUSTOM = 255
} T_AY_Stereo;

typedef struct {
    u16 freq;
    u8 channels; // 1=mono 2=stereo
    u16 bits; // 8 o 16
} T_Sound_Format;


class PSG //: public Dispositivo
{
    Keyboard* teclado;

	bool active = false;  
	// cuando se ejecuta una operacion se pone a TRUE y no se ejecutara otra operacion
	// hasta que no se ponga a FALSE con setInactive()
    // parece que eso es solo para el cpc+

    // el registro 14 es para leer el teclado
    BYTE registros[PSG_NUM_REGISTERS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    BYTE registroSeleccionado = 0;
    //BYTE funcion = PSG_INACTIVE;
    //bool inactivo = false;

    // datos del AY que son los mismos de los registros
    u16 *toneA = (u16*) &registros[0];
    u16 *toneB = (u16*) &registros[2];
    u16 *toneC = (u16*) &registros[4];
    u8      *noise = &registros[6];
    // flags del registro 7
    bool toneA_flag, toneB_flag, toneC_flag, noiseA_flag, noiseB_flag, noiseC_flag;
    u8   *volA = &registros[8];
    u8   *volB = &registros[9];
    u8   *volC = &registros[10];
    u16  *volumeEnvelopeFreq = (u16*) &registros[11];
    u8   *volumeEnvelopeShape = &registros[13];
        
public:

	PSG(Keyboard* teclado);
    Keyboard* getKeyboard();
	void update(u8 ciclos);

	void selectRegister(u8 numRegistro);
	u8   getSelectedRegister(); // sobra!
	void writeSelectedRegister(BYTE dato);
	void writeR7(BYTE dato); // para escribir solo en el registro 7
	void readSelectedRegister(BYTE *var);
	void readSelectedRegister(BYTE dato, BYTE *puertoA_val);

	void setInactive(); // tal vez aplique para el cpc+ pero no para el cpc

	void init();
	void reset();

	void print();

	void setSnaData(SNA_PSG* sna);
	void getSnaData(SNA_PSG* sna);

};
