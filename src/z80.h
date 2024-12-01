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
http://www.z80.info/index.htm
https://worldofspectrum.org/z88forever/dn327/z80undoc.htm
**/

#pragma once

#include <string>
#include <stack>
#include "tipos.h"
#include "memory.h"
#include "io.h"
#include "io_device.h"
#include "sna.h"
#include "compilation_options.h"


constexpr BYTE BIT_SIGN      = 0b10000000;
constexpr BYTE BIT_ZERO      = 0b01000000;
constexpr BYTE BIT_BIT5      = 0b00100000;
constexpr BYTE BIT_HALFCARRY = 0b00010000;
constexpr BYTE BIT_BIT3      = 0b00001000;
constexpr BYTE BIT_PV        = 0b00000100;
constexpr BYTE BIT_PARITY    = 0b00000100;
constexpr BYTE BIT_OVERFLOW  = 0b00000100;
constexpr BYTE BIT_SUB       = 0b00000010;
constexpr BYTE BIT_CARRY     = 0b00000001;


#define AF regAF.w
#define BC regBC.w
#define DE regDE.w
#define HL regHL.w
#define IX regIX.w
#define IY regIY.w
#define SP regSP.w
#define PC regPC.w
//#define A regAF.b.h
#define rA regAF.b.h
#define F regAF.b.l
#define rB regBC.b.h
#define C regBC.b.l
#define D regDE.b.h
#define E regDE.b.l
#define H regHL.b.h
#define L regHL.b.l
#define IXh regIX.b.h
#define IXl regIX.b.l
#define IYh regIY.b.h
#define IYl regIY.b.l

#define AF2 regAF2.w
#define BC2 regBC2.w
#define DE2 regDE2.w
#define HL2 regHL2.w

#define flag_parity flag_pv
#define flag_overflow flag_pv

#define getNumeroBit getRegistro8dstOpcode
#define getCondicionOpcode getRegistro8dst
#define getNumReinicio getRegistro8dstOpcode


class IO_Bus;


class Z80
{
    Memory* memoria;
    IO_Bus* io;


    // registros y variables de estado de la cpu----------
    WORD regAF, regBC, regDE, regHL;
    BYTE R, I;					// registros I R, Rb7 es ek bit 7, I vector de interrupciones
    bool IFF1_interrupcionesHabilitadas, IFF2_estadoAntIFF1;
    WORD regIX, regIY;
    WORD regSP, regPC;
    bool IM = false;
    WORD regAF2, regBC2, regDE2, regHL2;
	//----------------------------------------------------

	BYTE R7;
	// flags
    bool flag_sign, flag_zero, flag_halfcarry, flag_pv, flag_sub, flag_carry;
	bool flag_bit5, flag_bit3;
	//bool flag_q;
    bool haltActivo = false;
	bool interrupcionSolicitada; 
	IAttendedInterruptReceiver* receptorInterrupcionAtendida = nullptr;
#ifdef debug_ints_borde
	IDisplayInterrupt* displayInt = nullptr;
#endif
	WORD pcInt;
	u8 ei_ejecutado; // minicontador, ya que la interrupcion se ejecutaria despues de la siguiente instruccion
    u64 m_cycles;
	u8 ciclosInstr;

    BYTE  opcode;
    WORD* opcode_reg16; // el registro RR codificado en opcode
    BYTE* opcode_reg8dst; // el registro R destino codificado en opcode
    BYTE* opcode_reg8src; // el registro R fuente codificado en opcode
    SBYTE instruccion_offset; // [+d] de las instrucciones JR y IX+d
    BYTE  instruccion_datoN; // el dato de 8 bits de la instruccion
    WORD  instruccion_datoNN; // el dato de 16 bits de la instruccion
    BYTE  datoIndir; // el dato apuntado por HL, IX+d y IY+d
    // punteros que haran referencia a alguna de las variables de arriba
    WORD* regIdx16; // IX o IY
    BYTE* regIdxH;  // ixh o iyh
    BYTE* regIdxL;  // ixl o iyl
    BYTE* regIdx8; // ixh ixl iyh iyl
    u16   indir; // sera IX+d o IY+d

    bool* orden_flags[4] = { &flag_zero, &flag_carry, &flag_pv, &flag_sign }; // para evaluar el cumplimiento de la condicion
	BYTE* orden_registros8[8] = { &rB, &C, &D, &E, &H, &L, nullptr, &rA };  // registros principales de 8 bits
    WORD* orden_registros16[4] = { &regBC, &regDE, &regHL, &regSP };   // registros principales de 16 bits
    //WORD* orden_registros16_2[4] = { &regBC, &regDE, regIdx16, &regSP };   // registros principales de 16 bits
    WORD* orden_registros16_pushpop[4] = { &regBC, &regDE, &regHL, &regAF };   // registros principales de 16 bits
	//BYTE* orden_registros82[8] = { &rB, &C, &D, &E, regIdxH, regIdxL, nullptr, &rA };   // b,c,d,e,Xh,Xl,-,a
    static constexpr BYTE orden_rst[8] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 };
    static constexpr BYTE mascaraBit[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    //---------------------------------------

public:

    Z80(Memory* mem, IO_Bus* io);
//    ~Z80();
    
    void reset();
	u64 getCiclos(); // get_m_cycles
	u8 getInstructionCycles(); // obtiene los ciclos utilizados por la ultima instruccion ejecutada
	void reset_m_cycles();
    void executeNextInstruction ();
    void requestInterrupt();
	void setAttendedInterruptReceiver(IAttendedInterruptReceiver* ia) { receptorInterrupcionAtendida = ia; }
#ifdef debug_ints_borde
	void setDisplayInt(IDisplayInterrupt* d) { displayInt = d; }
#endif

	void setSnaData(SNA_Z80* sna, u8 version);
	void getSnaData(SNA_Z80* sna);

	// para pruebas:
	//u16  getPC() { return PC; }
	//WORD getAF() { return regAF; }
	//WORD getBC() { return regBC; }
	//WORD getDE() { return regDE; }
	//WORD getHL() { return regHL; }
    //void set_PC_SP(u16 _pc, u16 _sp) { PC = _pc; SP = _sp;}
    //void set_PC(u16 _pc) { PC = _pc; }

	std::string& getInstruccionASM();
    void printEstado ();
	void printBytesInstruccion();
	void printInstruccionASM();
	void printInstruccion();

#ifdef instr_asm
	std::string asm_opcodes;
	std::string asm_instruccion;
	u16 lastPC = 0xFFFF;

	void decodeInstructionAtPC();
	std::string& getInstructionOpcodes();
	std::string& getInstructionAsm();
	void printInstructionBytes();
	void printInstructionASM();
#endif


private:

    // para invocar las funciones
    void (Z80::*ptrFuncion)();
    
    void addCiclos(u8 n);
	void atenderInterrupcion();
    
    // funciones de los flags
    BYTE carry(); // devuelve 0 o 1 segun flag_carry
    //void strFlags(char* flags);
    void updateRegF();	// actualiza el valor del registro F a partir de los flags
    void updateFlags();	// actualiza los flags a partir del registro F
	void update_flags_53(BYTE v);
    void update_flag_sign(BYTE v);
    void update_flag_zero(BYTE v);
    void update_flag_parity(BYTE v);
    void updateFlagsRotaciones(BYTE n);

    void pc_getByte(BYTE * byte);	// byte <- (pc) ; pc++
    void pc_getWord(WORD * word);	// word <- (pc) ; pc<-pc+2
    void pc_getDatoN(); // instruccion_datoN <- (pc) ; pc++
    void pc_getDatoNN(); // instruccion_datoNN <- (pc) ; pc<-pc+2
    void pc_getDatoOffset();	// instruccion_datoOffset <- (pc) ; pc++
	void updateIndir();

    void getDatoHL(); // guarda en la varible datoHL el valor de (hl)
    void saveDatoHL();  // (hl) <- datoIndir
    void getDatoIndir(); // guarda en la varible datoHL el valor de (ix+d) (iy+d)
    void saveDatoIndir();  // (ix+d) <- datoIndir
    
    // del byteInstruccion actual
    void opcode_getReg8dst();  // el registro destino se codifica en oo111ooo en los opcodes, tambien el nº de bit en las operaciones de bit, b c d e h l nullptr a
    void opcode_getReg8src();  // el registro fuente se  codifica en ooooo111 en los opcodes, b c d e h l nullptr a
    void opcode_getReg16();    // el registro doble se codifica en oo11oooo en los opcodes, devuelve referencia a BC DE HL SP
    void opcode_getReg16_2();    // el registro doble se codifica en oo11oooo en los opcodes, devuelve referencia a BC DE II SP
    void opcode_getReg16_pushpop();    // el registro doble se codifica en oo11oooo en los opcodes, bc de hl
    void opcode_getReg8src2();  // b,c,d,e,Xh,Xl,nullptr,a
    void opcode_getReg8dst2();  // Xh|Xl
    BYTE opcode_getMascaraBit();   // obtiene la mascara del numero de bit
    BYTE opcode_getResultadoCondicionDirAbs();  // resultado de la condicion para JP/CALL
    

    void _inc8(BYTE *r8);
    void _dec8(BYTE *r8);
    void _push(WORD reg);
    void _pop(WORD* reg);
    void _and(BYTE n);
    void _cp(BYTE n);
    void _or(BYTE n);
    void _xor(BYTE n);
    void _add8(BYTE n);
    void _adc8(BYTE n);
    void _sub8(BYTE n);
    void _sbc8(BYTE n);
    void _add16(WORD* dst, WORD n); // dst puede ser hl,ix o iy
    void _rlc(BYTE n);
    void _rl(BYTE n);
    void _sla(BYTE n);
    void _sll(BYTE n);
    void _rrc(BYTE n);
    void _rr(BYTE n);
    void _sra(BYTE n);
    void _srl(BYTE n);

    void _bit(BYTE mascara, BYTE n);
    void _res(BYTE mascara, BYTE* n);
    void _set(BYTE mascara, BYTE* n);
    void _rlc(BYTE* n);
    void _rl(BYTE* n);
    void _rrc(BYTE* n);
    void _rr(BYTE* n);
    void _sla(BYTE* n);
    void _sll(BYTE* n);
    void _sra(BYTE* n);
    void _srl(BYTE* n);
    
	// ----- inst 1 byte -----

	void ADC_N ();
	void ADC_R ();
	void ADC_hl ();
	void ADD_HL_RR ();
	void ADD_N ();
	void ADD_R ();
	void ADD_hl ();
	void AND_N ();
	void AND_R ();
	void AND_hl ();
	void BITS ();
	void CALL_C_NN ();
	void CALL_NN ();
	void CCF ();
	void CPL ();
	void CP_N ();
	void CP_R ();
	void CP_hl ();
	void DAA ();
	void DEC_hl();
	void DEC_R ();
	void DEC_RR ();
	void DI ();
	void DJNZ_D ();
	void EI ();
	void EXX ();
	void EX_AF_AF ();
	void EX_DE_HL ();
	void EX_sp_HL ();
	void HALT ();
	void INC_hl();
	void INC_R ();
	void INC_RR ();
	void IN_A_n ();
	void JP_C_NN ();
	void JP_NN ();
	void JP_hl ();
	void JR_C_D ();
	void JR_D ();
	void LD_R_hl ();
	void LD_A_nn ();
	void LD_A_rr ();
	void LD_HL_nn ();
	void LD_RR_NN ();
	void LD_R_N ();
	void LD_R_R ();
	void LD_SP_HL ();
	void LD_hl_R ();
	void LD_hl_N();
	void LD_nn_A();
	void LD_nn_HL ();
	void LD_rr_A ();
	void MISC ();
	void NOP ();
	void OR_N ();
	void OR_R ();
	void OR_hl ();
	void OUT_n_A ();
	void POP_RR ();
	void PUSH_RR ();
	void xD_REGS_IND ();
	void RET ();
	void RET_C ();
	void RLA ();
	void RLCA ();
	void RRA ();
	void RRCA ();
	void RST ();
	void SBC_N ();
	void SBC_R ();
	void SBC_hl ();
	void SCF ();
	void SUB_N ();
	void SUB_R ();
	void SUB_hl ();
	void XOR_N ();
	void XOR_R ();
	void XOR_hl ();

	// ----- inst bits -----

	void BIT_B_R ();
	void BIT_B_hl ();
	void RES_B_R ();
	void RES_B_hl ();
	void RLC_R ();
	void RLC_hl ();
	void RL_R ();
	void RL_hl ();
	void RRC_R ();
	void RRC_hl ();
	void RR_R ();
	void RR_hl ();
	void SET_B_R ();
	void SET_B_hl ();
	void SLA_R ();
	void SLA_hl ();
	void SLL_R ();
	void SLL_hl ();
	void SRA_R ();
	void SRA_hl ();
	void SRL_R ();
	void SRL_hl ();

	// ----- inst misc -----

	void ADC_HL_RR ();
	void CPD ();
	void CPDR ();
	void CPI ();
	void CPIR ();
	void IM0 ();
	void IM1 ();
	void IM2 ();
	void IND ();
	void INDR ();
	void INI ();
	void INIR ();
	void IN_c ();
	void IN_R_c ();
	void LDD ();
	void LDDR ();
	void LDI ();
	void LDIR ();
	void LD_A_Ri ();
	void LD_A_Rr ();
	void LD_RR_nn ();
	void LD_Ri_A ();
	void LD_Rr_A ();
	void LD_nn_RR ();
	void NEG ();
	void OUTD ();
	void OUTI ();
	void OTDR ();
	void OTIR ();
	void OUT_c_0 ();
	void OUT_c_R ();
	void RETI ();
	void RETN ();
	void RLD ();
	void RRD ();
	void SBC_HL_RR ();

	// ----- inst de registro -----
	
	void q_INC_R ();
	void q_DEC_R ();
	void q_LD_R_N ();
	void q_LD_R_R ();

	void ADC_S ();
	void ADC_ii_d ();
	void ADD_S ();
	void ADD_II_RR ();
	void ADD_ii_d ();
	void AND_S ();
	void AND_ii_d ();
	void CP_S ();
	void CP_ii_d ();
	void DEC_Ih ();
	void DEC_Il ();
	void DEC_II ();
	void DEC_ii_d ();
	void EX_sp_II ();
	void INC_Ih ();
	void INC_Il ();
	void INC_II ();
	void INC_ii_d ();
	void xD_DD_IND_BITS ();
	void JP_ii ();
	void LD_II_NN ();
	void LD_II_nn ();
	void LD_I_I ();
	void LD_I_N ();
	void LD_I_S ();
	void LD_R_S ();

	void LD_R_ii_d (BYTE* reg8);
	void LD_A_ii_d ();
	void LD_B_ii_d ();
	void LD_C_ii_d ();
	void LD_D_ii_d ();
	void LD_E_ii_d ();
	void LD_H_ii_d ();
	void LD_L_ii_d ();

	void LD_SP_II ();
	void LD_ii_d_N ();
	void LD_ii_d_R ();
	void LD_nn_II ();
	void OR_S ();
	void OR_ii_d ();
	void POP_II ();
	void PUSH_II ();
	void SBC_S ();
	void SBC_ii_d ();
	void SUB_S ();
	void SUB_ii_d ();
	void XOR_S ();
	void XOR_ii_d ();

	// ----- inst de registro-bits -----

	void BIT_B_ii_d ();
	void RES_B_ii_d ();
	void RES_B_ii_d_R ();
	void RLC_ii_d ();
	void RLC_ii_d_R ();
	void RL_ii_d ();
	void RL_ii_d_R ();
	void RRC_ii_d ();
	void RRC_ii_d_R ();
	void RR_ii_d ();
	void RR_ii_d_R ();
	void SET_B_ii_d ();
	void SET_B_ii_d_R ();
	void SLA_ii_d ();
	void SLA_ii_d_R ();
	void SLL_ii_d ();
	void SLL_ii_d_R ();
	void SRA_ii_d ();
	void SRA_ii_d_R ();
	void SRL_ii_d ();
	void SRL_ii_d_R ();

    //---------------------------------------

	// static constexpr --> como sifuera const static
    static constexpr void (Z80::*instrucciones_principales[256])() = {	
    	&Z80::NOP,     &Z80::LD_RR_NN, &Z80::LD_rr_A,  &Z80::INC_RR,   &Z80::INC_R,     &Z80::DEC_R,   &Z80::LD_R_N,  &Z80::RLCA,    &Z80::EX_AF_AF, &Z80::ADD_HL_RR, &Z80::LD_A_rr,  &Z80::DEC_RR,   &Z80::INC_R,     &Z80::DEC_R,       &Z80::LD_R_N,  &Z80::RRCA, 
    	&Z80::DJNZ_D,  &Z80::LD_RR_NN, &Z80::LD_rr_A,  &Z80::INC_RR,   &Z80::INC_R,     &Z80::DEC_R,   &Z80::LD_R_N,  &Z80::RLA,     &Z80::JR_D,     &Z80::ADD_HL_RR, &Z80::LD_A_rr,  &Z80::DEC_RR,   &Z80::INC_R,     &Z80::DEC_R,       &Z80::LD_R_N,  &Z80::RRA, 
    	&Z80::JR_C_D,  &Z80::LD_RR_NN, &Z80::LD_nn_HL, &Z80::INC_RR,   &Z80::INC_R,     &Z80::DEC_R,   &Z80::LD_R_N,  &Z80::DAA,     &Z80::JR_C_D,   &Z80::ADD_HL_RR, &Z80::LD_HL_nn, &Z80::DEC_RR,   &Z80::INC_R,     &Z80::DEC_R,       &Z80::LD_R_N,  &Z80::CPL, 
    	&Z80::JR_C_D,  &Z80::LD_RR_NN, &Z80::LD_nn_A,  &Z80::INC_RR,   &Z80::INC_hl,    &Z80::DEC_hl,  &Z80::LD_hl_N, &Z80::SCF,     &Z80::JR_C_D,   &Z80::ADD_HL_RR, &Z80::LD_A_nn,  &Z80::DEC_RR,   &Z80::INC_R,     &Z80::DEC_R,       &Z80::LD_R_N,  &Z80::CCF, 
    	&Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,  &Z80::LD_R_hl, &Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,      &Z80::LD_R_hl, &Z80::LD_R_R, 
    	&Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,  &Z80::LD_R_hl, &Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,      &Z80::LD_R_hl, &Z80::LD_R_R, 
    	&Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,  &Z80::LD_R_hl, &Z80::LD_R_R,  &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,      &Z80::LD_R_hl, &Z80::LD_R_R, 
    	&Z80::LD_hl_R, &Z80::LD_hl_R,  &Z80::LD_hl_R,  &Z80::LD_hl_R,  &Z80::LD_hl_R,   &Z80::LD_hl_R, &Z80::HALT,    &Z80::LD_hl_R, &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,   &Z80::LD_R_R,   &Z80::LD_R_R,    &Z80::LD_R_R,      &Z80::LD_R_hl, &Z80::LD_R_R, 
    	&Z80::ADD_R,   &Z80::ADD_R,    &Z80::ADD_R,    &Z80::ADD_R,    &Z80::ADD_R,     &Z80::ADD_R,   &Z80::ADD_hl,  &Z80::ADD_R,   &Z80::ADC_R,    &Z80::ADC_R,     &Z80::ADC_R,    &Z80::ADC_R,    &Z80::ADC_R,     &Z80::ADC_R,       &Z80::ADC_hl,  &Z80::ADC_R, 
    	&Z80::SUB_R,   &Z80::SUB_R,    &Z80::SUB_R,    &Z80::SUB_R,    &Z80::SUB_R,     &Z80::SUB_R,   &Z80::SUB_hl,  &Z80::SUB_R,   &Z80::SBC_R,    &Z80::SBC_R,     &Z80::SBC_R,    &Z80::SBC_R,    &Z80::SBC_R,     &Z80::SBC_R,       &Z80::SBC_hl,  &Z80::SBC_R, 
    	&Z80::AND_R,   &Z80::AND_R,    &Z80::AND_R,    &Z80::AND_R,    &Z80::AND_R,     &Z80::AND_R,   &Z80::AND_hl,  &Z80::AND_R,   &Z80::XOR_R,    &Z80::XOR_R,     &Z80::XOR_R,    &Z80::XOR_R,    &Z80::XOR_R,     &Z80::XOR_R,       &Z80::XOR_hl,  &Z80::XOR_R, 
    	&Z80::OR_R,    &Z80::OR_R,     &Z80::OR_R,     &Z80::OR_R,     &Z80::OR_R,      &Z80::OR_R,    &Z80::OR_hl,   &Z80::OR_R,    &Z80::CP_R,     &Z80::CP_R,      &Z80::CP_R,     &Z80::CP_R,     &Z80::CP_R,      &Z80::CP_R,        &Z80::CP_hl,   &Z80::CP_R, 
    	&Z80::RET_C,   &Z80::POP_RR,   &Z80::JP_C_NN,  &Z80::JP_NN,    &Z80::CALL_C_NN, &Z80::PUSH_RR, &Z80::ADD_N,   &Z80::RST,     &Z80::RET_C,    &Z80::RET,       &Z80::JP_C_NN,  &Z80::BITS,     &Z80::CALL_C_NN, &Z80::CALL_NN,     &Z80::ADC_N,   &Z80::RST, 
    	&Z80::RET_C,   &Z80::POP_RR,   &Z80::JP_C_NN,  &Z80::OUT_n_A,  &Z80::CALL_C_NN, &Z80::PUSH_RR, &Z80::SUB_N,   &Z80::RST,     &Z80::RET_C,    &Z80::EXX,       &Z80::JP_C_NN,  &Z80::IN_A_n,   &Z80::CALL_C_NN, &Z80::xD_REGS_IND, &Z80::SBC_N,   &Z80::RST, 
    	&Z80::RET_C,   &Z80::POP_RR,   &Z80::JP_C_NN,  &Z80::EX_sp_HL, &Z80::CALL_C_NN, &Z80::PUSH_RR, &Z80::AND_N,   &Z80::RST,     &Z80::RET_C,    &Z80::JP_hl,     &Z80::JP_C_NN,  &Z80::EX_DE_HL, &Z80::CALL_C_NN, &Z80::MISC,        &Z80::XOR_N,   &Z80::RST, 
    	&Z80::RET_C,   &Z80::POP_RR,   &Z80::JP_C_NN,  &Z80::DI,       &Z80::CALL_C_NN, &Z80::PUSH_RR, &Z80::OR_N,    &Z80::RST,     &Z80::RET_C,    &Z80::LD_SP_HL,  &Z80::JP_C_NN,  &Z80::EI,       &Z80::CALL_C_NN, &Z80::xD_REGS_IND, &Z80::CP_N,    &Z80::RST, 
    };	
    
    static constexpr void (Z80::*instrucciones_bits[64])() = {	
    	&Z80::RLC_R, &Z80::RLC_R, &Z80::RLC_R, &Z80::RLC_R, &Z80::RLC_R, &Z80::RLC_R, &Z80::RLC_hl, &Z80::RLC_R, &Z80::RRC_R, &Z80::RRC_R, &Z80::RRC_R, &Z80::RRC_R, &Z80::RRC_R, &Z80::RRC_R, &Z80::RRC_hl, &Z80::RRC_R, 
    	&Z80::RL_R,  &Z80::RL_R,  &Z80::RL_R,  &Z80::RL_R,  &Z80::RL_R,  &Z80::RL_R,  &Z80::RL_hl,  &Z80::RL_R,  &Z80::RR_R,  &Z80::RR_R,  &Z80::RR_R,  &Z80::RR_R,  &Z80::RR_R,  &Z80::RR_R,  &Z80::RR_hl,  &Z80::RR_R, 
    	&Z80::SLA_R, &Z80::SLA_R, &Z80::SLA_R, &Z80::SLA_R, &Z80::SLA_R, &Z80::SLA_R, &Z80::SLA_hl, &Z80::SLA_R, &Z80::SRA_R, &Z80::SRA_R, &Z80::SRA_R, &Z80::SRA_R, &Z80::SRA_R, &Z80::SRA_R, &Z80::SRA_hl, &Z80::SRA_R, 
    	&Z80::SLL_R, &Z80::SLL_R, &Z80::SLL_R, &Z80::SLL_R, &Z80::SLL_R, &Z80::SLL_R, &Z80::SLL_hl, &Z80::SLL_R, &Z80::SRL_R, &Z80::SRL_R, &Z80::SRL_R, &Z80::SRL_R, &Z80::SRL_R, &Z80::SRL_R, &Z80::SRL_hl, &Z80::SRL_R, 
    };	
    
    static constexpr void (Z80::*instrucciones_misc[128])() = {	
    	&Z80::IN_R_c, &Z80::OUT_c_R, &Z80::SBC_HL_RR, &Z80::LD_nn_RR, &Z80::NEG, &Z80::RETN, &Z80::IM0, &Z80::LD_Ri_A, &Z80::IN_R_c, &Z80::OUT_c_R, &Z80::ADC_HL_RR, &Z80::LD_RR_nn, &Z80::NEG, &Z80::RETI, nullptr,   &Z80::LD_Rr_A, 
    	&Z80::IN_R_c, &Z80::OUT_c_R, &Z80::SBC_HL_RR, &Z80::LD_nn_RR, &Z80::NEG, &Z80::RETN, &Z80::IM1, &Z80::LD_A_Ri, &Z80::IN_R_c, &Z80::OUT_c_R, &Z80::ADC_HL_RR, &Z80::LD_RR_nn, &Z80::NEG, &Z80::RETI, &Z80::IM2, &Z80::LD_A_Rr, 
    	&Z80::IN_R_c, &Z80::OUT_c_R, &Z80::SBC_HL_RR, &Z80::LD_nn_RR, &Z80::NEG, &Z80::RETN, nullptr,   &Z80::RRD,     &Z80::IN_R_c, &Z80::OUT_c_R, &Z80::ADC_HL_RR, &Z80::LD_RR_nn, &Z80::NEG, &Z80::RETI, nullptr,   &Z80::RLD, 
    	&Z80::IN_c,   &Z80::OUT_c_0, &Z80::SBC_HL_RR, &Z80::LD_nn_RR, &Z80::NEG, &Z80::RETN, nullptr,   nullptr,       &Z80::IN_R_c, &Z80::OUT_c_R, &Z80::ADC_HL_RR, &Z80::LD_RR_nn, &Z80::NEG, &Z80::RETI, nullptr,   nullptr, 
    	nullptr,      nullptr,       nullptr,         nullptr,        nullptr,   nullptr,    nullptr,   nullptr,       nullptr,      nullptr,       nullptr,         nullptr,        nullptr,   nullptr,    nullptr,   nullptr, 
    	nullptr,      nullptr,       nullptr,         nullptr,        nullptr,   nullptr,    nullptr,   nullptr,       nullptr,      nullptr,       nullptr,         nullptr,        nullptr,   nullptr,    nullptr,   nullptr, 
    	&Z80::LDI,    &Z80::CPI,     &Z80::INI,       &Z80::OUTI,     nullptr,   nullptr,    nullptr,   nullptr,       &Z80::LDD,    &Z80::CPD,     &Z80::IND,       &Z80::OUTD,     nullptr,   nullptr,    nullptr,   nullptr, 
    	&Z80::LDIR,   &Z80::CPIR,    &Z80::INIR,      &Z80::OTIR,     nullptr,   nullptr,    nullptr,   nullptr,       &Z80::LDDR,   &Z80::CPDR,    &Z80::INDR,      &Z80::OTDR,     nullptr,   nullptr,    nullptr,   nullptr, 
    };	
    
    static constexpr void (Z80::*instrucciones_indice[256])() = {	
		nullptr,        nullptr,        nullptr,        nullptr,        &Z80::q_INC_R,  &Z80::q_DEC_R,  &Z80::q_LD_R_N,     nullptr,        nullptr,        &Z80::ADD_II_RR,nullptr,        nullptr,        &Z80::q_INC_R,  &Z80::q_DEC_R, 	&Z80::q_LD_R_N,     nullptr, 
		nullptr,        nullptr,        nullptr,        nullptr,        &Z80::q_INC_R,  &Z80::q_DEC_R,  &Z80::q_LD_R_N,     nullptr,        nullptr,        &Z80::ADD_II_RR,nullptr,        nullptr,        &Z80::q_INC_R,  &Z80::q_DEC_R, 	&Z80::q_LD_R_N,     nullptr, 
		nullptr,        &Z80::LD_II_NN, &Z80::LD_nn_II, 	&Z80::INC_II, 	&Z80::INC_Ih, 	&Z80::DEC_Ih, 	&Z80::LD_I_N,   nullptr,        nullptr,        &Z80::ADD_II_RR,&Z80::LD_II_nn, &Z80::DEC_II, 	&Z80::INC_Il,   &Z80::DEC_Il,   &Z80::LD_I_N,       nullptr, 
		nullptr,        nullptr, 		nullptr, 	nullptr, 	&Z80::INC_ii_d, 	&Z80::DEC_ii_d, 	&Z80::LD_ii_d_N,    nullptr,        nullptr,        &Z80::ADD_II_RR,nullptr,        nullptr,        &Z80::q_INC_R,  &Z80::q_DEC_R, 	&Z80::q_LD_R_N, 	nullptr, 
		&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_B_ii_d, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_C_ii_d,    &Z80::LD_R_S, 
		&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_D_ii_d, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_E_ii_d,    &Z80::LD_R_S, 
		&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_H_ii_d, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_I_S, 	&Z80::LD_L_ii_d,    &Z80::LD_I_S, 
		&Z80::LD_ii_d_R,&Z80::LD_ii_d_R,&Z80::LD_ii_d_R,&Z80::LD_ii_d_R,&Z80::LD_ii_d_R,&Z80::LD_ii_d_R,nullptr, 			&Z80::LD_ii_d_R,&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_R_S, 	&Z80::LD_A_ii_d,    &Z80::LD_R_S, 
		&Z80::ADD_S, 	&Z80::ADD_S, 	&Z80::ADD_S, 	&Z80::ADD_S, 	&Z80::ADD_S, 	&Z80::ADD_S, 	&Z80::ADD_ii_d, 	&Z80::ADD_S, 	&Z80::ADC_S, 	&Z80::ADC_S, 	&Z80::ADC_S, 	&Z80::ADC_S, 	&Z80::ADC_S, 	&Z80::ADC_S, 	&Z80::ADC_ii_d,     &Z80::ADC_S, 
		&Z80::SUB_S, 	&Z80::SUB_S, 	&Z80::SUB_S, 	&Z80::SUB_S, 	&Z80::SUB_S, 	&Z80::SUB_S, 	&Z80::SUB_ii_d, 	&Z80::SUB_S, 	&Z80::SBC_S, 	&Z80::SBC_S, 	&Z80::SBC_S, 	&Z80::SBC_S, 	&Z80::SBC_S, 	&Z80::SBC_S, 	&Z80::SBC_ii_d,     &Z80::SBC_S, 
		&Z80::AND_S, 	&Z80::AND_S, 	&Z80::AND_S, 	&Z80::AND_S, 	&Z80::AND_S, 	&Z80::AND_S, 	&Z80::AND_ii_d, 	&Z80::AND_S, 	&Z80::XOR_S, 	&Z80::XOR_S, 	&Z80::XOR_S, 	&Z80::XOR_S, 	&Z80::XOR_S, 	&Z80::XOR_S, 	&Z80::XOR_ii_d,     &Z80::XOR_S, 
		&Z80::OR_S, 	&Z80::OR_S, 	&Z80::OR_S, 	&Z80::OR_S, 	&Z80::OR_S, 	&Z80::OR_S, 	&Z80::OR_ii_d, 		&Z80::OR_S, 	&Z80::CP_S, 	&Z80::CP_S, 	&Z80::CP_S, 	&Z80::CP_S, 	&Z80::CP_S, 	&Z80::CP_S, 	&Z80::CP_ii_d, 	    &Z80::CP_S, 
		nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,            nullptr,        nullptr,        nullptr,        nullptr,        &Z80::xD_DD_IND_BITS, nullptr,  nullptr,        nullptr,            nullptr, 
		nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,            nullptr,        nullptr,        nullptr,        nullptr,        nullptr, 	    nullptr,        nullptr,        nullptr,            nullptr, 
		nullptr,        &Z80::POP_II,   nullptr,        &Z80::EX_sp_II, 	nullptr, 	&Z80::PUSH_II, 	nullptr,            nullptr,        nullptr,        &Z80::JP_ii,    nullptr,        nullptr,        nullptr,        nullptr,        nullptr,            nullptr, 
		nullptr,        nullptr,        nullptr,        nullptr, 	nullptr, 	nullptr, 	nullptr, 	nullptr,            nullptr,        &Z80::LD_SP_II, nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr
    };	
    
    static constexpr void (Z80::*instrucciones_indice_bits[64])() = {	
    	&Z80::RLC_ii_d_R, &Z80::RLC_ii_d_R, &Z80::RLC_ii_d_R, &Z80::RLC_ii_d_R, &Z80::RLC_ii_d_R, &Z80::RLC_ii_d_R, &Z80::RLC_ii_d, &Z80::RLC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d_R, &Z80::RRC_ii_d, &Z80::RRC_ii_d_R, 
    	&Z80::RL_ii_d_R,  &Z80::RL_ii_d_R,  &Z80::RL_ii_d_R,  &Z80::RL_ii_d_R,  &Z80::RL_ii_d_R,  &Z80::RL_ii_d_R,  &Z80::RL_ii_d,  &Z80::RL_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d_R,  &Z80::RR_ii_d,  &Z80::RR_ii_d_R, 
    	&Z80::SLA_ii_d_R, &Z80::SLA_ii_d_R, &Z80::SLA_ii_d_R, &Z80::SLA_ii_d_R, &Z80::SLA_ii_d_R, &Z80::SLA_ii_d_R, &Z80::SLA_ii_d, &Z80::SLA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d_R, &Z80::SRA_ii_d, &Z80::SRA_ii_d_R, 
    	&Z80::SLL_ii_d_R, &Z80::SLL_ii_d_R, &Z80::SLL_ii_d_R, &Z80::SLL_ii_d_R, &Z80::SLL_ii_d_R, &Z80::SLL_ii_d_R, &Z80::SLL_ii_d, &Z80::SLL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d_R, &Z80::SRL_ii_d, &Z80::SRL_ii_d_R, 
    };	

};
