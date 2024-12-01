/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CRTC implementation                                                       |
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


// cpcwiki.eu/index.php/CRTC

#include <stdio.h>
#include "log.h"
#include "tipos.h"
#include "crtc.h"
#include "monitor.h"
#include "sna.h"


#define R0 registros[0]
#define R1 registros[1]
#define R2 registros[2]
#define R3 registros[3]
#define R3h (registros[3] >> 4)
#define R3l (registros[3] & 0x0F)
#define R4 registros[4]
#define R5 registros[5]
#define R6 registros[6]
#define R7 registros[7]
#define R8 registros[8]
#define R9 registros[9]
#define R10 registros[10]
#define R11 registros[11]
#define R12 registros[12]
#define R13 registros[13]


#define NUM_REGISTROS_INI 14
const BYTE crtc_registros_inicio[NUM_REGISTROS_INI] = 
    {0x3F, 0x28, 0x2E, 0x8E, 0x26, 0x00, 0x19, 0x1E, 0x00, 0x07, 0x00, 0x00, 0x30, 0x00};


// TODO: diferenciar entre los tipos de CRTC, sobre todo para leer registros


CRTC:: CRTC(Z80* cpu, GateArray* ga) {
    debug("CRTC\n");
    
    this->cpu = cpu;
    this->gatearray = ga;

    reset();
}



void CRTC:: reset() {
    for (u8 i = 0; i < NUM_REGISTROS_INI; i++)  registros[i] = crtc_registros_inicio[i];

    pintar_caracter_horizontal = true;
    pintar_caracter_vertical = false;
    hsync = vsync = false;

    hcc = 0;
    vcc = 0;
    vlc = 0;
    vsc = 0;

    hsc = 0;
    vtac = 0;

	vdur = 0xFFE1;
	vma = 0;

    actualizarVariablesInternas();
}


void CRTC:: actualizarVariablesInternas() {
    horizontal_total = registros[0] + 1;
    vertical_total = (registros[4] & 0b01111111) + 1;
    lineas_adicionales = registros[5] & 0b00011111;
    vertical_displayed = registros[6] & 0b01111111;
    vertical_sync_position = registros[7] & 0b01111111;
    interlace_and_skew = registros[8] & 0b00000011;
    lineas_caracter = (registros[9] & 0b00000111) + 1;
    vdur_min = (registros[7]-35+1)*8+1; // no tocar!!

    // VSync width in scan-lines. (0 means 16 on some CRTC. Not present on all CRTCs, fixed to 16 lines on these)
    //fin_vsync = vertical_sync_position + (R3h == 0) ? 16 : R3h;
    // HSync pulse width in characters (0 means 16 on some CRTC), should always be more than 8
    fin_hsync = horizontal_sync_position + ((R3l == 0) ? 16 : R3l);

    u16 vmaPagina  = registros[12] & 0b00110000;
    vma_base = vmaPagina << 10;
    u16 vmaOffsetH = registros[12] & 0b00000011;
    u16 vmaOffsetL = registros[13];
    vma_origen = vma_base | (vmaOffsetH << 9) + (vmaOffsetL << 1);
    bancoRam = (registros[12] & 0b00110000) >> 4;
    
    if ((registros[12] & 0x0C)  ==  0x0C) {
        mascara_incremento = 0x7FFF;
        paginas = 2;
        limite_pagina = vma_base + 0x8000;
    }
    else {
        mascara_incremento = 0x3FFF;
        paginas = 1;
        limite_pagina = vma_base + 0x4000;
    }
    debug_crtc("CRTC::  VMA = %04X   VMA_limite=%04X \n", vma_origen, limite_pagina);

    
    debug_crtc("Monitor:: vdur_min=%04X,%d horz_total=%d vert_total=%d lineas_adicionales=%d " 
        "vert_displayed=%d vert_sync_position=%d lineas_caracter=%d "
        "fin_hsync=%d bancoRam=%d vma_origen=%04X paginas=%d mascara_inc=%04X\n",
        vdur_min, vdur_min,
        horizontal_total, vertical_total, lineas_adicionales,
        vertical_displayed, vertical_sync_position, lineas_caracter,
        fin_hsync, bancoRam, vma_origen, paginas, mascara_incremento);
    
/*
    debug("horz_total=%d horz_displayed=%d horz_sync_pos=%d fin_hsync=%d\n",
        horizontal_total, horizontal_displayed, horizontal_sync_position, fin_hsync);
*/
}



bool CRTC:: OUT(WORD puerto, BYTE dato) {
    if (puerto.b.h == 0xBC) {
        // seleccionar registro
        //debug_crtc("CRTC::  seleccionar_registro=%d\n", dato);
        registroSeleccionado = dato;
        return true;
    }
    if (puerto.b.h == 0xBD) {
        // escribir en registro
        debug_crtc("CRTC::  escribir_registro -> %d=%02X\n", registroSeleccionado, dato);
        registros[registroSeleccionado] = dato;
        //debug("CRTC %d=%02X\n", registroSeleccionado, dato);
        actualizarVariablesInternas();
        //if (registroSeleccionado == 13) pausa();
        return true;
    }
    return false;
}


bool CRTC:: IN(WORD puerto, BYTE *dato) {
    if (puerto.b.h == 0xBF) {
        // consultar registro
        if (registroSeleccionado > 11  &&  registroSeleccionado < 18)
            *dato = registros[registroSeleccionado];
        else
            *dato = 0;
        return true;
    }
    else return false;  // seguro?
}



void CRTC:: incLinea() {
    gatearray->incR52();

    if (vtac) { // lineas adicionales?
        vtac--;
        vlc++;
        if (vlc == lineas_caracter) {
            vlc = 0;
            vcc++;
        }
        if (!vtac)  vlc = 0;
        // aqui no incrementamos cont_linea_caracter
    }
    else {
        vlc++;
        if (vlc == lineas_caracter) {
            vlc = 0;
            vcc++;
        }
        if (vsync) { // el espacio de sincronizacion vertical se mide en lineas (no en caracteres)
            vsc++;
            if (vsc == R3h) {
                vsync = false;   
                vsc = 0; 
                //debug_crtc("CRTC::  VSYNC inactivo\n");
                vtac = lineas_adicionales;
                gatearray->setVSync(false);
                gatearray->nextFrame(vdur_min);
            }
        }
        else if (vcc == vertical_displayed) {
            pintar_caracter_vertical = false;
        }
        else if (vcc == vertical_total) {
            vcc = 0;
            pintar_caracter_vertical = true;
        }
        else if (vcc == vertical_sync_position) {
            //debug_crtc("CRTC::  VSYNC activo\n");
            vdur = vdur_min;
            vsync = true;
            vsc = 0;
            //gatearray->reset_R52();
            gatearray->setVSync(true);
            // deberiamos esperar 2 hsync antes  de empezar el frame
			// (https://grimware.org/doku.php/documentations/devices/gatearray)
            //gatearray->nextFrame();
            //if (!ajustado) {
            //    cpu->solicitarInterrupcion();
            //    c52 = 0;
            //    ajustado = true;
            //    debug_crtc("CRTC.  ajustado\n");
            //}
        }
    }
}


void CRTC:: update(u8 numCiclosCpu) 
{
    while (numCiclosCpu--) {
        if (hsync || vsync) {
            //debug("sync  ");
            //gatearray->sync(hsync, vsync);  no pintamos la zona sync
        }
        else if (pintar_caracter_horizontal && pintar_caracter_vertical) {
            gatearray->memoriaVideo(vma, paginas, bancoRam);
            vma++;
            gatearray->memoriaVideo(vma, paginas, bancoRam);
            vma++;
            
            if ((vma & 0x07FF) == 0) vma -= 0x800;
        }
        else {
            gatearray->borde();
        }

        hcc++;

        if (hcc == horizontal_total) {
            pintar_caracter_horizontal = true;
            hcc = 0;
            incLinea();

            vma = (vcc * horizontal_displayed*2) + (0x800 * vlc) + vma_origen;
            if ((vma - vma_base) >= (vlc+1) * 0x800) vma -= 0x800;
            if (vma >= limite_pagina)  vma -= (paginas * 0x4000);
            //debug_crtc("CRTC:: nueva_linea.  VMA=%04X  l=%d  sl=%d\n", vma, cont_vert, cont_linea_caracter);
        }
        else if (hcc == horizontal_displayed) {
            pintar_caracter_horizontal = false;
        }
        else if (hcc == horizontal_sync_position) {
            hsync = true;
            hsc = 0;
            //gatearray->updateColors();
            gatearray->updateMode();
        }
        else if (hcc == fin_hsync) {
            hsync = false;
            gatearray->nextLine();
        }
        
        if (vsc == 2  &&  hsc == R3l)  {
            // comprobad a mano, para que las interrupciones coincidan con las de winape
            gatearray->reset_R52();
        }

        if (hsync) hsc++;
        if (hsc == 5) vdur++;
    }
}





void CRTC:: print() {
    debug(" vcc=%02X vlc=%02X vsc=%02X  R52=%02X  hcc=%02X hsc=%02X  vma=%04X vdur=%04X  HV=%d,%d", //  vma=%04X\n",
        vcc, vlc, vsc, gatearray->getR52(), hcc, hsc, vma, vdur, hsync, vsync);
    /*
    debug("cH=%02d  cV=%02d  cLC=%02d  VS=%d  cAV=%d  LA=%d  VMA=%04X  C52=%d  pXY=%d,%d  sHV=%d,%d\n",
        cont_horz, cont_vert, cont_linea_caracter, cont_lineas_vsync, contLineasAdicionales, cont_lineas_abs, vma, c52, 
        pintar_caracter_horizontal, pintar_caracter_vertical,
        hsync, vsync);
    */
   //debug_crtc("%02X %02d %02d %03d %02d", gatearray->getR52(), vcc, vlc, vdur, vsc);
}

