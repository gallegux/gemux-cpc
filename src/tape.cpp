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

#include <string>
#include <cstring>
#include "tape.h"
#include "log.h"
#include "cdt.h"
#include "z80.h"


Tape:: Tape() {}

Tape:: ~Tape() {
    eject();
}


void Tape:: update(u8 ciclos, bool bitToWrite) {
    if (cinta != nullptr  &&  (motorStatus || stopMotorCount > 0)  &&  !pausePressed) {
		//debug_tape("TAPE:: update() %d %d%d %d %d%d \n", cinta!=nullptr,
		//		motorStatus, pausePressed, playPressed, recordPressed, bitToWrite);

        if (playPressed) {
            cinta->getLevel(ciclos);
            if (cinta->isFinished()) stop(); //setMotorStatus(false); //motorStatus = false;
        }
        else if (recordPressed) {
			cinta->setLevel(bitToWrite, ciclos);
			recording = true;
        }
    }
	if (requestStopMotor) {
		if (playPressed) {
			stopMotorCount -= (ciclos >> 2); // nops
			//debug_tape("%d\n", stopMotorCount);
			if (stopMotorCount <= 0) {
				debug_tape("TAPE:: STOP MOTOR REAL\n");
				requestStopMotor = false;
			}
		}
	}
}




bool Tape:: insert(std::string& _fichero) {
    eject();
    
    cinta = new CDT;
    bool ok = cinta->open(_fichero);

    if (ok) {
        debug_tape("TAPE:: insert: %s\n", _fichero.c_str());
        //osd_setMessage("Tape: " + _fichero + " inserted");
    }
    else {
        delete cinta;
        cinta = nullptr;
        debug_tape("TAPE:: insert -> fichero no valido\n");
        //osd_setMessage("Tape: Not valid file");
    }

    return ok;
//FILE* file = fopen(_fichero.c_str(), "rb");
//tape_insert_cdt(file);
}

void Tape:: eject() {
    if (cinta != nullptr) {
        debug_tape("TAPE:: eject\n");
        //osd_setMessage("Tape ejected");
        cinta->close();
        delete cinta;
        cinta = nullptr;

        playPressed = recordPressed = pausePressed = false;
        //motorStatus = false;
    }
//tape_eject();
}


bool Tape:: hasCDT() {
    return cinta != nullptr;
}



bool Tape:: play() {
    if (cinta != nullptr) {
		playPressed = true; //!playPressed;
        recordPressed = false;
		return playPressed;
	}
	else return false;
}

bool Tape:: record() {
    if (cinta != nullptr) {
		recordPressed = true; //!recordPressed;
        playPressed = false;
    	return recordPressed;
	}
	else return false;
}

bool Tape:: pause() {
	if (cinta != nullptr) {
		pausePressed = !pausePressed;
		return pausePressed;
	}
	else return false;
}

bool Tape:: isPlayPressed() { return playPressed; }

// para determinar cuando los datos han de escribirse, porque continuamente
// llegan valores al PPI
bool Tape:: isRecordPressed() { return recordPressed; }

void Tape:: stop() { 
    playPressed = recordPressed = pausePressed = false; 
}


void Tape:: rewind() {
    if (cinta != nullptr) {
		stop();
		cinta->rewind();
	}
}

void Tape:: wind() {
    if (cinta != nullptr) {
		stop();
		cinta->wind();
	}
}

void Tape:: rewind1() {
    if (cinta != nullptr) {
		stop();
		cinta->rewind1();
	}
}

void Tape:: wind1() {
    if (cinta != nullptr) {
		stop();
		cinta->wind1();
	}
}

u8 Tape:: getCurrentTrack() {
    return (cinta != nullptr)  ?  cinta->getCurrentBlock()  :  0;
}


u8 Tape:: getNumTracks() {
    return (cinta != nullptr)  ?  cinta->getNumBlocks()  :  0;
}



bool Tape:: getLevel() {
	return (cinta == nullptr) ? LEVEL_LOW : cinta->getLevel();
}

void Tape:: setMotorStatus(bool ms) { 
    if (ms != motorStatus) {
		if (ms) debug_tape("Tape:: motor=ON ");
		else    debug_tape("Tape:: motor=OFF");
		debug_tape("   %d  %d\n", requestStopMotor, stopMotorCount);
	}

	if (ms) { // si el motor se ha de poner en funcionamiento anulamos la solicitud de parada
		requestStopMotor = false;
		stopMotorCount = -1;
	}
	
	if (!ms  &&  motorStatus) {  // parar motor (pasa de encendido a apagado)
		if (playPressed) {
			requestStopMotor = true;
			if (stopMotorCount <= 0) {
				// si stopMotorCount>0 ya se solicito hace poco (menos de STOP_MOTOR_CYCLES ciclos)
				stopMotorCount = STOP_MOTOR_CYCLES;
				debug_cdt("Tape:: solicitud parar motor estando en PLAY\n");
			}
		}
		if (recording) {
			// parar el motor inmediatamente, no simula el tiempo que tarda en parar
			debug_tape("Tape:: fin grabacion, parar motor inmediatamente\n");
			cinta->endWrite();
			recording = false;
		}
	}

    motorStatus = ms;
}

bool Tape:: getMotorStatus() { 
	debug_tape("Tape:: getMotorStatus() motorStatus=%d\n", motorStatus);
	return motorStatus; 
}


void Tape:: setWriteProtection(bool p) {
    if (cinta != nullptr) return cinta->setProtected(p);
}

bool Tape:: getWriteProtection() {
    bool p = (cinta != nullptr) ? cinta->isProtected() : false;
    debug_tape("Tape:: getWriteProtection() proteccion = %d\n", p);
    return p;
}

bool Tape:: flipWriteProtection() {
    if (cinta != nullptr) {
        cinta->flipProtected();
        debug_tape("Tape:: flipWriteProtection() proteccion = %d\n", cinta->isProtected());
        //std::string prefijo = cinta->isProtected() ? "" : "not ";
        //osd_setMessage("Cassette " + prefijo + "protected");
        return cinta->isProtected();
    }
    else return false;
}
