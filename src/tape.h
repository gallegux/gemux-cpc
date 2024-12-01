/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Tape unit implementation                                                  |
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

#include <string>
#include "log.h"
#include "cdt.h"



class Tape
{
	static constexpr i32 STOP_MOTOR_CYCLES = 90000;  // nops que tarda el motor en pararse

    CDT* cinta = nullptr;

    bool playPressed = false;
    bool recordPressed = false;
    bool pausePressed = false;
    bool motorStatus = false; // 1=on 0=off
	bool recording = false;

	i32 stopMotorCount = 0;  // si es menor que 0 no hay que contar
	bool requestStopMotor = false;

public:

    Tape();
    ~Tape();

    //CDT* getCDT();

    bool insert(std::string& fichero);
    void eject();
    bool hasCDT();
    void setWriteProtection(bool p);
    bool getWriteProtection();
    bool flipWriteProtection();

    bool play();	// devuelve si PLAY queda presionado
    bool record();  // devuelve si RECORD queda presionado
    bool pause();	// devuelve si PAUSE queda presionado
    
    bool isPlayPressed();
    bool isRecordPressed();
	bool isPausePressed();

    void stop();    // playPressed=recordPressed=pausePressed=false
    void rewind();
    void wind();
    void rewind1();
    void wind1();
    u8 getNumTracks();
    u8 getCurrentTrack();

    void setMotorStatus(bool ms);
    bool getMotorStatus();
    
    void update(u8 ciclos, bool bitToWrite);
    bool getLevel();

};

