/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Z80 cpu implementation                                                    |
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

/**
    http://www.z80.info/z80info.htm
**/

#include <stdio.h>
#include <string>
#include "log.h"
#include "tipos.h"
#include "z80.h"
#include "memory.h"
#include "io.h"
#include "io_device.h"
#include "sna.h"
#include "compilation_options.h"



Z80::Z80(Memory* memoria, IO_Bus *io) {
    debug_z80("Z80\n");
    this->memoria = memoria;
    this->io = io;
    
    reset();
}

//Z80:: ~Z80() {}

/* Según el documento de Sean Young, que se encuentra en
 * [http://www.myquest.com/z80undocumented], la mejor manera de emular el
 * reset es poniendo PC, IFF1, IFF2, R e IM0 a 0 y todos los demás registros
 * a 0xFFFF.
 *
 * cuando la CPU recibe alimentación por primera vez, los
 * registros PC,I y R se inicializan a cero y el resto a 0xFF.
 * Si se produce un reset a través de la patilla correspondiente,
 * los registros PC e IR se inicializan a 0 y el resto se preservan.
 * En cualquier caso, todo parece depender bastante del modelo
 * concreto de Z80, así que se escoge el comportamiento del
 * modelo Zilog Z8400APS. Z80A CPU.
 * http://www.worldofspectrum.org/forums/showthread.php?t=34574
 */
void Z80::reset() {
    debug_z80("Z80:: reset\n");
    IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1 = false;
    interrupcionSolicitada = 0;
    PC = R = R7 = 0;
    IM = 1;

    AF = BC = DE = HL = AF2 = BC2 = DE2 = HL2 = IX = IY = SP = 0 ;//xFFFF;
    /*t_states =*/ m_cycles = 0;
    updateFlags();
}

u64 Z80:: getCiclos() { return m_cycles; }

u8 Z80:: getInstructionCycles() { return ciclosInstr; }

//u32 Z80:: get_t_states() { return t_states; }

void Z80:: reset_m_cycles() { m_cycles = 0; }

inline void Z80:: addCiclos(u8 n) {
    //t_states += n;
    n = (n+3) >> 2; // 1 t-state -> 1 m_cycle ; 4 t-states -> 1 m_cycle ; 5 t-states -> 2 m_clcyes
    m_cycles += n;
    ciclosInstr += n;
}

//------------------------------------------------- flags

void Z80::updateRegF() {
    F = 0;
    if (flag_sign)      F |= BIT_SIGN;
    if (flag_zero)      F |= BIT_ZERO;
    if (flag_halfcarry) F |= BIT_HALFCARRY;
    if (flag_pv)        F |= BIT_PV;
    if (flag_sub)       F |= BIT_SUB;
    if (flag_carry)     F |= BIT_CARRY;
    if (flag_bit5)      F |= BIT_BIT5;
    if (flag_bit3)      F |= BIT_BIT3;
}

void Z80::updateFlags() {
    flag_sign      = F & BIT_SIGN;
    flag_zero      = F & BIT_ZERO;
    flag_halfcarry = F & BIT_HALFCARRY;
    flag_parity    = F & BIT_PARITY;
    flag_sub       = F & BIT_SUB;
    flag_carry     = F & BIT_CARRY;
    flag_bit5      = F & BIT_BIT5;
    flag_bit3      = F & BIT_BIT3;
}

inline void Z80::update_flags_53(BYTE v)  { flag_bit5 = v & BIT_BIT5; flag_bit3 = v & BIT_BIT3; }
inline void Z80::update_flag_sign(BYTE v) { flag_sign = v & 0x80; }
inline void Z80::update_flag_zero(BYTE v) { flag_zero = (v == 0); }

void Z80::update_flag_parity(BYTE v) {
    flag_parity = 1;
    
    if (v & 0b00000001) flag_parity ^= 1;
    if (v & 0b00000010) flag_parity ^= 1;
    if (v & 0b00000100) flag_parity ^= 1;
    if (v & 0b00001000) flag_parity ^= 1;
    if (v & 0b00010000) flag_parity ^= 1;
    if (v & 0b00100000) flag_parity ^= 1;
    if (v & 0b01000000) flag_parity ^= 1;
    if (v & 0b10000000) flag_parity ^= 1;
}

inline BYTE Z80::carry() { return (flag_carry) ? 1 : 0; }

//----------------------------------------------

void Z80:: executeNextInstruction () {
    //debug("%04X: ", PC);
    //debug("%02X", opcode);
    ciclosInstr = 0;

    bool intAtendida = false;

    if (ei_ejecutado)     // ei_ejecutado > 0
        if (--ei_ejecutado == 0)
            IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1 = true;

    if (IFF1_interrupcionesHabilitadas && interrupcionSolicitada) {
        atenderInterrupcion();
        intAtendida = true;
    }

    if (!intAtendida) {
        pc_getByte(&opcode);
        R = (++R & 0x7F); // R++;

        ptrFuncion = instrucciones_principales[opcode];
        (this->*ptrFuncion)();
    }
    //else {
    //    debug("int atendida\n");
    //}
}

void Z80:: requestInterrupt() {
    //debug_z80("interrupcion solicitada  Int=%d  IM=%d T=%d\n", IFF1_interrupcionesHabilitadas, IM, m_cycles);
   	interrupcionSolicitada = true;
}

void Z80:: atenderInterrupcion() {
	// para debug:
    debug_z80("Z80:: atender interrupcion, no se ejecute la instruccion anterior, PC=#38\n");
	
	R = (++R) & 0x7F;
	IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1 = false;
	interrupcionSolicitada = false;
	
	if (receptorInterrupcionAtendida != nullptr) 
		receptorInterrupcionAtendida->notifyInterruptAttended();

	if (haltActivo) {
		haltActivo = false;
		PC++;
	}
#ifdef debug_ints_borde
	if (displayInt != nullptr) {
		displayInt->startInt();
		pcInt = regPC;
	}
#endif
	if (IM < 2) {
		_push(regPC);
		PC = 0x0038;
		addCiclos(17);
		//debug("salto a %04X\n", PC);
	}
	else {  // IM=2
		_push(regPC);
		WORD x;
		x.b.h = I;
		x.b.l = 0xFF;
		memoria->readWord(x.w, &regPC);
	}
}


//-----------------------------------------------------------------

// obtiene el byte apuntado por PC, despues se incrementa PC
inline void Z80:: pc_getByte(BYTE* dato) { memoria->readByte(PC++, dato); }

// obtiene el word apuntado por PC, despues PC se incrementa en 2
inline void Z80:: pc_getWord(WORD* dato) { memoria->readWord(PC, dato); PC += 2; }

inline void Z80:: getDatoHL() { memoria->readByte(HL, &datoIndir); }
inline void Z80:: saveDatoHL() { memoria->writeByte(HL, datoIndir); }
inline void Z80:: getDatoIndir() { memoria->readByte(indir, &datoIndir); }
inline void Z80:: saveDatoIndir() { memoria->writeByte(indir, datoIndir); }
inline void Z80:: pc_getDatoN() { pc_getByte(&instruccion_datoN); }
inline void Z80:: pc_getDatoNN() { pc_getWord(&instruccion_datoNN); }

inline void Z80:: pc_getDatoOffset () { BYTE b; memoria->readByte(PC++, &b); instruccion_offset = (SBYTE) b; }
inline void Z80:: updateIndir() { indir = regIdx16->w + instruccion_offset; }

    //b5 = (opcode & 0b00111000) >> 3;
    //opcode_reg8dst = orden_registros8[b5];
inline void Z80:: opcode_getReg8dst() { opcode_reg8dst = orden_registros8[ (opcode & 0b00111000) >> 3 ]; }

    //b5 = (opcode & 0b00000111);
    //opcode_reg8src = orden_registros8[b5];
inline void Z80::opcode_getReg8src()  { opcode_reg8src = orden_registros8 [opcode & 0b00000111]; }

void Z80::opcode_getReg8src2() { 
    // para las instrucciones con S={a,b,c,d,e,Ih,Il}
    u8 r = opcode & 0b00000111;
    if      (r == 4) opcode_reg8src = regIdxH;
    else if (r == 5) opcode_reg8src = regIdxL;
    else             opcode_reg8src = orden_registros8[r];
}

// vale para las instruccriones inc dec cp add sub adc sbc, ld Ih,?  y ld Il,?
inline void Z80::opcode_getReg8dst2() {  opcode_reg8dst = ((opcode & 0x08) == 0)  ?  regIdxH : regIdxL ;  }

    //b5 = (opcode & 0b00110000) >> 4;
    //opcode_reg16 = orden_registros16[b5];
inline void Z80::opcode_getReg16() {  opcode_reg16 = orden_registros16[ (opcode & 0b00110000) >> 4 ];  }

void Z80::opcode_getReg16_2() {  
    u8 r = (opcode & 0b00110000) >> 4;
    if (r == 2) opcode_reg16 = regIdx16;
    else        opcode_reg16 = orden_registros16[r];  
}

    //b5 = (opcode & 0b00110000) >> 4;
    //opcode_reg16 = orden_registros16[b5];
inline void Z80::opcode_getReg16_pushpop() { opcode_reg16 = orden_registros16_pushpop[ (opcode & 0b00110000) >> 4 ]; }

inline BYTE Z80::opcode_getMascaraBit() { return mascaraBit[ (0b00111000 & opcode) >> 3 ]; }

BYTE Z80::opcode_getResultadoCondicionDirAbs() {
    /*   nz 00 0
          z 00 1
         nc 01 0
          c 01 1
         po 10 0
         pe 10 1
          p 11 0
          m 11 1    */
    BYTE f = (opcode & 0b00110000) >> 4;
    BYTE flag1 = (opcode & 0b00001000);   // si != 0, pregunta si el flag esta a 1
    f = *orden_flags[f];   // valor del flag
    //printf("  fv:%d 0/1:%d r:%d", f, flag1, (f && flag1) || !(f || flag1));
    return (f && flag1) || !(f || flag1);  // (!a Y !b) = !(a O b)
}


//--------------------------------------------------------comunes

void intercambiarRegistros16(WORD* r1, WORD* r2) {
    WORD aux;
    aux.w = r1->w;
    r1->w = r2->w;
    r2->w = aux.w;
}

// actualizar los flags tras las rotaciones
void Z80::updateFlagsRotaciones(BYTE x) {
    flag_sub = false;
    flag_halfcarry = false;
    update_flag_zero(x);
    update_flag_sign(x);
    update_flag_parity(x);
    update_flags_53(x);
}

void Z80::_inc8(BYTE *r8) {  // vale para los normales y los indice
    BYTE x = ++(*r8);
    update_flag_sign(x);
    update_flag_zero(x);
    flag_halfcarry = (x & 0x0F) == 0x00;
    flag_overflow = (x == 0x80);
    flag_sub = false;
    update_flags_53(x);
}

void Z80::_dec8(BYTE *r8) {  // vale para los normales y los indice
    BYTE x = --(*r8);
    update_flag_sign(x);
    update_flag_zero(x);
    flag_halfcarry = (x & 0x0F) == 0x0F;
    flag_overflow = (x == 0x7F);
    flag_sub = true;
    update_flags_53(x);
}

inline void Z80::_push(WORD reg) { SP -= 2; memoria->writeWord(SP, reg); }

inline void Z80::_pop(WORD* reg) { 
    memoria->readWord(SP, reg);
#ifdef debug_ints_borde
    if (displayInt != nullptr  &&  reg->w == pcInt.w) {
		displayInt->endInt();
		debug_z80("salir de la interrupcion\n");
	}
#endif
    SP += 2; 
}

void Z80::_and(BYTE b) {
    rA &= b;
    flag_carry = false;
    flag_sub = false;
    flag_halfcarry = 1;
    update_flag_zero(rA);
    update_flag_sign(rA);
    update_flag_parity(rA);
    update_flags_53(rA);
}

void Z80::_cp(BYTE n) {
    i16 un = n;
    i16 res = rA - un;

    flag_halfcarry = (rA ^ res ^ un) & BIT_HALFCARRY;
    flag_overflow = (((un ^ rA) & (rA ^ res) & 0x80) >> 5) & BIT_PV;
    flag_carry = res & 0x0100;
    
    update_flag_zero(res & 0xFF);
    update_flag_sign(res & 0xFF);
    flag_sub = true;
    // In this case the bits (3 y 5) are copied from the argument 
    update_flags_53(n); // & 0xFF);
}

void Z80::_or(BYTE n) {
    rA |= n;
    update_flag_sign(rA);
    update_flag_zero(rA);
    update_flag_parity(rA);
    flag_halfcarry = false;
    flag_sub = false;
    flag_carry = false;
    update_flags_53(rA);
}

void Z80::_xor(BYTE n) {
    rA ^= n;
    update_flag_sign(rA);
    update_flag_zero(rA);
    update_flag_parity(rA);
    flag_halfcarry = false;
    flag_sub = false;
    flag_carry = false;
    update_flags_53(rA);
}

void Z80::_add8(BYTE n) {
    unsigned res = rA + n;
    flag_carry = res >> 8;
    flag_halfcarry = (rA ^ res ^ n) & BIT_HALFCARRY;
    flag_overflow = ((n ^ rA ^ 0x80) & (n ^ res) & 0x80) >> 5;
    rA = res;
    update_flag_sign(res);
    update_flag_zero(res);
    flag_sub = false;
    update_flags_53(rA);
}

void Z80::_adc8(BYTE n) {
    unsigned res = rA + n + carry();
    flag_carry = res >> 8;
    flag_halfcarry = (rA ^ res ^ n) & BIT_HALFCARRY;
    flag_overflow = ((n ^ rA ^ 0x80) & (n ^ res) & 0x80) >> 5;
    rA = res;
    update_flag_sign(res);
    update_flag_zero(res);
    flag_sub = false;
    update_flags_53(rA);
}

void Z80::_add16(WORD* dst, WORD n) {
    u32 tmp = dst->w + n.w;
    dst->w = static_cast<u16>(tmp);
    flag_carry = (tmp & 0x00010000);
    //flag_halfcarry = ((dst->w & 0x0FFF) < (n.w & 0x0FFF));
    flag_halfcarry = ((dst->w ^ tmp ^ n.w) >> 8) & BIT_HALFCARRY;
    flag_sub = false;
    update_flags_53(dst->b.h);
}

void Z80::_sub8(BYTE n) {
    unsigned un = n;
    unsigned res = rA - un;
    flag_halfcarry = (rA ^ res ^ un) & BIT_HALFCARRY;
    flag_overflow = ((un ^ rA) & (rA ^ res) & 0x80) >> 5;
    flag_carry = (res >> 8) & BIT_CARRY;
    rA = res;
    update_flag_zero(rA);
    update_flag_sign(rA);
    flag_sub = true;
    update_flags_53(rA);
}

void Z80::_sbc8(BYTE n) {
    unsigned un = n;
    unsigned res = rA - un - carry();
    flag_halfcarry = (rA ^ res ^ un) & BIT_HALFCARRY;
    flag_overflow = ((un ^ rA) & (rA ^ res) & 0x80) >> 5;
    flag_carry = (res >> 8) & BIT_CARRY;
    rA = res;
    update_flag_zero(rA);
    update_flag_sign(rA);
    flag_sub = true;
    update_flags_53(rA);
}

void Z80:: _rlc (BYTE* n) {
    BYTE x = *n;
    flag_carry = x & 0x80;
    x = x << 1;
    if (flag_carry) x |= 0x01;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _rrc(BYTE* n) {
    BYTE x = *n;
    flag_carry = x & 0x01;
    x = x >> 1;
    if (flag_carry) x |= 0x80;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _rl(BYTE* n) {
    BYTE x = *n;
    BYTE nuevoCarry = x & 0x80;
    x = x << 1;
    if (flag_carry) x |= 0x01;
    flag_carry = nuevoCarry;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _rr(BYTE* n) {
    BYTE x = *n;
    BYTE nuevoCarry = x & 0x01;
    x = x >> 1;
    if (flag_carry) x |= 0x80;
    flag_carry = nuevoCarry;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _sla(BYTE* n) {
    BYTE x = *n;
    flag_carry = x & 0x80;
    x = x << 1;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _sll(BYTE* n) {
    BYTE x = *n;
    flag_carry = x & 0x80;
    x = (x << 1) | 0x01;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _sra(BYTE* n) {
    BYTE x = *n;
    BYTE bit7 = x & 0x80;
    flag_carry = x & 0x01;
    x = bit7 | (x >> 1);
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _srl(BYTE* n) {
    BYTE x = *n;
    flag_carry = x & 0x01;
    x = x >> 1;
    *n = x;
    updateFlagsRotaciones(x);
}

void Z80:: _bit (BYTE mascara, BYTE n) {
    update_flag_sign(n & mascara);
    flag_halfcarry = true;
    flag_sub = false;
    flag_pv = flag_zero = (n & mascara) == 0;  // mascara solo tiene un bit a 1
    update_flags_53(n); // los bits 3 y 5 son los del operando
}

inline void Z80:: _res (BYTE mascara, BYTE* n) { *n &= ~mascara; }
inline void Z80:: _set (BYTE mascara, BYTE* n) { *n |= mascara; }

//---------------------------------------------

// ----- inst 1 byte -----

void Z80:: ADC_N ()  { pc_getDatoN(); _adc8(instruccion_datoN); addCiclos(7); }
void Z80:: ADC_R ()  { opcode_getReg8src(); _adc8(*opcode_reg8src); addCiclos(4); }
void Z80:: ADC_hl () { getDatoHL(); _adc8(datoIndir); addCiclos(7); }

void Z80:: ADD_HL_RR ()  { opcode_getReg16(); _add16(&regHL, *opcode_reg16); update_flags_53(H); addCiclos(11); }

void Z80:: ADD_N ()  { pc_getDatoN(); _add8(instruccion_datoN); addCiclos(7); }
void Z80:: ADD_R ()  { opcode_getReg8src(); _add8(*opcode_reg8src); addCiclos(4); }
void Z80:: ADD_hl () { getDatoHL(); _add8(datoIndir); addCiclos(7); }

void Z80:: AND_N ()  { pc_getDatoN(); _and(instruccion_datoN); addCiclos(7); }
void Z80:: AND_R ()  { opcode_getReg8src(); _and(*opcode_reg8src); addCiclos(4); }
void Z80:: AND_hl () { getDatoHL(); _and(datoIndir); addCiclos(7); }

void Z80:: BITS ()  {
    pc_getByte(&opcode);
    R = (++R & 0x7F); //R++;
    
    if (opcode < 0x40) {
        //debug("%02X", opcode);
        ptrFuncion = instrucciones_bits[opcode];
        (this->*ptrFuncion)();
    }
    else {
        BYTE instHL = (opcode & 0b00000111) == 0x06;
        
        if (opcode < 0x80) {
            if (instHL) BIT_B_hl();
            else BIT_B_R();
        }
        else if (opcode < 0xC0) {
            if (instHL) RES_B_hl();
            else RES_B_R();
        }
        else {
            if (instHL) SET_B_hl();
            else SET_B_R();
        }
    }
}

void Z80:: CALL_C_NN ()  {
    if (opcode_getResultadoCondicionDirAbs()) {
        CALL_NN();
    }
    else {
        PC += 2;
        addCiclos(10);
    }
}
void Z80:: CALL_NN ()  { pc_getDatoNN(); _push(regPC); PC = instruccion_datoNN.w; addCiclos(17); }
void Z80:: CCF () { 
    flag_halfcarry = flag_carry; //  H, previous carry is copied
    flag_sub = false;
    flag_bit5 = (rA & BIT_BIT5) | flag_bit5 | flag_carry;
    flag_bit3 = rA & BIT_BIT3;
    flag_carry = !flag_carry;
    addCiclos(4); 
}
void Z80:: CPL ()  {
    rA = ~rA;
    flag_sub = true;
    flag_halfcarry = true;
    update_flags_53(rA);
    addCiclos(4);
}
void Z80:: CP_N ()  { pc_getDatoN(); _cp(instruccion_datoN); addCiclos(7); }
void Z80:: CP_R ()  { opcode_getReg8src(); _cp(*opcode_reg8src); addCiclos(4); }
void Z80:: CP_hl () { getDatoHL(); _cp(datoIndir); addCiclos(7); }

inline bool in(u8 valor, u8 limInf, u8 limSup) { return limInf <= valor && valor <= limSup; }

void Z80:: DAA ()  {    // chat-gpt
	//updateRegF();
    BYTE correccion = 0;
    BYTE na = 0;
    bool c = flag_carry;

    if (flag_halfcarry || (rA & 0x0F) > 9) correccion |= 0x06;
    if (flag_carry || rA > 0x99) correccion |= 0x60;
    if (rA > 0x99) c = true;

    if (flag_sub) {
        na = rA - correccion;
        flag_halfcarry = (na & 0x0F) > (rA & 0x0F);
    }
    else {
        na = rA + correccion;
        flag_halfcarry = (na & 0x0F) < (rA & 0x0F);
    }

    rA = na;
    update_flag_sign(rA);
    update_flag_zero(rA);
    update_flag_parity(rA);
    flag_carry = c;
    update_flags_53(rA);
    // flag_sub se mantiene

    addCiclos(4);
}

void Z80:: DEC_R ()  { opcode_getReg8dst(); _dec8(opcode_reg8dst); addCiclos(4); }
void Z80:: DEC_RR () { opcode_getReg16(); opcode_reg16->w--; addCiclos(6); }
void Z80:: DEC_hl () { getDatoHL(); _dec8(&datoIndir); saveDatoHL(); addCiclos(11); }

void Z80:: DI ()  {
    IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1 = false;
    ei_ejecutado = 0;
    //interrupcionSolicitada = 0;
    addCiclos(4);
}

void Z80:: DJNZ_D ()  {
    if (--rB == 0) {
        PC++;
        addCiclos(12);//addCiclos(8);
    }
    else {
        pc_getDatoOffset();
        PC += instruccion_offset; 
        addCiclos(13);
    }
}
void Z80:: EI ()  { 
    //IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1 = true; 
    ei_ejecutado = 2;
    addCiclos(4); 
}

void Z80:: EXX ()  {
    intercambiarRegistros16(&regBC, &regBC2);
    intercambiarRegistros16(&regDE, &regDE2);
    intercambiarRegistros16(&regHL, &regHL2);
    addCiclos(4);
}
void Z80:: EX_AF_AF ()  {
    updateRegF();
    intercambiarRegistros16(&regAF, &regAF2);
    updateFlags();
    addCiclos(4);
}
void Z80:: EX_DE_HL ()  { intercambiarRegistros16(&regDE, &regHL); addCiclos(4); }
void Z80:: EX_sp_HL ()  {
    WORD aux;
    memoria->readWord(SP, &aux);
    intercambiarRegistros16(&regHL, &aux);
    memoria->writeWord(SP, aux);
    addCiclos(23/*19*/);
}

void Z80:: HALT ()  { haltActivo = true; PC--; addCiclos(4); }

void Z80:: INC_R ()   { opcode_getReg8dst(); _inc8(opcode_reg8dst); addCiclos(4); }
void Z80:: INC_RR ()  { opcode_getReg16(); opcode_reg16->w++; addCiclos(6); }
void Z80:: INC_hl ()  { getDatoHL(); _inc8(&datoIndir); saveDatoHL(); addCiclos(11); }

void Z80:: IN_A_n ()  {  // A <- in(A<<8 + N)
    WORD puerto;
    pc_getByte(&puerto.b.l); //pc_getDatoN(); puerto.b.h = instruccion_datoN;
    puerto.b.h = rA;
    rA = io->IN(puerto); 
    addCiclos(11); 
}

void Z80:: JP_C_NN ()  {
    if (opcode_getResultadoCondicionDirAbs()) {
        JP_NN();
    }
    else {
        PC += 2;
        addCiclos(10);
    }
}
void Z80:: JP_NN ()  { pc_getDatoNN(); PC = instruccion_datoNN.w; addCiclos(10); }
void Z80:: JP_hl ()  { PC = HL; addCiclos(4); }

void Z80:: JR_C_D ()  { 
//    bool resultadoCondicion = false;
//         if (opcode == 0x20/*nz*/  &&  flag_zero  == 0) resultadoCondicion = true;
//    else if (opcode == 0x28/*z*/   &&  flag_zero  != 0) resultadoCondicion = true;
//    else if (opcode == 0x30/*nc*/  &&  flag_carry == 0) resultadoCondicion = true;
//    else if (opcode == 0x38/*c*/   &&  flag_carry != 0) resultadoCondicion = true;
    bool resultadoCondicion = (opcode == 0x20/*nz*/  &&  flag_zero  == 0) ||
        (opcode == 0x28/*z*/   &&  flag_zero  != 0) ||
        (opcode == 0x30/*nc*/  &&  flag_carry == 0) ||
        (opcode == 0x38/*c*/   &&  flag_carry != 0) ;

    if (resultadoCondicion) {
        JR_D();
    }
    else {
        PC++;
        addCiclos(7);
    }
}
void Z80:: JR_D ()  { pc_getDatoOffset(); PC += instruccion_offset; addCiclos(12); }

void Z80:: LD_R_hl ()  { opcode_getReg8dst(); memoria->readByte(HL, opcode_reg8dst); addCiclos(7); }
void Z80:: LD_A_nn ()  { pc_getDatoNN(); memoria->readByte(instruccion_datoNN.w, &rA); addCiclos(13); }
void Z80:: LD_A_rr ()  { opcode_getReg16(); memoria->readByte(opcode_reg16->w, &rA); addCiclos(7); }
void Z80:: LD_HL_nn () { pc_getDatoNN(); memoria->readWord(instruccion_datoNN.w, &regHL); addCiclos(20); }
void Z80:: LD_RR_NN () { opcode_getReg16(); pc_getDatoNN(); *opcode_reg16 = instruccion_datoNN; addCiclos(10); }
void Z80:: LD_R_N ()   { opcode_getReg8dst(); pc_getByte(opcode_reg8dst); addCiclos(7); }
void Z80:: LD_R_R ()   { opcode_getReg8dst(); opcode_getReg8src(); *opcode_reg8dst = *opcode_reg8src; addCiclos(4); }
void Z80:: LD_SP_HL () { SP = HL; addCiclos(6); }
void Z80:: LD_hl_N ()  { pc_getDatoN(); memoria->writeByte(HL, instruccion_datoN); addCiclos(10); }
void Z80:: LD_hl_R ()  { opcode_getReg8src(); memoria->writeByte(HL, *opcode_reg8src); addCiclos(7); }
void Z80:: LD_nn_HL () { pc_getDatoNN();  memoria->writeWord(instruccion_datoNN.w, regHL); addCiclos(20); }
void Z80:: LD_rr_A ()  { opcode_getReg16(); memoria->writeByte(opcode_reg16->w, rA); addCiclos(7); }

void Z80:: MISC ()  {
    pc_getByte(&opcode);
    R = (++R & 0x7F);

    ptrFuncion = instrucciones_misc[opcode - 0x40]; // el array es mas pequeño
    if (ptrFuncion != nullptr)  (this->*ptrFuncion)();
}


void Z80:: NOP ()  { addCiclos(4); }
void Z80:: OR_N ()  { pc_getDatoN(); _or(instruccion_datoN); addCiclos(7); }
void Z80:: OR_R ()  { opcode_getReg8src(); _or(*opcode_reg8src); addCiclos(4); }
void Z80:: OR_hl ()  { getDatoHL(); _or(datoIndir); addCiclos(7); }
void Z80:: OUT_n_A ()  { WORD p; pc_getByte(&p.b.h); io->OUT(p, rA); addCiclos(11); }

void Z80:: POP_RR ()  {
    opcode_getReg16_pushpop();
    _pop(opcode_reg16);
    if (opcode_reg16 == &regAF) updateFlags();
    addCiclos(10);
}
void Z80:: PUSH_RR ()  {
    opcode_getReg16_pushpop();
    if (opcode_reg16 == &regAF) updateRegF();
    _push(*opcode_reg16);
    addCiclos(15);//addCiclos(11);
}

void Z80:: xD_REGS_IND ()  {
    if (opcode == 0xDD) {
        regIdx16 = &regIX;
        regIdxH = &IXh;
        regIdxL = &IXl;
    }
    else { // 0xFD
        regIdx16 = &regIY;
        regIdxH = &IYh;
        regIdxL = &IYl;
    }
    pc_getByte(&opcode);
    R = (++R & 0x7F); //R++;
    ptrFuncion = instrucciones_indice[opcode];
    //debug_z80("ptrFuncion==null ? %d \n", ptrFuncion==nullptr);
    (this->*ptrFuncion)();
}

void Z80:: RET ()  { _pop(&regPC); addCiclos(10); }
void Z80:: RET_C ()  {
    if (opcode_getResultadoCondicionDirAbs()) {
        _pop(&regPC);
        addCiclos(15);//addCiclos(11);
    }
    else addCiclos(5);
}
void Z80:: RLA ()  {    // modifica menos flags que "RL A"
    BYTE nuevoCarry = rA & 0x80;
    rA = rA << 1;
    if (flag_carry) rA |= 0x01;
    flag_carry = nuevoCarry;
    flag_sub = false;
    flag_halfcarry = false;
    update_flags_53(rA);
    addCiclos(4); 
}
void Z80:: RLCA () {    // modifica menos flags que "RLC A"
    flag_carry = rA & 0x80;
    rA = rA << 1;
    if (flag_carry) rA |= 0x01;
    flag_sub = false;
    flag_halfcarry = false;
    update_flags_53(rA);
    addCiclos(4); 
}
void Z80:: RRA ()  {     // modifica menos flags que "RR A"
    BYTE nuevoCarry = rA & 0x01;
    rA = rA >> 1;
    if (flag_carry) rA |= 0x80;
    flag_carry = nuevoCarry;
    flag_sub = false;
    flag_halfcarry = false;
    update_flags_53(rA);
    addCiclos(4); 
}
void Z80:: RRCA () {     // modifica menos flags que "RRC A"
    flag_carry = rA & 0x01;
    rA = rA >> 1;
    if (flag_carry) rA |= 0x80;
    flag_sub = false;
    flag_halfcarry = false;
    update_flags_53(rA);
    addCiclos(4); 
}

void Z80:: RST ()  {
    _push(regPC);
    //debug_z80("Z80:: RST() PC=%04X SP=%04X \n", regPC.w, regSP.w);
    regPC.b.l = orden_rst[ (opcode & 0b00111000) >> 3 ];
    regPC.b.h = 0;
    //debug_z80("Z80:: RST() PC=%04X \n", regPC.w);
    addCiclos(15);//addCiclos(11);
}
void Z80:: SBC_N ()  { pc_getDatoN(); _sbc8(instruccion_datoN); addCiclos(7); }
void Z80:: SBC_R ()  { opcode_getReg8src(); _sbc8(*opcode_reg8src); addCiclos(4); }
void Z80:: SBC_hl () { getDatoHL(); _sbc8(datoIndir); addCiclos(7); }

void Z80:: SCF ()  {
    flag_carry = true;
    flag_halfcarry = false;
    flag_sub = false;
    addCiclos(4);
}
void Z80:: SUB_N ()  { pc_getDatoN(); _sub8(instruccion_datoN); addCiclos(7); }
void Z80:: SUB_R ()  { opcode_getReg8src(); _sub8(*opcode_reg8src); addCiclos(4); }
void Z80:: SUB_hl () { getDatoHL(); _sub8(datoIndir); addCiclos(7); }

void Z80:: XOR_N ()  { pc_getDatoN(); _xor(instruccion_datoN); addCiclos(7); }
void Z80:: XOR_R ()  { opcode_getReg8src(); _xor(*opcode_reg8src); addCiclos(4); }
void Z80:: XOR_hl () { getDatoHL(); _xor(datoIndir); addCiclos(7); }

// ----- instrucciones de bits -----

// 00bbbrrr    b=numero de bit, rrr=numero de registro
void Z80:: BIT_B_R ()  { opcode_getReg8src(); _bit(opcode_getMascaraBit(), *opcode_reg8src); addCiclos(8); }
void Z80:: BIT_B_hl () { 
    getDatoHL(); 
    _bit(opcode_getMascaraBit(), datoIndir); 
    update_flags_53(H);
    addCiclos(8); 
}
void Z80:: RES_B_R ()  { opcode_getReg8src(); _res(opcode_getMascaraBit(), opcode_reg8src); addCiclos(8); }
void Z80:: RES_B_hl () { getDatoHL(); _res(opcode_getMascaraBit(), &datoIndir); saveDatoHL(); addCiclos(8); }
void Z80:: SET_B_R ()  { opcode_getReg8src(); _set(opcode_getMascaraBit(), opcode_reg8src); addCiclos(8); }
void Z80:: SET_B_hl () { getDatoHL(); _set(opcode_getMascaraBit(), &datoIndir); saveDatoHL(); addCiclos(8); }
void Z80:: RLC_R ()    { opcode_getReg8src(); _rlc(opcode_reg8src); addCiclos(8); }
void Z80:: RLC_hl ()   { getDatoHL(); _rlc(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: RL_R ()     { opcode_getReg8src(); _rl(opcode_reg8src); addCiclos(8); }
void Z80:: RL_hl ()    { getDatoHL(); _rl(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: RRC_R ()    { opcode_getReg8src(); _rrc(opcode_reg8src); addCiclos(8); }
void Z80:: RRC_hl ()   { getDatoHL(); _rrc(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: RR_R ()     { opcode_getReg8src(); _rr(opcode_reg8src); addCiclos(8); }
void Z80:: RR_hl ()    { getDatoHL(); _rr(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: SLA_R ()    { opcode_getReg8src(); _sla(opcode_reg8src); addCiclos(8); }
void Z80:: SLA_hl ()   { getDatoHL(); _sla(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: SLL_R ()    { opcode_getReg8src(); _sll(opcode_reg8src); addCiclos(8); }
void Z80:: SLL_hl ()   { getDatoHL(); _sll(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: SRA_R ()    { opcode_getReg8src(); _sra(opcode_reg8src); addCiclos(8); }
void Z80:: SRA_hl ()   { getDatoHL(); _sra(&datoIndir); saveDatoHL(); addCiclos(15); }
void Z80:: SRL_R ()    { opcode_getReg8src(); _srl(opcode_reg8src); addCiclos(8); }
void Z80:: SRL_hl ()   { getDatoHL(); _srl(&datoIndir); saveDatoHL(); addCiclos(15); }

// ----- inst misc -----

void Z80:: ADC_HL_RR ()  {
    opcode_getReg16();
    u32 tmp = HL + opcode_reg16->w + carry();
    flag_sign = tmp & 0x8000; // el signo es el primer bit por la izquierda
    flag_zero = (tmp & 0xFFFF) == 0;
    flag_carry = tmp & 0x10000;
    flag_halfcarry = (HL ^ tmp ^ opcode_reg16->w) & 0x1000; // bit 12
    flag_sub = false;
    flag_pv = (opcode_reg16->w ^ HL ^ 0x8000) & (tmp ^ opcode_reg16->w) & 0x8000; // bit 15
    HL = static_cast<u16>(tmp);
    update_flags_53(H);
    addCiclos(15);
}

void Z80:: CPD ()  {
    getDatoHL();
    BYTE res = rA - datoIndir;
    //_cp(datoIndir);
    HL--;
    BC--;
    flag_sub = true;
    flag_halfcarry = (rA ^ datoIndir ^ res) & BIT_HALFCARRY;
    if (flag_halfcarry) --res;
    flag_bit5 = res & BIT_SUB;
    flag_bit3 = res & BIT_BIT3;
    flag_pv = (BC != 0);
    update_flag_sign(res);
    update_flag_zero(res);
    // carry igual
    addCiclos(16);
}
void Z80:: CPDR ()  {
    CPD();
    if (BC != 0 && !flag_zero) {
        addCiclos(5);//addCiclos(5);
        PC -= 2;
    }
}
void Z80:: CPI ()  {
    getDatoHL();
    BYTE res = rA - datoIndir;
    HL++;
    BC--;
    update_flag_sign(res);
    update_flag_zero(res);
    flag_sub = true;
    flag_halfcarry = (rA ^ res ^ datoIndir) & BIT_HALFCARRY;
    if (flag_halfcarry) res--;
    flag_pv = (BC != 0);
    flag_bit5 = res & BIT_SUB;
    flag_bit3 = res & BIT_BIT3;
    // carry igual
    addCiclos(16);
}
void Z80:: CPIR ()  {
    CPI();
    if (BC != 0 && !flag_zero) {
        addCiclos(5);//addCiclos(5);
        PC -= 2;
    }
}  // TODO

void Z80:: IM0 ()  { IM = 0; addCiclos(8); }
void Z80:: IM1 ()  { IM = 1; addCiclos(8); }
void Z80:: IM2 ()  { IM = 2; addCiclos(8); }

void Z80:: IND ()  {
    datoIndir = io->IN(regBC);
    saveDatoHL();
    rB--;
    HL--;
    flag_sub = true;
    update_flag_zero(rB);
    addCiclos(16);
}
void Z80:: INDR ()  {
    IND();
    if (rB != 0) {
        PC -= 2;    // si B!=0 volver a ejecutar
        addCiclos(5);
    }
}

void Z80:: INI ()  {
    datoIndir = io->IN(regBC);
    saveDatoHL();
    rB--;
    HL++;
    flag_sub = true;
    update_flag_zero(rB);
    addCiclos(16);
}
void Z80:: INIR ()  {
    INI();
    if (rB != 0) {
        PC -= 2;    // si B!=0 volver a ejecutar
        addCiclos(5);
    }
}
void Z80:: IN_c ()  {
    BYTE b = io->IN(regBC);
    flag_sub = false;
    flag_halfcarry = false;
    update_flag_sign(b);
    update_flag_zero(b);
    update_flag_parity(b);
    addCiclos(12);
}
void Z80:: IN_R_c ()  {
    opcode_getReg8dst();
    *opcode_reg8dst = io->IN(regBC);
    flag_sub = false;
    flag_halfcarry = false;
    BYTE x = *opcode_reg8dst;
    update_flag_sign(x);
    update_flag_zero(x);
    update_flag_parity(x);
    update_flags_53(x);
    addCiclos(13);//addCiclos(12);
}

void Z80:: LDD ()  {
    getDatoHL(); //memoria->readByte(HL--, &dato);
    memoria->writeByte(DE, datoIndir);
    HL--;
    DE--;
    BC--;
    flag_pv = (BC != 0);
    flag_sub = false;
    flag_halfcarry = false;
    flag_bit5 = (datoIndir + rA) & 0x02;
    flag_bit3 = (datoIndir + rA) & 0x08;
    addCiclos(16);
}
void Z80:: LDDR ()  {
    LDD();
    if (BC != 0) {
        PC -= 2;
        addCiclos(5);
    }
}

void Z80:: LDI ()  {
    getDatoHL();
    memoria->writeByte(DE, datoIndir);
    HL++;
    DE++;
    BC--;
    flag_pv = (BC != 0);
    flag_sub = false;
    flag_halfcarry = false;
    flag_bit5 = (datoIndir + rA) & 0x02;
    flag_bit3 = (datoIndir + rA) & 0x08;
    addCiclos(20);//addCiclos(16);
}
void Z80:: LDIR ()  {
    LDI();
    if (BC != 0) {
        addCiclos(4);//addCiclos(5);
        PC -= 2;
    }
}

void Z80:: LD_A_Ri ()  {
    rA = I;
    update_flag_sign(rA);
    update_flag_zero(rA);
    flag_halfcarry = false;
	flag_pv = IFF2_estadoAntIFF1;
    flag_sub = false;
	//flag_q = true;
    update_flags_53(rA);
    addCiclos(9);
}
void Z80:: LD_A_Rr ()  {
    rA = (R & 0x7F) | R7;
    update_flag_sign(rA);
    update_flag_zero(rA);
    flag_sub = false;
    flag_halfcarry = false;
	flag_pv = IFF2_estadoAntIFF1;
    // flag carry  igual
    update_flags_53(rA);
	//flag_q = true;
    addCiclos(9);
}
void Z80:: LD_RR_nn ()  {
    opcode_getReg16();
    pc_getDatoNN();
    memoria->readWord(instruccion_datoNN.w, opcode_reg16);
    addCiclos(24);//addCiclos(20);
}
void Z80:: LD_Ri_A ()  { I = rA; addCiclos(9); }
void Z80:: LD_Rr_A ()  { R = rA; R7 = rA & 0x80; addCiclos(9); }
void Z80:: LD_nn_A ()  { pc_getDatoNN();  memoria->writeByte(instruccion_datoNN.w, rA); addCiclos(13); }
void Z80:: LD_nn_RR () { 
    opcode_getReg16(); 
    pc_getDatoNN(); 
    memoria->writeWord(instruccion_datoNN.w, *opcode_reg16); 
    addCiclos(20); 
    if (opcode_reg16 != &regHL) addCiclos(4);  // !!!!!! segun la tabla no deberia sumarse
}
void Z80:: NEG () { BYTE x = rA; rA = 0; _sub8(x); }

void Z80:: OUTD ()  {
    rB--;
    getDatoHL();
    io->OUT(regBC, datoIndir);
    HL--;
    flag_sub = true;
    update_flag_zero(rB);
    addCiclos(16);
}
void Z80:: OTDR ()  {
    OUTD();
    if (rB != 0) {
        PC -= 2;
        addCiclos(5);
    }
}
void Z80:: OUTI ()  {
    rB--;
    getDatoHL();
    io->OUT(regBC, datoIndir);
    HL++;
    flag_sub = true;
    update_flag_zero(rB);
    addCiclos(16);
}
void Z80:: OTIR ()  {
    OUTI();
    if (rB != 0) {
        PC -= 2;
        addCiclos(5);
    }
}
void Z80:: OUT_c_0 ()  {
    io->OUT(regBC, (BYTE) 0);
    addCiclos(13);//addCiclos(12);
}
void Z80:: OUT_c_R ()  {
    opcode_getReg8dst();
    io->OUT(regBC, *opcode_reg8dst);
    addCiclos(13);//addCiclos(12);
}

void Z80:: RETI ()  { IFF1_interrupcionesHabilitadas = IFF2_estadoAntIFF1; _pop(&regPC); addCiclos(14);  }
void Z80:: RETN ()  { RETI(); }

void Z80:: RLD ()  {
    getDatoHL();
    BYTE prev_hl = datoIndir;
    datoIndir = (prev_hl << 4) | (rA & 0x0F);
    saveDatoHL();
    rA = (rA & 0xF0) | (prev_hl >> 4);
    
    flag_sub = false;
    flag_halfcarry = false;
    update_flag_parity(rA);
    update_flag_sign(rA);
    update_flag_zero(rA);
    addCiclos(18);
}
void Z80:: RRD ()  {
    getDatoHL();
    BYTE prev_hl = datoIndir;
    datoIndir = (datoIndir >> 4) | (rA << 4);
    saveDatoHL();
    rA = (rA & 0xF0) | (prev_hl & 0x0F);
    
    flag_sub = false;
    flag_halfcarry = false;
    update_flag_parity(rA);
    update_flag_sign(rA);
    update_flag_zero(rA);
    addCiclos(18);
}

void Z80:: SBC_HL_RR ()  {
    opcode_getReg16();
    i32 res = HL - opcode_reg16->w - carry();
    flag_carry = res & 0x10000;
    flag_halfcarry = (HL ^ res ^ opcode_reg16->w) & 0x1000; // (BIT_HALFCARRY << 8)
    flag_sign = res & 0x8000; // el signo es el primer bit por la izquierda
    flag_zero = (res & 0xffff) == 0;
    flag_sub = true;
    flag_pv = (opcode_reg16->w ^ HL) & (res ^ HL) & 0x8000; // bit 15
    update_flags_53(H);
    HL = static_cast<u16>(res);
    addCiclos(15);
}

// ----- inst de indice -----

// instrucciones principales repetidas
void Z80:: q_INC_R ()  { INC_R(); addCiclos(4); }
void Z80:: q_DEC_R ()  { DEC_R(); addCiclos(4); }
void Z80:: q_LD_R_N () { LD_R_N(); addCiclos(4); }
void Z80:: q_LD_R_R () { LD_R_R(); addCiclos(4); }
//-
void Z80:: ADC_S ()     { opcode_getReg8src2(); _adc8(*opcode_reg8src); addCiclos(8); }
void Z80:: ADD_S ()     { opcode_getReg8src2(); _add8(*opcode_reg8src); addCiclos(8); }
void Z80:: AND_S ()     { opcode_getReg8src2(); _and( *opcode_reg8src); addCiclos(8); }
void Z80:: CP_S ()      { opcode_getReg8src2(); _cp(  *opcode_reg8src); addCiclos(8); }
void Z80:: OR_S ()      { opcode_getReg8src2(); _or(  *opcode_reg8src); addCiclos(8); }
void Z80:: SBC_S ()     { opcode_getReg8src2(); _sbc8(*opcode_reg8src); addCiclos(8); }
void Z80:: SUB_S ()     { opcode_getReg8src2(); _sub8(*opcode_reg8src); addCiclos(8); }
void Z80:: XOR_S ()     { opcode_getReg8src2(); _xor( *opcode_reg8src); addCiclos(8); }
void Z80:: SBC_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _sbc8(datoIndir); addCiclos(19); }
void Z80:: ADC_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _adc8(datoIndir); addCiclos(19); }
void Z80:: ADD_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _add8(datoIndir); addCiclos(19); }
void Z80:: AND_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _and( datoIndir); addCiclos(19); }
void Z80:: CP_ii_d ()   { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _cp(  datoIndir); addCiclos(19); }
void Z80:: OR_ii_d ()   { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _or(  datoIndir); addCiclos(19); }
void Z80:: SUB_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _sub8(datoIndir); addCiclos(19); }
void Z80:: XOR_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _xor( datoIndir); addCiclos(19); }
void Z80:: ADD_II_RR () { 
    opcode_getReg16(); // reutilizamos el array
    if (opcode_reg16 == &regHL) opcode_reg16 = regIdx16;
    _add16(regIdx16, *opcode_reg16); addCiclos(15); 
}
void Z80:: DEC_Ih ()  { _dec8(regIdxH); addCiclos(8); }
void Z80:: DEC_Il ()  { _dec8(regIdxL); addCiclos(8); }
void Z80:: DEC_II ()  { regIdx16->w--; addCiclos(10); }

void Z80:: DEC_ii_d ()  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _dec8(&datoIndir); saveDatoIndir(); addCiclos(19); }

void Z80:: EX_sp_II () { 
    WORD aux;
    memoria->readWord(SP, &aux);
    intercambiarRegistros16(regIdx16, &aux);
    memoria->writeWord(SP, aux);
    addCiclos(19);
}

void Z80:: INC_Ih () { _inc8(regIdxH); addCiclos(8); }
void Z80:: INC_Il () { _inc8(regIdxL); addCiclos(8); }
void Z80:: INC_II () { regIdx16->w++; addCiclos(10); }
void Z80:: INC_ii_d () { pc_getDatoOffset(); updateIndir(); getDatoIndir(); _inc8(&datoIndir); saveDatoIndir(); addCiclos(19); }

void Z80:: xD_DD_IND_BITS ()  {
    // estas instrucciones estan codificadas como:
    // para IX: DD CB desplazamiento OPCODE
    // para IY: DD FD desplazamiento OPCODE
    // por lo que primero tenemos que obtener es el desplazamiento, y con eso ya tenemos la direccion de
    // memoria a la que apunta IX+d o IY+d, que es idxOffset
    pc_getDatoOffset(); 
    //debug_z80("offset=%d\n", instruccion_offset);
    updateIndir();
    //debug_z80("<indir=%04X>", indir);
    pc_getByte(&opcode);
    
    if (opcode < 0x40) {
        ptrFuncion = instrucciones_indice_bits[opcode];
        (this->*ptrFuncion)();
    }
    else {
        if (opcode < 0x80) {
            BIT_B_ii_d();
        }
        else {
            bool almacenarEnRegistro = (opcode & 0b00000111) != 6;
    
            if (opcode < 0xC0) {
                if (almacenarEnRegistro) RES_B_ii_d_R();
                else RES_B_ii_d();
            }
            else {
                if (almacenarEnRegistro) SET_B_ii_d_R();
                else SET_B_ii_d();
            }
        }
    }
}

void Z80:: JP_ii ()  { PC = regIdx16->w; addCiclos(2); }

void Z80:: LD_II_NN ()   { pc_getWord(regIdx16); addCiclos(14); }
void Z80:: LD_II_nn ()   { pc_getDatoNN(); memoria->readWord(instruccion_datoNN.w, regIdx16); addCiclos(20); }
void Z80:: LD_I_N ()     { opcode_getReg8dst2(); pc_getDatoN(); *opcode_reg8dst = instruccion_datoN; addCiclos(11); }
void Z80:: LD_I_S ()     { opcode_getReg8dst2(); opcode_getReg8src2(); *opcode_reg8dst = *opcode_reg8src; addCiclos(8); }
// S = conjunto de registros: a,b,c,d,e,ixh,ixl,iyh,iyl
void Z80:: LD_R_S ()     { opcode_getReg8dst(); opcode_getReg8src2(); *opcode_reg8dst = *opcode_reg8src; addCiclos(8); }

void Z80:: LD_R_ii_d (BYTE* reg8)  { pc_getDatoOffset(); updateIndir(); getDatoIndir(); *reg8 = datoIndir; addCiclos(19); }
// estos de abajo se pueden agrupar con la mascara 00111000
void Z80:: LD_A_ii_d () { LD_R_ii_d(&rA); }
void Z80:: LD_B_ii_d () { LD_R_ii_d(&rB); }
void Z80:: LD_C_ii_d () { LD_R_ii_d(&C); }
void Z80:: LD_D_ii_d () { LD_R_ii_d(&D); }
void Z80:: LD_E_ii_d () { LD_R_ii_d(&E); }
void Z80:: LD_H_ii_d () { LD_R_ii_d(&H); }
void Z80:: LD_L_ii_d () { LD_R_ii_d(&L); }

void Z80:: LD_SP_II ()   { SP = regIdx16->w; addCiclos(10); }

void Z80:: LD_ii_d_N ()  { // esta instruccion tiene dos bytes para leer: el desplazamiento y el numero
    pc_getDatoOffset();
    updateIndir();
    pc_getDatoN();
    memoria->writeByte(indir, instruccion_datoN);
    addCiclos(23); //addCiclos(19);
}
void Z80:: LD_ii_d_R ()  {
    pc_getDatoOffset();
    updateIndir();
    opcode_getReg8src();
    memoria->writeByte(indir, *opcode_reg8src);
    addCiclos(19);
}
void Z80:: LD_nn_II ()  { pc_getDatoNN(); memoria->writeWord(instruccion_datoNN.w, *regIdx16); addCiclos(24/*20*/); }

void Z80:: POP_II ()    { _pop(regIdx16); addCiclos(14); }
void Z80:: PUSH_II ()   { _push(*regIdx16); addCiclos(19); /*ciclos cambiados*/ }


// ----- instrucciones de indice-bits -----

void Z80:: BIT_B_ii_d ()  { 
    getDatoIndir(); 
    _bit(opcode_getMascaraBit(), datoIndir); 
    update_flags_53(indir >> 8);
    addCiclos(24/*20*/); 
}

void Z80:: RES_B_ii_d ()    { 
    getDatoIndir(); 
    _res(opcode_getMascaraBit(), &datoIndir); 
    saveDatoIndir();
    addCiclos(27/*23*/); 
}
void Z80:: RES_B_ii_d_R ()  { 
    RES_B_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; 
}

void Z80:: SET_B_ii_d ()    { 
    getDatoIndir(); 
    _set(opcode_getMascaraBit(), &datoIndir); 
    saveDatoIndir(); 
    addCiclos(27/*23*/); 
}
void Z80:: SET_B_ii_d_R ()  { 
    SET_B_ii_d(); 
    opcode_getReg8src(); 
    *opcode_reg8src = datoIndir; 
}

void Z80:: RLC_ii_d ()    { getDatoIndir(); _rlc(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: RLC_ii_d_R ()  { RLC_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: RL_ii_d ()    { getDatoIndir(); _rl(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: RL_ii_d_R ()  { RL_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: RRC_ii_d ()    { getDatoIndir(); _rrc(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: RRC_ii_d_R ()  { RRC_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: RR_ii_d ()    { getDatoIndir(); _rr(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: RR_ii_d_R ()  { RR_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: SLA_ii_d ()    { getDatoIndir(); _sla(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: SLA_ii_d_R ()  { SLA_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: SLL_ii_d ()    { getDatoIndir(); _sll(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: SLL_ii_d_R ()  { SLL_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: SRA_ii_d ()    { getDatoIndir(); _sra(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: SRA_ii_d_R ()  { SRA_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }

void Z80:: SRL_ii_d ()    { getDatoIndir(); _srl(&datoIndir); saveDatoIndir(); addCiclos(27/*23*/); }
void Z80:: SRL_ii_d_R ()  { SRL_ii_d(); opcode_getReg8src(); *opcode_reg8src = datoIndir; }


//-----------------------------------

/*
void Z80::strFlags(char* flags) {
    flags[0] = flag_sign        ? '1' : '0';
    flags[1] = flag_zero        ? '1' : '0';
    flags[2] = flag_bit5        ? '1' : '0';
    flags[3] = flag_halfcarry   ? '1' : '0';
    flags[4] = flag_bit3        ? '1' : '0';
    flags[5] = flag_parity      ? '1' : '0';
    flags[6] = flag_sub         ? '1' : '0';
    flags[7] = flag_carry       ? '1' : '0';
}
*/

void Z80::printEstado () {
    updateRegF();
	WORD pila;
	memoria->readWord(SP, &pila);

    debug("[%ld %04X %04X %04X %04X %04X %04X %04X %04X [%04X] %02X %02X i:%d,%d,%d]",
        m_cycles, AF, BC, DE, HL, IX, IY, PC, SP, pila, I, R,
        IFF1_interrupcionesHabilitadas, interrupcionSolicitada, ei_ejecutado );
}

