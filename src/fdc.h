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

#pragma once


#include "tipos.h"
#include "io_device.h"
#include "dsk.h"
#include "sna.h"


#define MAX_DRIVES 4

// comandos
constexpr BYTE CMD_READ_TRACK            = 0x02;
constexpr BYTE CMD_SPECIFY               = 0x03;
constexpr BYTE CMD_SENSE_DRIVE_STATUS    = 0x04;
constexpr BYTE CMD_WRITE_DATA            = 0x05;
constexpr BYTE CMD_READ_DATA             = 0x06;
constexpr BYTE CMD_RECALIBRATE           = 0x07;
constexpr BYTE CMD_SESE_INTERRUPT_STATUS = 0x08;
constexpr BYTE CMD_WRITE_DELETED_DATA    = 0x09;
constexpr BYTE CMD_READ_ID               = 0x0A;
constexpr BYTE CMD_READ_DELETED_DATA     = 0x0C;
constexpr BYTE CMD_FORMAT_TRACK          = 0x0D;
constexpr BYTE CMD_SEEK                  = 0x0F;
constexpr BYTE CMD_SCAN_EQUAL            = 0x11;
constexpr BYTE CMD_SCAN_LOW_OR_EQUAL     = 0x19;
constexpr BYTE CMD_SCAN_HIGH_OR_EQUAL    = 0x1D;
constexpr BYTE CMD_NONE                  = 0xFF;

// main register
constexpr BYTE MR_RQM_READY          = 0x80; // (1=ready for next byte)
constexpr BYTE MR_DIO_CPU_TO_FDC     = 0x00;
constexpr BYTE MR_DIO_FDC_TO_CPU     = 0x40;
constexpr BYTE MR_EXM_EXECUTION_MODE = 0x20; // (still in execution-phase, non_DMA_only)
constexpr BYTE MR_FDC_BUSY           = 0x10; // (still in command-, execution- or result-phase)

// registro 0
constexpr BYTE R0_HD_HEAD_ADDRESS    = 0x04; // (head during interrupt)
constexpr BYTE R0_NR_NOT_READY       = 0x08; // (drive not ready or non-existing 2nd head selected)
constexpr BYTE R0_EC_EQUIPMENT_CHECK = 0x10; // (drive failure or recalibrate failed (retry))
constexpr BYTE R0_SE_SEEK_END        = 0x20; // (Set if seek-command completed)
// codigos de interrupcion
constexpr BYTE R0_IC_COMANDO_OK            = 0x00;
constexpr BYTE R0_IC_COMANDO_INTERRUMPIDO  = 0x40;
constexpr BYTE R0_IC_COMANDO_INVALIDO      = 0x80;
constexpr BYTE R0_IC_COMANDO_INTERRUMPIDO2 = 0xC0;

// registro 1
constexpr BYTE R1_MA_MISSING_ADDRESS_MARK = 0x01; // Missing Address Mark (Sector_ID or DAM not found)
constexpr BYTE R1_NW_NOT_WRITEABLE        = 0x02; // (tried to write/format disc with wprot_tab=on)
constexpr BYTE R1_ND_NO_DATA              = 0x04; // (Sector_ID not found, CRC fail in ID_field)
constexpr BYTE R1_OR_OVER_RUN             = 0x20; // (CPU too slow in execution-phase (ca. 26us/Byte))
constexpr BYTE R1_DE_DATA_ERROR           = 0x40; // (CRC-fail in ID- or Data-Field)
constexpr BYTE R1_EN_END_OF_TRACK         = 0x80; // (set past most read/write commands) (see IC)

// registro 2
constexpr BYTE R2_MD_MISSING_ADDRESS_MARK_IN_DATA_FIELD = 0x01;  // (DAM not found)
constexpr BYTE R2_BC_BAD_CYLINDER                       = 0x02;// (read/programmed track-ID different and read-ID = FF)
constexpr BYTE R2_SN_SCAN_NOT_SATISFIED                 = 0x04;  // (no fitting sector found)
constexpr BYTE R2_SH_SCAN_EQUAL_HIT                     = 0x08; // (equal)
constexpr BYTE R2_WC_WRONG_CYLINDER                     = 0x10; // (read/programmed track-ID different) (see b1)
constexpr BYTE R2_DD_DATA_ERROT_IN_DATA                 = 0x20; // (CRC-fail in data-field)
constexpr BYTE R2_CM_CONTROL_MARK                       = 0x40; // (read/scan command found sector with deleted DAM)

// registro 3
constexpr BYTE R3_US_UNIT_SELECT     = 0x03; // (pin 28,29 of FDC)
constexpr BYTE R3_HD_HEAD_ADDRESS    = 0x04; // (pin 27 of FDC)
constexpr BYTE R3_TS_TWO_SIDE        = 0x08; // (0=yes, 1=no (!))
constexpr BYTE R3_T0_TRACK_0         = 0x10; // (on track 0 we are)
constexpr BYTE R3_RY_READY           = 0x20; // (drive ready signal)
constexpr BYTE R3_WP_WRITE_PROTECTED = 0x40; // (write protected)
constexpr BYTE R3_FT_FAULT           = 0x80; // (if supported: 1=Drive failure)

// TODO: ajustar estos dos valores
constexpr u8 CICLOS_ENTRE_LECTURAS   = 26;
constexpr u8 CICLOS_ENTRE_ESCRITURAS = 30;


enum FDC_Fase { LIBRE, FASE_COMANDO, FASE_EJECUCION, FASE_RESULTADO};
enum FDC_DIO  { CPU_A_FDC, FDC_A_CPU}; // 0=CPU->FDC, 1=FDC->CPU



class FDC : public IODevice
{
    DSK* currentDsk;
    DSK* disk[MAX_DRIVES] = {nullptr};   // el controlador puede con 4 unidades
    u8 currentSide[MAX_DRIVES] = {0};
    u8 currentTrack[MAX_DRIVES] = {0};
    u8 currentSector[MAX_DRIVES] = {0};

    // FLAGS varios
    bool f_seek[MAX_DRIVES] = {false};
    bool f_statusChanged[MAX_DRIVES] = {false};
    //bool f_scanFailed;
    //bool f_overrun;
    //bool f_skip;
    // estas se utilizan para el registro principal de estado
    bool f_driveBusy[MAX_DRIVES] = {false};
    bool f_fdcBusy;
    FDC_Fase fase;
    FDC_DIO dio;

    BYTE regEstado0, regEstado1, regEstado2;
    
    bool led = false; // por si se visualiza el led de la disquetera
    i8 contadorReady = 0; // para emular los tiempos normales
    bool ready = true; // aplica cuando turboMode=false, cuando se escribe o lee hay un tiempo entre lecturas o escrituras
    bool turboMode = false; // para que vaya mas rapido, si esta a true RQM siempre sera 1
    bool motorOn = false; // estado del motor

    u8 comando, contBytesEntrada, contBytesSalida, numBytesEntrada, numBytesSalida;
    BYTE bytesEntrada[8], bytesSalida[7]; // buffers para el comando y parametros, y resultado
    bool bytesCopiados; // indica si se ha terminado de copiar los bytes
    BYTE *buffer, *finBuffer, *puntBuffer; // buffer, fin del buffer, puntero que se mueve entre buffer(inicio) y finBuffer
    u16 contador, maxContador;

    // parametros de los comandos
    bool mt_multiTrack, mf_mfmMode, sk_skip;
    u8 p_unidad; 
    //, p_1_cabezal; se envia el head dos veces, pero parece que el bueno es el que llega en el 2o byte
    u8 p_pista, p_cabezal, p_sector, p_tamSector, p_ultSectorPista, p_gap, p_longDatosSi;

    typedef void (FDC::*PtrFuncion)();
    PtrFuncion ptrFuncion;

    // punteros a las estructuras que se crean en el proceso de formateo
    FormatData* formatData;
    ReadTrackData* readTrackData;

    void prepareCommandFDC(u8 numBytesEntrada, u8 numBytesSalida, FDC_DIO _dio, bool led, PtrFuncion funcion);

    void getParam_MT();
    void getParam_MF();
    void getParam_SK();
    void getDriveHead(); // obtiene la unidad Y el cabezal del comando

    void getParams_CHRN_EOT_GPL_DTL(); 
    // obtiene los parametros num_pisca,cabezal,sector,long_sector,ult_sector,espacio,long_datos
    // los mismos parametros se utilizan en varios comandos

    void setOutputBytes_RS012_CHRN(BYTE pista, BYTE cabezal, BYTE sector, BYTE tamSector);
    // envia el resultado regEstado0,regEstado1,regEstado2,num_pista,cabezal,sector,num_sector
    // el mismo formato de resultado se envia en varios comandos

    // funciones de los comandos
    void seek();    // busqueda de una pista
    void senseDriveStatus(); // leer estado de unidad
    void specify(); // dar datos de unidad
    void senseInterruptStaus(); // leer estado interrupciones
    void recalibrate(); // busqueda de la pista 0
    void readData();
    void readDeletedData();
    void writeData();
    void writeData_execution(BYTE dato);
    void writeDeletedData();
    void readTrack();
    void readTrack_execution(BYTE* dato);
    void readTrack_readSector();
    void readId();
    void formatTrack();
    void formatTrack_execution(BYTE dato);
    void scanEqual();
    void scan_execution(BYTE dato);
    void scanSlowOrEqual();
    void scanHighOrEqual();
    void invalid();

    void out_FB7F(BYTE dato);

    BYTE getMainStatusReg();
        
public:
    
    FDC();
    void reset() override;

    void setTurboMode(bool turbo); // para reducir las esperas del dato disponibles
    
    bool OUT(WORD puerto, BYTE dato) override;
    bool IN(WORD puerto, BYTE* dato) override;
    void update(u8 ciclos) override;
    bool getLED();  // devuelve true si el FDC esta en la fase de ejecucion de comandos que impliquen leer o escribir
  
    void setDisk(u8 drive, DSK* dsk);
    DSK* getDisk(u8 drive); // 0=A 1=B

	void setSnaData(SNA_FDC* sna);
	void getSnaData(SNA_FDC* sna);

    void printBuffer(); // para pruebas
};
