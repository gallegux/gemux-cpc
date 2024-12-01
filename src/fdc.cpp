/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  FDC implementation                                                        |
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

#include <fstream>
#include <vector>
#include <stdio.h>
#include "tipos.h"
#include "fdc.h"
#include "log.h"
#include "dsk.h"
#include "util.h"
#include "sna.h"



#define SET4(_a,_v) { for (u8 i=0; i<4; i++) _a[i]=_v; }
#define PRINT4(_a) { for (u8 i=0; i<4; i++) debug_fdc("%d",_a[i]); debug_fdc("\n"); }



bool FDC::FAST_MODE = false;


bool FDC:: getFastMode() {
	return FAST_MODE;
}

void FDC:: setFastMode(bool fastMode) {
	FAST_MODE = fastMode;
}

bool FDC:: flipFastMode() {
	FAST_MODE = !FAST_MODE;
	return FAST_MODE;
}



void printInputBytes(BYTE* bs, u8 _numBytesEntrada) {
    debug_fdc("\nFDC:: entrada = ");
    for (u8 i = 0; i < _numBytesEntrada; i++) {
        debug_fdc("%02X ", *bs);
        bs++;
    }
    debug_fdc("\n");
}
void printOutputBytes(BYTE* bs, u8 _numBytesSalida) {
    debug_fdc("FDC:: resultado  = ");
    for (u8 i = 0; i < _numBytesSalida; i++) {
        debug_fdc("%02X ", *bs);
        bs++;
    }
    debug_fdc("\n");
}

void FDC:: printBuffer() {
    BYTE* p = buffer;
    while (p != finBuffer) {
        debug_fdc("%02X ", *p);
        p++;
    }
    debug_fdc("\n");
}



FDC:: FDC() {
    debug_fdc("FDC\n");
    reset();
}


void FDC:: reset() {
    for (u8 i = 0; i < 4; i++)  f_driveBusy[i] = false;
	
    f_fdcBusy = false;
    numBytesEntrada = 0;
    p_cabezal = 0;
    p_pista = 0;
    p_sector = 0;
    p_tamSector = 0;
    currentDsk = nullptr;
    regEstado0 = regEstado1 = regEstado2 = 0;
    fase = FREE_PHASE;
    ready = true;
    led = false;
}

void FDC:: update(u8 ciclos) {
    if (fase == EXECUTION_PHASE) {
        contadorReady -= ciclos;

		ready = contadorReady <= 0;
        //if (dio = FDC_A_CPU) {
        //    // cuando lee tarda 26 microsegundos (22 NOPs) aproximadamente en enviar un byte al microprocesador
        //    if (contadorReady <= 0) {
        //        ready = true;
        //    }
        //}
        //else {
        //    // la escritura es mas lenta, pongamos 25 NOPs
        //    if (contadorReady <= 0) {
        //       ready = true;
        //    }
        //}
    }
    else {
        // cuando no esta leyendo o escribiendo el tiempo de espera es despreciable
        ready = true;
    }
}

// devuelve si la unidad esta lista
//bool FDC:: getRQM() {
//    return ready;
//}
bool FDC:: getLED() { return (led && fase == EXECUTION_PHASE); }

void FDC:: setDisk(u8 drive, DSK* dsk) { disk[drive] = dsk; }


inline void FDC:: prepareCommandFDC(u8 _numBytesEntrada, u8 _numBytesSalida, 
        FDC_DIO _dio, bool _led, PtrFuncion funcion) {
    this->numBytesEntrada = _numBytesEntrada;
    this->numBytesSalida = _numBytesSalida;
    this->led = _led;   // es una operacion que implica que el led pase a rojo
    this->dio = _dio;
    this->ptrFuncion = funcion;
}


bool FDC:: OUT(WORD puerto, BYTE dato) {
    switch (puerto.w) {
        case 0xFA7E:    // floppy motor control
            debug_fdc("FDC:: motor %d\n", dato);
            motorOn = dato == 1;
            SET4(f_statusChanged, true);
            return true;
        case 0xFB7F:
            //debug_fdc("FDC:: OUT dat, %02X\n", dato);
            out_FB7F(dato);
            return true;
        default: return false;
    }
}


void FDC:: out_FB7F(BYTE dato) {
    if (fase == FREE_PHASE) {
        fase = COMMAND_PHASE;
        contBytesEntrada = 0;
        f_fdcBusy = true;
        dio = CPU_TO_FDC;
        comando = dato & 0x1F;

        // identificamose el primer byte de entrada que se corresponde con el comando
        switch (comando) {
            case CMD_READ_TRACK:            prepareCommandFDC(9, 7, FDC_TO_CPU, 1, &FDC::readTrack); break;// read a track o read diagnostic
            case CMD_SPECIFY:               prepareCommandFDC(3, 0, FDC_TO_CPU, 0, &FDC::specify); break; // specify
            case CMD_SENSE_DRIVE_STATUS:    prepareCommandFDC(2, 1, FDC_TO_CPU, 0, &FDC::senseDriveStatus); break; // sense drive status
            case CMD_WRITE_DATA:            prepareCommandFDC(9, 7, CPU_TO_FDC, 1, &FDC::writeData); break; // write data
            case CMD_READ_DATA:             prepareCommandFDC(9, 7, FDC_TO_CPU, 1, &FDC::readData); break; // read data
            case CMD_RECALIBRATE:           prepareCommandFDC(2, 0, FDC_TO_CPU, 0, &FDC::recalibrate); break; // 0x7
            case CMD_SESE_INTERRUPT_STATUS: prepareCommandFDC(1, 2, FDC_TO_CPU, 0, &FDC::senseInterruptStaus); break;// sense interrupt status
            case CMD_WRITE_DELETED_DATA:    prepareCommandFDC(9, 7, CPU_TO_FDC, 1, &FDC::writeDeletedData); break; // write deleted data
            case CMD_READ_ID:               prepareCommandFDC(2, 7, FDC_TO_CPU, 1, &FDC::readId); break;// 0xA
            case CMD_READ_DELETED_DATA:     prepareCommandFDC(9, 7, FDC_TO_CPU, 1, &FDC::readDeletedData); break; // read deleted data
            case CMD_FORMAT_TRACK:          prepareCommandFDC(6, 7, CPU_TO_FDC, 1, &FDC::formatTrack); break;// format a track
            case CMD_SEEK:                  prepareCommandFDC(3, 0, FDC_TO_CPU, 0, &FDC::seek); break; // 0xF
            case CMD_SCAN_EQUAL:            prepareCommandFDC(9, 7, CPU_TO_FDC, 1, &FDC::scanEqual); break;// scan equal
            case CMD_SCAN_LOW_OR_EQUAL:     prepareCommandFDC(9, 7, CPU_TO_FDC, 1, &FDC::scanSlowOrEqual); break; // scan slow or equal
            case CMD_SCAN_HIGH_OR_EQUAL:    prepareCommandFDC(9, 7, CPU_TO_FDC, 1, &FDC::scanHighOrEqual); break; // scan high or equal
            default:                        prepareCommandFDC(1, 1, FDC_TO_CPU, 0, &FDC::invalid); break; // invalid
        }
        //debug_fdc("FDC:: comando fdc --> %02X\n", comando);
    }

    if (fase == COMMAND_PHASE) {
        // almacenamos el resto de bytes de entrada en su buffer
        bytesEntrada[contBytesEntrada] = dato;
        if (++contBytesEntrada == numBytesEntrada) {
            printInputBytes(bytesEntrada, numBytesEntrada);
            //numBytesEntrada = 0;    // reset para el siguiente comando
            contBytesSalida = 0;
            ready = true; 
            (this->*ptrFuncion)(); // ejecutamos el comando
            if (numBytesSalida > 0) printOutputBytes(bytesSalida, numBytesSalida);
        }
    }
    else if (fase == EXECUTION_PHASE) {
        switch (comando) {
            case CMD_WRITE_DATA:
            case CMD_WRITE_DELETED_DATA:
                writeData_execution(dato);
                break;
            case CMD_FORMAT_TRACK:
                formatTrack_execution(dato);
                break;
            case CMD_SCAN_EQUAL:
            case CMD_SCAN_LOW_OR_EQUAL:
            case CMD_SCAN_HIGH_OR_EQUAL:
                scan_execution(dato);
                break;
        }
    }
}



bool FDC:: IN(WORD puerto, BYTE* dato) {
    switch (puerto.w) {
        case 0xFB7E:    // status register
            *dato = getMainStatusReg();
            //if (fase != FASE_EJECUCION) debug_fdc("FDC:: IN sta = %02X\n", *dato);
            //debug_fdc("FDC:: IN status = %02X\n", *dato);
            //debug_fdc(" --> status_main_register = %02X\n", *dato);
            return true;
        case 0xFB7F:    // data register
            // este devuelve el array de salida
            if (fase == RESULT_PHASE) {
                *dato = bytesSalida[contBytesSalida];
                if (comando == CMD_FORMAT_TRACK) {
                    debug_fdc("FDC:: IN <- %02X\n", *dato);
                }
                if (++contBytesSalida == numBytesSalida) {
                    f_fdcBusy = false;
                    fase = FREE_PHASE;
                    dio = CPU_TO_FDC;
                    comando = CMD_NONE;
                }
                //debug_fdc("FDC:: IN dat = %02X\n", *dato);
            }
            else if (fase == EXECUTION_PHASE) {
                if (comando == CMD_READ_DATA || comando == CMD_READ_DELETED_DATA) {
                        // || comando == CMD_READ_DELETED_DATA) {
                    *dato = *puntBuffer;
                    //debug_fdc("FDC:: read %02X\t", *dato);
                    if (++puntBuffer == finBuffer) {
                        //debug_fdc("DATOS READ: ");
                        //printBuffer();
                        finBuffer = nullptr;
                        puntBuffer = nullptr;
                        //debug_fdc("\nFDC:: delete buffer\n");
                        delete buffer;
                        bytesCopiados = true;
                        fase = RESULT_PHASE;
                        debug_fdc("FDC:: fin transferencia\n");
                    }
                }
                else if (comando == CMD_READ_TRACK) {
                    readTrack_execution(dato);
                }
            }
            else {
                *dato = 0xFF;
            }
            //debug_fdc("FDC:: data_register  %02X\n", *dato);
            return true;
    }
    return false;
}



BYTE FDC:: getMainStatusReg() {
    //if (bytesEstadoPrincipal.quedan() > 0) return bytesEstadoPrincipal.getSiguiente();

    BYTE estado = 0; //MR_RQM_READY; // siempre preparado

    if (FAST_MODE) {
        estado |= MR_RQM_READY;
    }
    else if (ready) {
        estado |= MR_RQM_READY;
		// TODO mirar con detenimiento los tiempos entre escrituras y lectoras, son los mismos?
        if      (dio == FDC_TO_CPU)     contadorReady += CICLOS_ENTRE_LECTURAS;
        else /*if (dio == CPU_A_FDC)*/ contadorReady += CICLOS_ENTRE_ESCRITURAS;
    }

    if (fase == COMMAND_PHASE) {
        estado |= MR_DIO_CPU_TO_FDC;
        estado |= MR_FDC_BUSY;
    }
    else if (fase == EXECUTION_PHASE) {
        estado |= MR_EXM_EXECUTION_MODE;
        estado |= MR_FDC_BUSY;

        if (dio == FDC_TO_CPU) estado |= MR_DIO_FDC_TO_CPU; 
    }
    else if (fase == RESULT_PHASE) {
        estado |= MR_DIO_FDC_TO_CPU; 
        estado |= MR_FDC_BUSY;
    }
    else {
        estado = MR_RQM_READY;
    }
    //if (dio == FDC_A_CPU)       estado |= MR_DIO_FDC_TO_CPU; 
    //if (fase == FASE_EJECUCION) estado |= MR_EXM_EXECUTION_MODE;
    if (f_fdcBusy)              estado |= MR_FDC_BUSY;
    if (comando == CMD_NONE)    estado = MR_RQM_READY;

    // busy de las unidades
    u8 bit = 1;
    for (u8 i = 0; i < MAX_DRIVES; i++) {
        if (f_driveBusy[i])  estado |= bit;
        bit = bit << 1;
    }
    if (comando == CMD_READ_ID && fase == EXECUTION_PHASE) debug_fdc("FDC:: MSR %02X\n", estado);
	
    return estado;
}

inline void FDC:: getParam_MT() { mt_multiTrack = bytesEntrada[0] & 0x80; }
inline void FDC:: getParam_MF() { mf_mfmMode = bytesEntrada[0] & 0x40; }
inline void FDC:: getParam_SK() { sk_skip = bytesEntrada[0] & 0x20; }

void FDC:: getDriveHead() { 
    p_unidad = bytesEntrada[1] & 0x03; 
    currentDsk = disk[p_unidad];
    currentSide[p_unidad] = p_cabezal = (bytesEntrada[1] & 0x04) >> 2;
}

void FDC:: getParams_CHRN_EOT_GPL_DTL() {
    getDriveHead();
    p_pista = bytesEntrada[2];
    //p_cabezal = bytesEntrada[3]; // parece que este no es el cabezal
    p_sector = bytesEntrada[4];
    p_tamSector = bytesEntrada[5];
    p_ultSectorPista = bytesEntrada[6];
    p_gap = bytesEntrada[7];
    p_longDatosSi = bytesEntrada[8];
    //debug_fdc("FDC::\tunidad=%d pista=%d cab=%d sector=%02X tam_sec=%d num_sec=%02X gap=%d longDatosSi=%d\n", 
    //            p_unidad, p_pista, p_cabezal, p_sector, p_tamSector, p_ultSectorPista, p_gap, p_longDatosSi);
}

void FDC:: setOutputBytes_RS012_CHRN(BYTE pista, BYTE cabezal, BYTE sector, BYTE longSector) {
    // la mayoria de los comandos devuelven los mismos bytes
    bytesSalida[0] = regEstado0;
    bytesSalida[1] = regEstado1;
    bytesSalida[2] = regEstado2;
    bytesSalida[3] = pista;
    bytesSalida[4] = cabezal;
    bytesSalida[5] = sector;
    bytesSalida[6] = longSector;
}


// REVISAR!!!!!!!!!!!
/*  se leen todos los bytes de datos de la pista.
    finaliza cuando haya sido leido el sector indicado como ultimo o, 
    si no fue hallado, cuando la perforacion de indice genere el segundo
    impulso tras el inicio del comando

    no funciona con discology, ya que trata de leer desde el primer sector
    con tamanio 6k, esto consigue leer los bytes que corresponden a 
    cabeceras de las pistas, y me falta documentacion :(

    funciona cuando el tamanio del sector que se le pasa es el adecuado
*/
void FDC:: readTrack() {
    debug_fdc("FDC:: readTrack()\n");
    getParams_CHRN_EOT_GPL_DTL();

    if (currentDsk != nullptr) {
        regEstado0 = 0;
        // registro estado 0
        if (!disk[p_unidad] || p_cabezal >= disk[p_unidad]->getSides()) regEstado0 |= R0_NR_NOT_READY; // not ready
        //if (!disk[p_unidad]) x |= 0x40; 
        bytesSalida[0] = regEstado0;
        // registro estado 1
        regEstado1 = 0;
        if (disk[p_unidad] && disk[p_unidad]->isProtected()) regEstado1 |= R1_NW_NOT_WRITEABLE;
        if (disk[p_unidad] && !disk[p_unidad]->existsSector(p_pista, p_cabezal, p_sector)) regEstado1 |= R1_ND_NO_DATA; 
            // buscar el sector en la pista, si no existe poner el bit 2 a 1
        bytesSalida[1] = regEstado1;
        // registro estado 2
        bytesSalida[2] = regEstado2 = 0;
        
        fase = EXECUTION_PHASE;
        bytesCopiados = false;
        contadorReady = CICLOS_ENTRE_LECTURAS;

        T_DskTrackInfo trackInfo;
        std::vector<T_SectorInfo> sectores;
        currentDsk->getTrack(p_pista, p_cabezal, &trackInfo, sectores);

        readTrackData = new ReadTrackData(&trackInfo, sectores, p_tamSector, p_ultSectorPista - p_sector + 1);

        // resultado
        setOutputBytes_RS012_CHRN(p_pista, p_cabezal, p_ultSectorPista, p_tamSector);
    }
    else {
        fase = RESULT_PHASE;
        bytesSalida[0] = R0_IC_COMANDO_INTERRUMPIDO | R0_NR_NOT_READY;
        bytesSalida[1] = bytesSalida[2] = 0;
    }
}


void FDC:: readTrack_execution(BYTE* dato) {
    debug_fdc(",");
    
    if (readTrackData->getCurrentSector() == 0xFF) {
        debug_fdc("FDC:: sectorSize=%d\n", readTrackData->getSectorSize());
        currentDsk->goTrackSide(readTrackData->getTrack(), readTrackData->getSide());
        readTrackData->goFirstSector();
        debug_fdc("FDC:: currentSector=%02X\n", readTrackData->getCurrentSector());

        buffer = new BYTE[readTrackData->getSectorSize()];
        finBuffer = buffer + readTrackData->getSectorSize();
        readTrack_readSector();
    }

    *dato = *puntBuffer;

    //debug_fdc("FDC:: read %02X\t", *dato);
    if (++puntBuffer == finBuffer) {
        debug_fdc("FDC:: currentSector=%02X\n", readTrackData->getCurrentSector());
        readTrackData->nextSector();
        if (readTrackData->getCurrentSector() == 0) 
            currentDsk->goTrackSide(readTrackData->getTrack(), readTrackData->getSide());
        debug_fdc("FDC:: currentSector=%02X\n", readTrackData->getCurrentSector());
        if (readTrackData->getNumSectoresEscanear() == readTrackData->getNumSectoresEscaneados() 
                    ||  readTrackData->getEndTrackCounter() == 2)  {
            fase = RESULT_PHASE;
            delete buffer;
            buffer = puntBuffer = finBuffer = nullptr;
            delete readTrackData;
            debug_fdc("FDC:: fin pista\n");
        }
        else {
            debug_fdc("FDC:: readTrack_exec readSector()\n");
            readTrack_readSector();
        }
    }
}


void FDC:: readTrack_readSector() {
    puntBuffer = buffer;
    T_SectorInfo si = readTrackData->getSector();
    currentDsk->getSectorInfo_ID(si.track, si.side, si.sectorId, &si);
    currentDsk->readSectorData(readTrackData->getSectorSize(), puntBuffer);
    debug_fdc("FDC:: %02X\n", *puntBuffer);
    //bytesSalida[5] = si.sectorId;

    /*
    if (currentDsk->getSectorInfo_ID(sectorInfo->track, sectorInfo->side, sectorInfo->sectorId, sectorInfo)) { 
        // sector encontrado
        if (sectorInfo->fdcSt1 & R1_DE_DATA_ERROR  ||  sectorInfo->fdcSt2 & R2_DD_DATA_ERROT_IN_DATA) {
            regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
            regEstado1 = R1_OR_OVER_RUN;
            regEstado2 = 0;
            fase = FASE_RESULTADO;
            finBuffer = nullptr;
            puntBuffer = nullptr;
            delete buffer;
            delete readTrackData;
            bytesCopiados = true;
        }
        else {
            currentDsk->readSectorData(readTrackData->getSectorSize(), buffer); // obtener datos
            puntBuffer = buffer;
        }
    }
    else {  // sector no encontrado
        debug_fdc("FDC:: sector no encontrado\n");
        regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
        regEstado1 = R1_ND_NO_DATA;
        regEstado2 = 0;
        fase = FASE_RESULTADO;
        bytesCopiados = true;
    }
    */
}



// comando 03 - no hay bytes de salida
void FDC:: specify() {
    debug_fdc("FDC:: ---------- specify()\n");
    fase = FREE_PHASE;
    f_fdcBusy = false;
    BYTE stepRateTime   = bytesEntrada[1] >> 4;
    BYTE headLoadTime   = bytesEntrada[1] & 0x0F;
    BYTE headUnloadTime = bytesEntrada[2] >> 1;
    BYTE dmaDisable     = bytesEntrada[2] & 0x01;
    //debug_fdc("FDC:: stepRateTime=%d headLoadTime=%d headUnloadTime=%d dmaDisable=%d\n", 
    //    stepRateTime, headLoadTime, headUnloadTime, dmaDisable);
}


void FDC:: senseDriveStatus() {
    debug_fdc("FDC:: ---------- senseDriveStatus()\n");
    // HU
    getDriveHead();
    debug_fdc("\tunidad=%d cabezal=%d\n", p_unidad, p_cabezal);
    
    BYTE reg3 = p_unidad;

    if (currentDsk == nullptr) {
        reg3 |= R3_FT_FAULT;
    }
    else {
        if (p_cabezal) reg3 |= R3_HD_HEAD_ADDRESS;
        if (currentDsk->getSides() == 2) reg3 |= R3_TS_TWO_SIDE;
        if (currentTrack[p_unidad] == 0) reg3 |= R3_T0_TRACK_0;
        reg3 |= R3_RY_READY;
        if (currentDsk->isProtected()) reg3 |= R3_WP_WRITE_PROTECTED;
    }
    debug_fdc("FDC:: reg_estado_3 = %02X\n", reg3);

    bytesSalida[0] = reg3;

    fase = RESULT_PHASE;
    //f_fdcBusy = true;
}


void FDC:: writeData() {
    debug_fdc("FDC:: ---------- writeData()\n");
    getDriveHead();
    getParams_CHRN_EOT_GPL_DTL();
    contadorReady = CICLOS_ENTRE_ESCRITURAS;
    ready = false;

    if (currentDsk == nullptr) { // no hay disco
        debug_fdc("FDC:: writeData() no hay disco\n");
        regEstado0 = R0_NR_NOT_READY;
    }
    else if (currentDsk->isProtected()) {
        regEstado0 = R0_NR_NOT_READY;
        if (p_cabezal == 1  &&  currentDsk->getSides() == 1) regEstado0 = R0_NR_NOT_READY;
        regEstado1 = R1_NW_NOT_WRITEABLE;
    }
    else {
        debug_fdc("FDC:: do write_data\n");
        T_SectorInfo sectorInfo;
        bool sectorEncontrado = currentDsk->getSectorInfo_ID(p_pista, p_cabezal, p_sector, &sectorInfo);
        if (sectorEncontrado) {
            debug_fdc("FDC:: writeData() sector encontrado\n");
            debug_fdc("FDC:: writeData() comprobacion tamanios sector %d,%d\n", p_tamSector, sectorInfo.sectorSize);
            u16 sectorSize = sectorInfo.getDataLength();  // ((u16) p_tamSector) << 8;
            puntBuffer = buffer = new BYTE[sectorSize];
            finBuffer = buffer + sectorSize;
            bytesCopiados = false;
            //f_fdcBusy = true;
            fase = EXECUTION_PHASE;

            regEstado0 = R0_IC_COMANDO_INTERRUMPIDO | p_unidad;   
            if (p_cabezal)  regEstado0 |= R0_HD_HEAD_ADDRESS;
            regEstado1 = R1_EN_END_OF_TRACK; // puesto tras operacion COPY de CPM
        }
        else {
            debug_fdc("FDC:: writeData() sector no encontrado\n");
        }
    }
    setOutputBytes_RS012_CHRN(p_pista, p_cabezal, p_sector, p_tamSector);
}

void FDC:: writeData_execution(BYTE dato) {
    // llenar en buffer
    *puntBuffer = dato;
    //debug_fdc("FDC:: write %02X\t", dato);
    if (++puntBuffer == finBuffer) {
        //debug_fdc("DATOS WRITE: ");
        //printBuffer();
        debug_fdc("\nFDC:: writeSector pista=%d cabezal=%d sector=%02X\n", p_pista, p_cabezal, p_sector);
        //disk[p_unidad]->writeSector(p_pista, p_cabezal, p_sector, buffer);
        currentDsk->writeSectorData(BYTES_SECTOR(p_tamSector), buffer);
        delete buffer;
        bytesCopiados = true;
        fase = RESULT_PHASE;
        dio = FDC_TO_CPU; // los siguientes datos seran la transferencia del resultado
    }
}

void FDC:: writeDeletedData() {
    debug_fdc("FDC:: ---------- writeDeletedData()\n");
    writeData();
}


void FDC:: readData() {
    // puede leer varios sectores
    debug_fdc("FDC:: readData() ---------------------------------------\n");
    getDriveHead();
    getParams_CHRN_EOT_GPL_DTL();
    contadorReady = CICLOS_ENTRE_LECTURAS;
    ready = false;

    if (currentDsk == nullptr) { // no hay disco
        // cuando se ejecuta CPM no hay un readId previo, sino que se intenta leer
        // directamente el sector 0x41 de la pista 0
        debug_fdc("FDC:: readData() no hay disco\n");
        regEstado0 = R0_IC_COMANDO_INTERRUMPIDO | R0_NR_NOT_READY;
        regEstado1 = R1_OR_OVER_RUN;
        regEstado2 = 0;
        fase = RESULT_PHASE;
        //f_fdcBusy = true;
        setOutputBytes_RS012_CHRN(0,0,0,0);
        return;
    }
    else if (currentDsk->existsTrack(p_pista)) {     // }, p_cabezal)) {
        T_DskTrackInfo trackInfo;
        currentDsk->getTrackInfo(p_pista, p_cabezal, &trackInfo);
        //debug_fdc("FDC:: readData() numero de sectores en la pista=%d\n", trackInfo.sectors);
        if (trackInfo.isFormatted()) { // pista formateada
            //debug_fdc("FDC:: readData() pista formateada\n");
            T_SectorInfo sectorInfo;
            if (currentDsk->getSectorInfo_ID(p_pista, p_cabezal, p_sector, &sectorInfo)) { // sector encontrado
                if (sectorInfo.fdcSt1 & R1_DE_DATA_ERROR  ||  sectorInfo.fdcSt2 & R2_DD_DATA_ERROT_IN_DATA) {
                    regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
                    regEstado1 = R1_ND_NO_DATA;
                    fase = RESULT_PHASE;
                }
                else {
                    //debug_fdc("FDC:: readData() sector encontrado\n");
                    u8 numSectores = p_ultSectorPista - p_sector + 1;
                    //debug_fdc("FDC:: readData() num sectores: %d\n", numSectores);
                    u16 bufferSize = BYTES_SECTOR(p_tamSector) * numSectores;
					debug_fdc("FDC:: read_data()  n_sectores=%d buffer_size=%d  tam_sector=%02X\n", 
								numSectores, bufferSize, p_tamSector);
                    puntBuffer = buffer = new BYTE[bufferSize];
                    //debug_fdc("FDC:: readData() buffer_size=%d\n", bufferSize);
                    finBuffer = buffer + bufferSize;
                    //currentDsk->readSectorData(bufferSize, buffer); // obtener datos
					currentDsk->readSectorData(bufferSize, buffer);
					{
					//	for (u16 i = 0, cont = 0; i < bufferSize; i++) {
					//		printf("%02X ", buffer[i]);
					//		if (++cont % 48 == 0) printf("\n");
					//	}
					}
                    // comprobamos que tenemos los datos que queremos
                    //debug_fdc("DATOS SECTOR DISCO: ");
                    //printBuffer();
                    regEstado0 = R0_IC_COMANDO_INTERRUMPIDO; // realmente no es un error ni comando interrumpido, ver documentacion
                    regEstado1 = R1_EN_END_OF_TRACK; // en el emulador caprice se ve que para todas las lecturas correctas devuelve 0x40 0x80
                    bytesCopiados = false;
                    //f_fdcBusy = true;
                    fase = EXECUTION_PHASE;
                }
            }
            else {
                debug_fdc("FDC:: sector no encontrado\n");
                regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
                regEstado1 = R1_ND_NO_DATA;
                regEstado2 = 0;
                fase = RESULT_PHASE;
            }
        }
        else {
            debug_fdc("FDC:: readData() pista no formateada\n");
            regEstado0 |= R0_IC_COMANDO_INTERRUMPIDO;
            regEstado1 |= R1_MA_MISSING_ADDRESS_MARK;
            fase = RESULT_PHASE;
        }
    }
    else {
        regEstado2 = R2_WC_WRONG_CYLINDER;
        if (p_pista == 0xFF) regEstado2 |= R2_BC_BAD_CYLINDER;
    }
    regEstado0 |= p_unidad;
    setOutputBytes_RS012_CHRN(p_pista, p_cabezal, p_ultSectorPista, p_tamSector);
}

void FDC:: readDeletedData() {
    debug_fdc("FDC:: ---------- readDeletedData()\n");
    readData();
}


// no hay bytes de salida
void FDC:: recalibrate() {
    debug_fdc("FDC:: ---------- recalibrate()\n");
    // HU
    getDriveHead();
    f_driveBusy[p_unidad] = true;
    f_fdcBusy = false;
    currentDsk = disk[p_unidad];
    p_pista = currentTrack[p_unidad] = 0;
    f_seek[p_unidad] = true; 
    fase = FREE_PHASE;
}


void FDC:: senseInterruptStaus() {
    // en funcionamiento no dma, el fdc genera interrupciones en los siguientes casos:
    // durante la fase de ejecucion, al inicio de la fase de resultados, al finalizar un recalibrado o un seek,
    // si varia la señal Ready de alguna de las unidades
    debug_fdc("FDC:: ---------- senseInterruptStaus()  unidad=%d\n", p_unidad);
    //bytesSalida[0] = estado0;
    //bytesSalida[1] = numero pista tras orden de busqueda
    //for (u8 i = 0; i < 4; i++) f_driveBusy[i] = false;
    SET4(f_driveBusy, false);
    
    fase = RESULT_PHASE;
    //f_fdcBusy = true;
    regEstado0 = 0;

    //debug_fdc("f_seek "); PRINT4(f_seek);
    //debug_fdc("f_statusChanged "); PRINT4(f_statusChanged);
    
    bool cambio = false;
    // mirar en que disco se ha completado el seek
    for (u8 d = 0; d < 4 && !cambio; d++) {
        if (f_seek[d]) {
            debug_fdc("FDC:: ssi1 %d\n", d);
            regEstado0 |= R0_SE_SEEK_END | d;
            //SET4(f_seek, false); //f_seek[d] = false;
            //SET4(f_statusChanged, false); //f_seek[d] = false;
            f_seek[d] = false;
            f_statusChanged[d] = false;
            bytesSalida[1] = currentTrack[d];
            cambio = true;
        }
    }
    if (!cambio) { // invalid comand
        for (u8 d = 0; d < 4 && !cambio; d++) {
            if (f_statusChanged[d]) {
                debug_fdc("FDC:: ssi2 %d\n", d);
                regEstado0 |= R0_IC_COMANDO_INTERRUMPIDO2 | d;  // comando interrumpido
                //if (disk[d] == nullptr || (!disk[d]->isFormatted() || !motorOn))  regEstado0 |= R0_NR_NOT_READY;
                //SET4(f_statusChanged, false); //f_seek[d] = false;
                f_statusChanged[d] = false;
                bytesSalida[1] = currentTrack[d];
                cambio = true;
            }
        }
    }
    if (!cambio) {
        regEstado0 = R0_IC_COMANDO_INVALIDO;
        numBytesSalida--; // solo se devuelve un byte
    }
    bytesSalida[0] = regEstado0;
}




void FDC:: readId() {
    debug_fdc("FDC:: ---------- readId() ----------\n");
    getDriveHead();

    fase = RESULT_PHASE;
    //f_fdcBusy = true;

    if (currentDsk == nullptr) { // no hay disco
        debug_fdc("FDC:: readId() no hay disco\n"); FLUSH;
        regEstado0 = 0x48; // comando interrumpido + not ready
        regEstado1 = regEstado2 = 0;
        setOutputBytes_RS012_CHRN(0,0,0,0);
    }
    //else if (!activeDsk->isFormatted()) { // disco no formateado
    //    debug_fdc("FDC:: readId() disco no formateado\n");
    //    regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
    //    regEstado1 = R1_MA_MISSING_ADDRESS_MARK;
    //    setOutputBytes_RS012_CHRN(0,0,0,0);
    //}
    else if (!currentDsk->existsTrackSide(p_pista, p_cabezal)) { // pista no encontrada
        debug_fdc("FDC:: readId() pista no existe\n"); FLUSH;
        regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
        regEstado1 = R1_MA_MISSING_ADDRESS_MARK;
        setOutputBytes_RS012_CHRN(0,0,0,0);
    }
    else {
        debug_fdc("FDC:: readId() obtener\n"); FLUSH;
        currentSide[p_unidad] = p_cabezal;
        if (p_cabezal == 1 && currentDsk->getSides() == 0) {
            p_cabezal = currentSide[p_unidad] = 0;
        }
        currentTrack[p_unidad] = p_pista;
        
        T_DskTrackInfo trackInfo;
        currentDsk->getTrackInfo(p_pista, p_cabezal, &trackInfo);
        
        if (++p_sector > trackInfo.sectors) p_sector = 1;

        T_SectorInfo sectorInfo;
        currentDsk->getSectorInfo_N(p_pista, p_cabezal, p_sector, &sectorInfo);
        //p_sector = sectorInfo.sectorId;
        p_tamSector = sectorInfo.sectorSize;
        //debug_fdc("FDC:: readId() sector=%02X tam=%02X\n", sectorInfo.sectorId, p_tamSector);
        regEstado0 = p_unidad;
        regEstado1 = 0;
        setOutputBytes_RS012_CHRN(p_pista, p_cabezal, sectorInfo.sectorId, p_tamSector);
    }
	debug_fdc("FDC:: fin readId\n");
}



// TODO: contemplar errores
void FDC:: formatTrack() {
    debug_fdc("FDC:: ---------- formatTrack()\n");
    getDriveHead();
    p_tamSector = bytesEntrada[2];
    p_ultSectorPista = bytesEntrada[3]; // nº sectores en la pista
    p_gap = bytesEntrada[4];
    BYTE p_fillByte = bytesEntrada[5];
    // se supone que formatea la pista despues del seek o recalibrate
    debug_fdc("FDC:: HD_dr=%d HD_hd=%d pista=%d SZ=%d sectores=%d GP=%02X FB=%02X\n", 
            p_unidad, p_cabezal, p_pista,
            p_tamSector, p_ultSectorPista, p_gap, p_fillByte);
    currentDsk = disk[p_unidad];

    regEstado0 = p_unidad;
    regEstado1 = 0;
    regEstado2 = 0;

    if (currentDsk != nullptr) {
        if (p_cabezal) regEstado0 |= R0_HD_HEAD_ADDRESS; // el cabezal seleccionado o el que habia?
        if (currentDsk->getSides() == 1 && p_cabezal == 1) regEstado0 |= R0_NR_NOT_READY;
        if (p_pista > currentDsk->getTracks()) regEstado0 |= R0_IC_COMANDO_INVALIDO;
        if (currentDsk->isProtected()) regEstado1 |= R1_NW_NOT_WRITEABLE;
        
        /*if (activeDsk->getTrackSize() < tamPista  ||  !activeDsk->existsTrackInFile(activeTrack[p_unidad], p_cabezal)) {
            // el tamanio de la pista existente es menor que el que queremos formar, con lo que hay que recolocar datos en disco
            // se podrian crear discos de tamanio irreal al igual que en winape
            activeDsk->reconstruir(p_pista, tamPista);
        }
        else debug_fdc("FDC:: no reconstruir\n");
        */
    }

    formatData = new FormatData(currentTrack[p_unidad], p_cabezal, p_ultSectorPista /*num_sectores*/, 
        p_tamSector, p_gap, p_fillByte);

    fase = EXECUTION_PHASE;
    //f_fdcBusy = true;
    
    setOutputBytes_RS012_CHRN(currentTrack[p_unidad], /*nota*/p_cabezal, p_ultSectorPista/*num.sectores*/, p_tamSector);
                                                     // nota: en monitor se ve un 4
}

void FDC:: formatTrack_execution(BYTE dato) {
    //debug_fdc("FDC:: formatTrack_execution %02X\n", dato);
    formatData->addByte(dato);
    if (formatData->isFull()) {
        debug_fdc("FDC:: formatTrack_execution formatData lleno\n");
        formatData->print();
        if (currentDsk != nullptr) currentDsk->formatTrack(formatData);
        delete formatData;
        //
        fase = RESULT_PHASE;
        dio = FDC_TO_CPU; // los siguientes datos seran la transferencia del resultado
    }
}


// comando 0F - no hay bytes de salida
void FDC:: seek() {
    debug_fdc("FDC:: ---------- seek()\n");
    // HU
    getDriveHead();
    currentTrack[p_unidad] = p_pista = bytesEntrada[2];
    f_driveBusy[p_unidad] = true;

    //if (currentDsk != nullptr) f_seek[p_unidad] = true;
    f_seek[p_unidad] = true;

    //debug_fdc("\tunidad=%d cabezal=%d pista=%d\n", p_unidad, p_cabezal, p_pista);

    fase = FREE_PHASE;

    //if (currentDsk == nullptr || currentDsk->getTracks() == 0 || !motorOn); // 0x48
    if (currentDsk != nullptr  &&  p_pista > currentDsk->getTracks()) 
        currentTrack[p_unidad] = p_pista = currentDsk->getTracks()-1;
}

// TODO contemplar errores
void FDC:: scanEqual() {
    debug_fdc("FDC:: ---------- scanEqual()\n");
    getDriveHead();
    getParams_CHRN_EOT_GPL_DTL();
    debug_fdc("FDC:: unidad=%d cabezal=%d pista=%d\n", p_unidad, p_cabezal, p_pista);

    T_SectorInfo sectorInfo;
    bool encontrado = currentDsk->getSectorInfo_ID(p_pista, p_cabezal, p_sector, &sectorInfo);
    debug_fdc("FDC:: encontrado = %d\n", encontrado);

    if (encontrado) {
        regEstado0 = p_unidad;
        if (p_cabezal) regEstado0 |= R0_HD_HEAD_ADDRESS;

        regEstado1 = 0;
        regEstado2 = R2_SH_SCAN_EQUAL_HIT;

        contador = 0;
        maxContador = BYTES_SECTOR(sectorInfo.sectorSize);
        fase = EXECUTION_PHASE;
    }
    else {
        regEstado0 = R0_IC_COMANDO_INTERRUMPIDO;
        regEstado1 = R1_ND_NO_DATA;
        regEstado2 = R2_DD_DATA_ERROT_IN_DATA;
        fase = RESULT_PHASE;
    }
    
    setOutputBytes_RS012_CHRN(p_pista, p_cabezal, p_sector, p_tamSector);
}


void FDC:: scan_execution(BYTE dato) {
    contador++;
    if (contador == maxContador) {
        fase = RESULT_PHASE;
        dio = FDC_TO_CPU;
    }
}

void FDC:: scanSlowOrEqual() {
    debug_fdc("FDC:: ---------- scanSlowOrEqual()\n");
    scanEqual();
}

void FDC:: scanHighOrEqual() {
    debug_fdc("FDC:: ---------- scanHighOrEqual()\n");
    scanEqual();
}


void FDC:: invalid() {
    debug_fdc("FDC:: ---------- invalid()\n");
    dio = FDC_TO_CPU;
    //f_fdcBusy = true;
    fase = RESULT_PHASE;
    bytesSalida[0] = 0x80;  // noel llopis lo espera
}


DSK* FDC:: getDisk(u8 drive) {
    if (drive >= 2) debug_fdc("FDC::  jaaaaarl!!\n");

    if (drive < 4)
        return disk[drive];
    else
        return nullptr;
}


void FDC:: setSnaData(SNA_FDC* sna) {
	led = sna->led;

	memcpy(currentTrack, sna->currentTracks, MAX_DRIVES);
}

void FDC:: getSnaData(SNA_FDC* sna) {
	sna->led = led;

	memcpy(sna->currentTracks, currentTrack, MAX_DRIVES);
}
