/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Gatearry implementation                                                   |
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


/***

(wikicpc)
Generación de interrupciones
Las interrupciones en el CPC son creadas por el GA en función de los ajustes del CRTC. 
El GA tiene un contador interno R52 (la R es de Raster) que cuenta de 0 a 51, incrementándose después de cada señal HSYNC.

En todos los CRTC, las interrupciones R52 siempre comienzan 1 µs después del final de un HSYNC. Pero en los CRTC 3/4,
los HSYNC ocurren 1 µs después que en los CRTC 0/1/2. Lo que significa que en los CRTC 3/4, las interrupciones 
comienzan 1 µs después que en los CRTC 0/1/2.

R52 volverá a 0 y el Gate Array enviará una solicitud de interrupción en cualquiera de estas condiciones:
- Cuando supere 51
- Al establecer el bit 4 del registro RMR del Gate Array en 1
- Al final del segundo HSYNC después del inicio del VSYNC

Cuando el Gate Array envía una solicitud de interrupción:

- Si las interrupciones se autorizaron en el momento de la solicitud, entonces el bit 5 de R52 se borra 
(pero R52 se restableció a 0 de todos modos) y se produce la interrupción
- Si no se autorizan las interrupciones, entonces el contador R52 continúa incrementándose y 
la interrupción permanece armada (el Gate Array mantiene su señal INT). Cuando se habilitan las interrupciones
(usando la instrucción EI) y después de la instrucción que sigue a EI (por lo tanto, no inmediatamente después de EI),
el bit 5 de R52 se borra y se produce la interrupción


(grimware)
Las interrupciones enmascarables de la CPU son generadas por el Gate Array. Esto se hace utilizando un contador interno 
de 6 bits y monitoreando las señales HSync y VSync producidas por el CRTC.

En cada flanco descendente de la señal HSync, el Gate Array incrementará el contador en uno. Cuando el contador llega a 52,
el Gate Array aumenta la señal INT y reinicia el contador. Con configuraciones PAL CRTC de 50 Hz (un HSync cada 64 µs), 
esto producirá una tasa de interrupción de 300 Hz.

Cuando la CPU reconoce la interrupción (por ejemplo, va a saltar al vector de interrupción), el Gate Array reiniciará 
el bit 5 del contador, por lo que la siguiente interrupción no puede ocurrir más cerca de 32 HSync.

Cuando se produce un VSync, el Gate Array esperará dos HSync y:

- Si el contador >=32 (bit5=1), entonces no se emite ninguna solicitud de interrupción y el contador se restablece a 0.
- Si el contador <32 (bit5=0), entonces se emite una solicitud de interrupción y el contador se restablece a 0.

Este retraso de 2 HSync después de un VSync se utiliza para permitir que el programa principal, ejecutado por la CPU,
tenga tiempo suficiente para detectar el VSync (para la sincronización con la pantalla, probablemente) antes de que
finalmente se ejecute una rutina de servicio de interrupción.

Por lo tanto, todos los tiempos de interrupción están determinados principalmente por las configuraciones CRTC. 
Aparte de eso, el contador de interrupciones interno se puede borrar en cualquier momento mediante software utilizando 
el registro RMR del Gate Array. (bit 4)

El flanco descendente del HSync activa el contador, por lo tanto, modificar la duración del HSync con el Registro CRTC 3 
puede retrasar las solicitudes de interrupción unos pocos microsegundos. Esto se puede utilizar para ajustar los tiempos 
de interrupción entre las máquinas CPC y Plus...

***/


#include "tipos.h"
#include "io_device.h"
#include "memory.h"
#include "monitor.h"
#include "z80.h"
#include "sna.h"
#include "compilation_options.h"




// tal vez los arrays coloresTintas y coloresRGB sea redundantes

class GateArray : public IO_Device, public IAttendedInterruptReceiver
#ifdef debug_ints_borde
, public IDisplayInterrupt
#endif
{
public:
	static constexpr u8 HW_BORDER = 16;
	static constexpr u8 GA_PALETTE_SIZE = 17;

private:
    Z80* cpu;
    Memory* memoria;
    Monitor* monitor;


    i8 tintaSeleccionada;
    u8 coloresHardware[GA_PALETTE_SIZE]; // el 16 es el borde
    //u8 cambioColor[17] = {255};
    u8 screenMode;
    u8 cambioMode = 255;

    //u8 screenMode, numPaginas;
    u8 arrayTintas[8];  // cada byte tiene 8 subpixeles
    u8 modoVideoLinea = 1;
    
    u8 r52;
    bool vsync;
    
    u8 colorBordeInt; // para mostrar las interrupciones

	// ----- variables para guardar en los SNA -----
    BYTE ramConfiguration;
    bool upperRomDisabled = true;
    bool lowerRomDisabled = true;
	u8 selectedRom;

    
public:

    TColor coloresRGB[17]; // para que consular las componentes RGB sea mas rapido

    GateArray(Z80* cpu, Memory* memoria, Monitor* monitor);

    //void setMonitor(Monitor* m);
    void notifyInterruptAttended() override;
    
    bool OUT(WORD puerto, BYTE dato) override;
    bool IN(WORD puerto, BYTE* dato) override;
    void reset() override;
    void update(u8 ciclos) override;
  
    // determina la forma y color de los dos bytes que obtiene de la memoria principal
    // numPaginas es para acceder a la memoria principal por el metodo rapido o el normal
    void memoriaVideo(u16 vma, u8 numPaginas, u8 bancoRam);
    void sync(bool hsync, bool vsync);
    void borde();

    void calcSubpixelsByte(BYTE b);

    //void updateColors();
    void updateMode();

    void nextLine();
    void nextFrame(i16 vdur_min);

    void incR52();
    u8 getR52();
    bool getVSync() { return vsync; };
    void setVSync(bool s) { vsync = s; }

    void reset_R52() { r52 = 0; };
    void reset_R52_bit5();
    bool get_R52_bit5();

    // metodos para los sna
	void setSnaData(SNA_GA* sna, u8 version);
	void getSnaData(SNA_GA* sna);

    void configureRam(u8 configuracion);
    void configureModeAndRoms(BYTE dato);


#ifdef debug_ints_borde
    void startInt();
    void endInt();
#endif

};
