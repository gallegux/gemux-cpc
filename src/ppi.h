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
#include "dispositivo.h"
#include "psg.h"
#include "crtc.h"
#include "unidad_cinta.h"
#include "gatearray.h"


#define PPI_INPUT  true
#define PPI_OUTPUT false
#define PPI_READ   true
#define PPI_WRITE  false


class PPI : public Dispositivo
{
    GateArray* gatearray;
    CRTC* crtc;
    PSG* psg;
    UnidadCinta* unidadCinta;

    bool puertoA_dir;  // input/output
    BYTE puertoA_val;

    bool puertoB_dir;
    BYTE puertoB_val;
    
    // el puerto C interviene en el grupo A y B, en el grupo A esta la parte alta (H) y en el B la baja (L)
    bool puertoCh_dir;
    bool puertoCl_dir;
    BYTE puertoC_val;

    BYTE control = 0;
    
        
public:
    
    PPI(PSG* _psg, GateArray* _gatearray, CRTC* _crtc, UnidadCinta* _unidadCinta);

    bool OUT(WORD puerto, BYTE dato) override;
    bool IN(WORD puerto, BYTE* dato) override;
    void reset() override;
    void update(u8 ciclos) override;
  
    void print();
};
