/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Decode of instruction at program counter (PC)                             |
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


#include <stdio.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "tipos.h"
#include "memory.h"
#include "z80.h"
#include "compilation_options.h"


#ifdef instr_asm
//-----------------------------------------------------------------

const std::unordered_map<u8, std::string> hmInstrucciones = {
{0x00, "nop"},	{0x01, "ld bc,nn"},	{0x02, "ld (bc),a"},	{0x03, "inc bc"},	{0x04, "inc b"},	{0x05, "dec b"},	{0x06, "ld b,n"},	{0x07, "rlca"},	{0x08, "ex af,af'"},	{0x09, "add hl,bc"},	{0x0A, "ld a,(bc)"},	{0x0B, "dec bc"},	{0x0C, "inc c"},	{0x0D, "dec c"},	{0x0E, "ld c,n"},	{0x0F, "rrca"},
{0x10, "djnz o"},	{0x11, "ld de,nn"},	{0x12, "ld (de),a"},	{0x13, "inc de"},	{0x14, "inc d"},	{0x15, "dec d"},	{0x16, "ld d,n"},	{0x17, "rla"},	{0x18, "jr o"},	{0x19, "add hl,de"},	{0x1A, "ld a,(de)"},	{0x1B, "dec de"},	{0x1C, "inc e"},	{0x1D, "dec e"},	{0x1E, "ld e,n"},	{0x1F, "rra"},
{0x20, "jr nz,o"},	{0x21, "ld hl,nn"},	{0x22, "ld (nn),hl"},	{0x23, "inc hl"},	{0x24, "inc h"},	{0x25, "dec h"},	{0x26, "ld h,n"},	{0x27, "daa"},	{0x28, "jr z,o"},	{0x29, "add hl,hl"},	{0x2A, "ld hl,(nn)"},	{0x2B, "dec hl"},	{0x2C, "inc l"},	{0x2D, "dec l"},	{0x2E, "ld l,n"},	{0x2F, "cpl"},
{0x30, "jr nc,o"},	{0x31, "ld sp,nn"},	{0x32, "ld (nn),a"},	{0x33, "inc sp"},	{0x34, "inc (hl)"},	{0x35, "dec (hl)"},	{0x36, "ld (hl),n"},	{0x37, "scf"},	{0x38, "jr c,o"},	{0x39, "add hl,sp"},	{0x3A, "ld a,(nn)"},	{0x3B, "dec sp"},	{0x3C, "inc a"},	{0x3D, "dec a"},	{0x3E, "ld a,n"},	{0x3F, "ccf"},
{0x40, "ld b,b"},	{0x41, "ld b,c"},	{0x42, "ld b,d"},	{0x43, "ld b,e"},	{0x44, "ld b,h"},	{0x45, "ld b,l"},	{0x46, "ld b,(hl)"},	{0x47, "ld b,a"},	{0x48, "ld c,b"},	{0x49, "ld c,c"},	{0x4A, "ld c,d"},	{0x4B, "ld c,e"},	{0x4C, "ld c,h"},	{0x4D, "ld c,l"},	{0x4E, "ld c,(hl)"},	{0x4F, "ld c,a"},
{0x50, "ld d,b"},	{0x51, "ld d,c"},	{0x52, "ld d,d"},	{0x53, "ld d,e"},	{0x54, "ld d,h"},	{0x55, "ld d,l"},	{0x56, "ld d,(hl)"},	{0x57, "ld d,a"},	{0x58, "ld e,b"},	{0x59, "ld e,c"},	{0x5A, "ld e,d"},	{0x5B, "ld e,e"},	{0x5C, "ld e,h"},	{0x5D, "ld e,l"},	{0x5E, "ld e,(hl)"},	{0x5F, "ld e,a"},
{0x60, "ld h,b"},	{0x61, "ld h,c"},	{0x62, "ld h,d"},	{0x63, "ld h,e"},	{0x64, "ld h,h"},	{0x65, "ld h,l"},	{0x66, "ld h,(hl)"},	{0x67, "ld h,a"},	{0x68, "ld l,b"},	{0x69, "ld l,c"},	{0x6A, "ld l,d"},	{0x6B, "ld l,e"},	{0x6C, "ld l,h"},	{0x6D, "ld l,l"},	{0x6E, "ld l,(hl)"},	{0x6F, "ld l,a"},
{0x70, "ld (hl),b"},	{0x71, "ld (hl),c"},	{0x72, "ld (hl),d"},	{0x73, "ld (hl),e"},	{0x74, "ld (hl),h"},	{0x75, "ld (hl),l"},	{0x76, "halt"},	{0x77, "ld (hl),a"},	{0x78, "ld a,b"},	{0x79, "ld a,c"},	{0x7A, "ld a,d"},	{0x7B, "ld a,e"},	{0x7C, "ld a,h"},	{0x7D, "ld a,l"},	{0x7E, "ld a,(hl)"},	{0x7F, "ld a,a"},
{0x80, "add a,b"},	{0x81, "add a,c"},	{0x82, "add a,d"},	{0x83, "add a,e"},	{0x84, "add a,h"},	{0x85, "add a,l"},	{0x86, "add a,(hl)"},	{0x87, "add a,a"},	{0x88, "adc a,b"},	{0x89, "adc a,c"},	{0x8A, "adc a,d"},	{0x8B, "adc a,e"},	{0x8C, "adc a,h"},	{0x8D, "adc a,l"},	{0x8E, "adc a,(hl)"},	{0x8F, "adc a,a"},
{0x90, "sub b"},	{0x91, "sub c"},	{0x92, "sub d"},	{0x93, "sub e"},	{0x94, "sub h"},	{0x95, "sub l"},	{0x96, "sub (hl)"},	{0x97, "sub a"},	{0x98, "sbc a,b"},	{0x99, "sbc a,c"},	{0x9A, "sbc a,d"},	{0x9B, "sbc a,e"},	{0x9C, "sbc a,h"},	{0x9D, "sbc a,l"},	{0x9E, "sbc a,(hl)"},	{0x9F, "sbc a,a"},
{0xA0, "and b"},	{0xA1, "and c"},	{0xA2, "and d"},	{0xA3, "and e"},	{0xA4, "and h"},	{0xA5, "and l"},	{0xA6, "and (hl)"},	{0xA7, "and a"},	{0xA8, "xor b"},	{0xA9, "xor c"},	{0xAA, "xor d"},	{0xAB, "xor e"},	{0xAC, "xor h"},	{0xAD, "xor l"},	{0xAE, "xor (hl)"},	{0xAF, "xor a"},
{0xB0, "or b"},	{0xB1, "or c"},	{0xB2, "or d"},	{0xB3, "or e"},	{0xB4, "or h"},	{0xB5, "or l"},	{0xB6, "or (hl)"},	{0xB7, "or a"},	{0xB8, "cp b"},	{0xB9, "cp c"},	{0xBA, "cp d"},	{0xBB, "cp e"},	{0xBC, "cp h"},	{0xBD, "cp l"},	{0xBE, "cp (hl)"},	{0xBF, "cp a"},
{0xC0, "ret nz"},	{0xC1, "pop bc"},	{0xC2, "jp nz,nn"},	{0xC3, "jp nn"},	{0xC4, "call nz,nn"},	{0xC5, "push bc"},	{0xC6, "add a,n"},	{0xC7, "rst 00h"},	{0xC8, "ret z"},	{0xC9, "ret"},	{0xCA, "jp z,nn"},		{0xCC, "call z,nn"},	{0xCD, "call nn"},	{0xCE, "adc a,n"},	{0xCF, "rst 08h"},
{0xD0, "ret nc"},	{0xD1, "pop de"},	{0xD2, "jp nc,nn"},	{0xD3, "out (n),a"},	{0xD4, "call nc,nn"},	{0xD5, "push de"},	{0xD6, "sub n"},	{0xD7, "rst 10h"},	{0xD8, "ret c"},	{0xD9, "exx"},	{0xDA, "jp c,nn"},	{0xDB, "in a,(n)"},	{0xDC, "call c,nn"},		{0xDE, "sbc a,n"},	{0xDF, "rst 18h"},
{0xE0, "ret po"},	{0xE1, "pop hl"},	{0xE2, "jp po,nn"},	{0xE3, "ex (sp),hl"},	{0xE4, "call po,nn"},	{0xE5, "push hl"},	{0xE6, "and n"},	{0xE7, "rst 20h"},	{0xE8, "ret pe"},	{0xE9, "jp (hl)"},	{0xEA, "jp pe,nn"},	{0xEB, "ex de,hl"},	{0xEC, "call pe,nn"},		{0xEE, "xor n"},	{0xEF, "rst 28h"},
{0xF0, "ret p"},	{0xF1, "pop af"},	{0xF2, "jp p,nn"},	{0xF3, "di"},	{0xF4, "call p,nn"},	{0xF5, "push af"},	{0xF6, "or n"},	{0xF7, "rst 30h"},	{0xF8, "ret m"},	{0xF9, "ld sp,hl"},	{0xFA, "jp m,nn"},	{0xFB, "ei"},	{0xFC, "call m,nn"},		{0xFE, "cp n"},	{0xFF, "rst 38h"},
// main-bit
{0xCB00, "rlc b"},	{0xCB01, "rlc c"},	{0xCB02, "rlc d"},	{0xCB03, "rlc e"},	{0xCB04, "rlc h"},	{0xCB05, "rlc l"},	{0xCB06, "rlc (hl)"},	{0xCB07, "rlc a"},	{0xCB08, "rrc b"},	{0xCB09, "rrc c"},	{0xCB0A, "rrc d"},	{0xCB0B, "rrc e"},	{0xCB0C, "rrc h"},	{0xCB0D, "rrc l"},	{0xCB0E, "rrc (hl)"},	{0xCB0F, "rrc a"},
{0xCB10, "rl b"},	{0xCB11, "rl c"},	{0xCB12, "rl d"},	{0xCB13, "rl e"},	{0xCB14, "rl h"},	{0xCB15, "rl l"},	{0xCB16, "rl (hl)"},	{0xCB17, "rl a"},	{0xCB18, "rr b"},	{0xCB19, "rr c"},	{0xCB1A, "rr d"},	{0xCB1B, "rr e"},	{0xCB1C, "rr h"},	{0xCB1D, "rr l"},	{0xCB1E, "rr (hl)"},	{0xCB1F, "rr a"},
{0xCB20, "sla b"},	{0xCB21, "sla c"},	{0xCB22, "sla d"},	{0xCB23, "sla e"},	{0xCB24, "sla h"},	{0xCB25, "sla l"},	{0xCB26, "sla (hl)"},	{0xCB27, "sla a"},	{0xCB28, "sra b"},	{0xCB29, "sra c"},	{0xCB2A, "sra d"},	{0xCB2B, "sra e"},	{0xCB2C, "sra h"},	{0xCB2D, "sra l"},	{0xCB2E, "sra (hl)"},	{0xCB2F, "sra a"},
{0xCB30, "sll b"},	{0xCB31, "sll c"},	{0xCB32, "sll d"},	{0xCB33, "sll e"},	{0xCB34, "sll h"},	{0xCB35, "sll l"},	{0xCB36, "sll (hl)"},	{0xCB37, "sll a"},	{0xCB38, "srl b"},	{0xCB39, "srl c"},	{0xCB3A, "srl d"},	{0xCB3B, "srl e"},	{0xCB3C, "srl h"},	{0xCB3D, "srl l"},	{0xCB3E, "srl (hl)"},	{0xCB3F, "srl a"},
{0xCB40, "bit 0,b"},	{0xCB41, "bit 0,c"},	{0xCB42, "bit 0,d"},	{0xCB43, "bit 0,e"},	{0xCB44, "bit 0,h"},	{0xCB45, "bit 0,l"},	{0xCB46, "bit 0,(hl)"},	{0xCB47, "bit 0,a"},	{0xCB48, "bit 1,b"},	{0xCB49, "bit 1,c"},	{0xCB4A, "bit 1,d"},	{0xCB4B, "bit 1,e"},	{0xCB4C, "bit 1,h"},	{0xCB4D, "bit 1,l"},	{0xCB4E, "bit 1,(hl)"},	{0xCB4F, "bit 1,a"},
{0xCB50, "bit 2,b"},	{0xCB51, "bit 2,c"},	{0xCB52, "bit 2,d"},	{0xCB53, "bit 2,e"},	{0xCB54, "bit 2,h"},	{0xCB55, "bit 2,l"},	{0xCB56, "bit 2,(hl)"},	{0xCB57, "bit 2,a"},	{0xCB58, "bit 3,b"},	{0xCB59, "bit 3,c"},	{0xCB5A, "bit 3,d"},	{0xCB5B, "bit 3,e"},	{0xCB5C, "bit 3,h"},	{0xCB5D, "bit 3,l"},	{0xCB5E, "bit 3,(hl)"},	{0xCB5F, "bit 3,a"},
{0xCB60, "bit 4,b"},	{0xCB61, "bit 4,c"},	{0xCB62, "bit 4,d"},	{0xCB63, "bit 4,e"},	{0xCB64, "bit 4,h"},	{0xCB65, "bit 4,l"},	{0xCB66, "bit 4,(hl)"},	{0xCB67, "bit 4,a"},	{0xCB68, "bit 5,b"},	{0xCB69, "bit 5,c"},	{0xCB6A, "bit 5,d"},	{0xCB6B, "bit 5,e"},	{0xCB6C, "bit 5,h"},	{0xCB6D, "bit 5,l"},	{0xCB6E, "bit 5,(hl)"},	{0xCB6F, "bit 5,a"},
{0xCB70, "bit 6,b"},	{0xCB71, "bit 6,c"},	{0xCB72, "bit 6,d"},	{0xCB73, "bit 6,e"},	{0xCB74, "bit 6,h"},	{0xCB75, "bit 6,l"},	{0xCB76, "bit 6,(hl)"},	{0xCB77, "bit 6,a"},	{0xCB78, "bit 7,b"},	{0xCB79, "bit 7,c"},	{0xCB7A, "bit 7,d"},	{0xCB7B, "bit 7,e"},	{0xCB7C, "bit 7,h"},	{0xCB7D, "bit 7,l"},	{0xCB7E, "bit 7,(hl)"},	{0xCB7F, "bit 7,a"},
{0xCB80, "res 0,b"},	{0xCB81, "res 0,c"},	{0xCB82, "res 0,d"},	{0xCB83, "res 0,e"},	{0xCB84, "res 0,h"},	{0xCB85, "res 0,l"},	{0xCB86, "res 0,(hl)"},	{0xCB87, "res 0,a"},	{0xCB88, "res 1,b"},	{0xCB89, "res 1,c"},	{0xCB8A, "res 1,d"},	{0xCB8B, "res 1,e"},	{0xCB8C, "res 1,h"},	{0xCB8D, "res 1,l"},	{0xCB8E, "res 1,(hl)"},	{0xCB8F, "res 1,a"},
{0xCB90, "res 2,b"},	{0xCB91, "res 2,c"},	{0xCB92, "res 2,d"},	{0xCB93, "res 2,e"},	{0xCB94, "res 2,h"},	{0xCB95, "res 2,l"},	{0xCB96, "res 2,(hl)"},	{0xCB97, "res 2,a"},	{0xCB98, "res 3,b"},	{0xCB99, "res 3,c"},	{0xCB9A, "res 3,d"},	{0xCB9B, "res 3,e"},	{0xCB9C, "res 3,h"},	{0xCB9D, "res 3,l"},	{0xCB9E, "res 3,(hl)"},	{0xCB9F, "res 3,a"},
{0xCBA0, "res 4,b"},	{0xCBA1, "res 4,c"},	{0xCBA2, "res 4,d"},	{0xCBA3, "res 4,e"},	{0xCBA4, "res 4,h"},	{0xCBA5, "res 4,l"},	{0xCBA6, "res 4,(hl)"},	{0xCBA7, "res 4,a"},	{0xCBA8, "res 5,b"},	{0xCBA9, "res 5,c"},	{0xCBAA, "res 5,d"},	{0xCBAB, "res 5,e"},	{0xCBAC, "res 5,h"},	{0xCBAD, "res 5,l"},	{0xCBAE, "res 5,(hl)"},	{0xCBAF, "res 5,a"},
{0xCBB0, "res 6,b"},	{0xCBB1, "res 6,c"},	{0xCBB2, "res 6,d"},	{0xCBB3, "res 6,e"},	{0xCBB4, "res 6,h"},	{0xCBB5, "res 6,l"},	{0xCBB6, "res 6,(hl)"},	{0xCBB7, "res 6,a"},	{0xCBB8, "res 7,b"},	{0xCBB9, "res 7,c"},	{0xCBBA, "res 7,d"},	{0xCBBB, "res 7,e"},	{0xCBBC, "res 7,h"},	{0xCBBD, "res 7,l"},	{0xCBBE, "res 7,(hl)"},	{0xCBBF, "res 7,a"},
{0xCBC0, "set 0,b"},	{0xCBC1, "set 0,c"},	{0xCBC2, "set 0,d"},	{0xCBC3, "set 0,e"},	{0xCBC4, "set 0,h"},	{0xCBC5, "set 0,l"},	{0xCBC6, "set 0,(hl)"},	{0xCBC7, "set 0,a"},	{0xCBC8, "set 1,b"},	{0xCBC9, "set 1,c"},	{0xCBCA, "set 1,d"},	{0xCBCB, "set 1,e"},	{0xCBCC, "set 1,h"},	{0xCBCD, "set 1,l"},	{0xCBCE, "set 1,(hl)"},	{0xCBCF, "set 1,a"},
{0xCBD0, "set 2,b"},	{0xCBD1, "set 2,c"},	{0xCBD2, "set 2,d"},	{0xCBD3, "set 2,e"},	{0xCBD4, "set 2,h"},	{0xCBD5, "set 2,l"},	{0xCBD6, "set 2,(hl)"},	{0xCBD7, "set 2,a"},	{0xCBD8, "set 3,b"},	{0xCBD9, "set 3,c"},	{0xCBDA, "set 3,d"},	{0xCBDB, "set 3,e"},	{0xCBDC, "set 3,h"},	{0xCBDD, "set 3,l"},	{0xCBDE, "set 3,(hl)"},	{0xCBDF, "set 3,a"},
{0xCBE0, "set 4,b"},	{0xCBE1, "set 4,c"},	{0xCBE2, "set 4,d"},	{0xCBE3, "set 4,e"},	{0xCBE4, "set 4,h"},	{0xCBE5, "set 4,l"},	{0xCBE6, "set 4,(hl)"},	{0xCBE7, "set 4,a"},	{0xCBE8, "set 5,b"},	{0xCBE9, "set 5,c"},	{0xCBEA, "set 5,d"},	{0xCBEB, "set 5,e"},	{0xCBEC, "set 5,h"},	{0xCBED, "set 5,l"},	{0xCBEE, "set 5,(hl)"},	{0xCBEF, "set 5,a"},
{0xCBF0, "set 6,b"},	{0xCBF1, "set 6,c"},	{0xCBF2, "set 6,d"},	{0xCBF3, "set 6,e"},	{0xCBF4, "set 6,h"},	{0xCBF5, "set 6,l"},	{0xCBF6, "set 6,(hl)"},	{0xCBF7, "set 6,a"},	{0xCBF8, "set 7,b"},	{0xCBF9, "set 7,c"},	{0xCBFA, "set 7,d"},	{0xCBFB, "set 7,e"},	{0xCBFC, "set 7,h"},	{0xCBFD, "set 7,l"},	{0xCBFE, "set 7,(hl)"},	{0xCBFF, "set 7,a"},
// ix
{0xDD04, "inc b"},	{0xDD05, "dec b"},	{0xDD06, "ld b,n"},			{0xDD09, "add ix,bc"},			{0xDD0C, "inc c"},	{0xDD0D, "dec c"},	{0xDD0E, "ld c,n"},	
{0xDD14, "inc d"},	{0xDD15, "dec d"},	{0xDD16, "ld d,n"},			{0xDD19, "add ix,de"},			{0xDD1C, "inc e"},	{0xDD1D, "dec e"},	{0xDD1E, "ld e,n"},	
{0xDD21, "ld ix,nn"},	{0xDD22, "ld (nn),ix"},	{0xDD23, "inc ix"},	{0xDD24, "inc ixh"},	{0xDD25, "dec ixh"},	{0xDD26, "ld ixh,n"},			{0xDD29, "add ix,ix"},	{0xDD2A, "ld ix,(nn)"},	{0xDD2B, "dec ix"},	{0xDD2C, "inc ixl"},	{0xDD2D, "dec ixl"},	{0xDD2E, "ld ixl,n"},	
{0xDD34, "inc (ix+o)"},	{0xDD35, "dec (ix+o)"},	{0xDD36, "ld (ix+o),n"},	{0xDD37, "ld (ix+o),n"},		{0xDD39, "add ix,sp"},			{0xDD3C, "inc a"},	{0xDD3D, "dec a"},	{0xDD3E, "ld a,n"},	
{0xDD40, "ld b,b"},	{0xDD41, "ld b,c"},	{0xDD42, "ld b,d"},	{0xDD43, "ld b,e"},	{0xDD44, "ld b,ixh"},	{0xDD45, "ld b,ixl"},	{0xDD46, "ld b,(ix+o)"},	{0xDD47, "ld b,a"},	{0xDD48, "ld c,b"},	{0xDD49, "ld c,c"},	{0xDD4A, "ld c,d"},	{0xDD4B, "ld c,e"},	{0xDD4C, "ld c,ixh"},	{0xDD4D, "ld c,ixl"},	{0xDD4E, "ld c,(ix+o)"},	{0xDD4F, "ld c,a"},
{0xDD50, "ld d,b"},	{0xDD51, "ld d,c"},	{0xDD52, "ld d,d"},	{0xDD53, "ld d,e"},	{0xDD54, "ld d,ixh"},	{0xDD55, "ld d,ixl"},	{0xDD56, "ld d,(ix+o)"},	{0xDD57, "ld d,a"},	{0xDD58, "ld e,b"},	{0xDD59, "ld e,c"},	{0xDD5A, "ld e,d"},	{0xDD5B, "ld e,e"},	{0xDD5C, "ld e,ixh"},	{0xDD5D, "ld e,ixl"},	{0xDD5E, "ld e,(ix+o)"},	{0xDD5F, "ld e,a"},
{0xDD60, "ld ixh,b"},	{0xDD61, "ld ixh,c"},	{0xDD62, "ld ixh,d"},	{0xDD63, "ld ixh,e"},	{0xDD64, "ld ixh,ixh"},	{0xDD65, "ld ixh,ixl"},	{0xDD66, "ld h,(ix+o)"},	{0xDD67, "ld ixh,a"},	{0xDD68, "ld ixl,b"},	{0xDD69, "ld ixl,c"},	{0xDD6A, "ld ixl,d"},	{0xDD6B, "ld ixl,e"},	{0xDD6C, "ld ixl,ixh"},	{0xDD6D, "ld ixl,ixl"},	{0xDD6E, "ld l,(ix+o)"},	{0xDD6F, "ld ixl,a"},
{0xDD70, "ld (ix+o),b"},	{0xDD71, "ld (ix+o),c"},	{0xDD72, "ld (ix+o),d"},	{0xDD73, "ld (ix+o),e"},	{0xDD74, "ld (ix+o),h"},	{0xDD75, "ld (ix+o),l"},		{0xDD77, "ld (ix+o),a"},	{0xDD78, "ld a,b"},	{0xDD79, "ld a,c"},	{0xDD7A, "ld a,d"},	{0xDD7B, "ld a,e"},	{0xDD7C, "ld a,ixh"},	{0xDD7D, "ld a,ixl"},	{0xDD7E, "ld a,(ix+o)"},	{0xDD7F, "ld a,a"},
{0xDD80, "add a,b"},	{0xDD81, "add a,c"},	{0xDD82, "add a,d"},	{0xDD83, "add a,e"},	{0xDD84, "add a,ixh"},	{0xDD85, "add a,ixl"},	{0xDD86, "add a,(ix+o)"},	{0xDD87, "add a,a"},	{0xDD88, "adc a,b"},	{0xDD89, "adc a,c"},	{0xDD8A, "adc a,d"},	{0xDD8B, "adc a,e"},	{0xDD8C, "adc a,ixh"},	{0xDD8D, "adc a,ixl"},	{0xDD8E, "adc a,(ix+o)"},	{0xDD8F, "adc a,a"},
{0xDD90, "sub b"},	{0xDD91, "sub c"},	{0xDD92, "sub d"},	{0xDD93, "sub e"},	{0xDD94, "sub ixh"},	{0xDD95, "sub ixl"},	{0xDD96, "sub (ix+o)"},	{0xDD97, "sub a"},	{0xDD98, "sbc a,b"},	{0xDD99, "sbc a,c"},	{0xDD9A, "sbc a,d"},	{0xDD9B, "sbc a,e"},	{0xDD9C, "sbc a,ixh"},	{0xDD9D, "sbc a,ixl"},	{0xDD9E, "sbc a,(ix+o)"},	{0xDD9F, "sbc a,a"},
{0xDDA0, "and b"},	{0xDDA1, "and c"},	{0xDDA2, "and d"},	{0xDDA3, "and e"},	{0xDDA4, "and ixh"},	{0xDDA5, "and ixl"},	{0xDDA6, "and (ix+o)"},	{0xDDA7, "and a"},	{0xDDA8, "xor b"},	{0xDDA9, "xor c"},	{0xDDAA, "xor d"},	{0xDDAB, "xor e"},	{0xDDAC, "xor ixh"},	{0xDDAD, "xor ixl"},	{0xDDAE, "xor (ix+o)"},	{0xDDAF, "xor a"},
{0xDDB0, "or b"},	{0xDDB1, "or c"},	{0xDDB2, "or d"},	{0xDDB3, "or e"},	{0xDDB4, "or ixh"},	{0xDDB5, "or ixl"},	{0xDDB6, "or (ix+o)"},	{0xDDB7, "or a"},	{0xDDB8, "cp b"},	{0xDDB9, "cp c"},	{0xDDBA, "cp d"},	{0xDDBB, "cp e"},	{0xDDBC, "cp ixh"},	{0xDDBD, "cp ixl"},	{0xDDBE, "cp (ix+o)"},	{0xDDBF, "cp a"},
{0xDDE1, "pop ix"},		{0xDDE3, "ex (sp),ix"},		{0xDDE5, "push ix"},				{0xDDE9, "jp (ix)"},						
{0xDDF9, "ld sp,ix"},	
// misc
{0xED40, "in b,(c)"},	{0xED41, "out (c),b"},	{0xED42, "sbc hl,bc"},	{0xED43, "ld (nn),bc"},	{0xED44, "neg"},	{0xED45, "retn"},	{0xED46, "im 0"},	{0xED47, "ld i,a"},	{0xED48, "in c,(c)"},	{0xED49, "out (c),c"},	{0xED4A, "adc hl,bc"},	{0xED4B, "ld bc,(nn)"},	{0xED4C, "neg"},	{0xED4D, "reti"},		{0xED4F, "ld r,a"},
{0xED50, "in d,(c)"},	{0xED51, "out (c),d"},	{0xED52, "sbc hl,de"},	{0xED53, "ld (nn),de"},	{0xED54, "neg"},	{0xED55, "retn"},	{0xED56, "im 1"},	{0xED57, "ld a,i"},	{0xED58, "in e,(c)"},	{0xED59, "out (c),e"},	{0xED5A, "adc hl,de"},	{0xED5B, "ld de,(nn)"},	{0xED5C, "neg"},	{0xED5D, "reti"},	{0xED5E, "im 2"},	{0xED5F, "ld a,r"},
{0xED60, "in h,(c)"},	{0xED61, "out (c),h"},	{0xED62, "sbc hl,hl"},	{0xED63, "ld (nn),hl"},	{0xED64, "neg"},	{0xED65, "retn"},		{0xED67, "rrd"},	{0xED68, "in l,(c)"},	{0xED69, "out (c),l"},	{0xED6A, "adc hl,hl"},	{0xED6B, "ld hl,(nn)"},	{0xED6C, "neg"},	{0xED6D, "reti"},		{0xED6F, "rld"},
{0xED70, "in (c)"},	{0xED71, "out (c),0"},	{0xED72, "sbc hl,sp"},	{0xED73, "ld (nn),sp"},	{0xED74, "neg"},	{0xED75, "retn"},	{0xED76, "slp"},		{0xED78, "in a,(c)"},	{0xED79, "out (c),a"},	{0xED7A, "adc hl,sp"},	{0xED7B, "ld sp,(nn)"},	{0xED7C, "neg"},	{0xED7D, "reti"},		
{0xEDA0, "ldi"},	{0xEDA1, "cpi"},	{0xEDA2, "ini"},	{0xEDA3, "outi"},					{0xEDA8, "ldd"},	{0xEDA9, "cpd"},	{0xEDAA, "ind"},	{0xEDAB, "outd"},				
{0xEDB0, "ldir"},	{0xEDB1, "cpir"},	{0xEDB2, "inir"},	{0xEDB3, "otir"},					{0xEDB8, "lddr"},	{0xEDB9, "cpdr"},	{0xEDBA, "indr"},	{0xEDBB, "otdr"},				
// iy
{0xFD04, "inc b"},	{0xFD05, "dec b"},	{0xFD06, "ld b,n"},			{0xFD09, "add iy,bc"},			{0xFD0C, "inc c"},	{0xFD0D, "dec c"},	{0xFD0E, "ld c,n"},		
{0xFD14, "inc d"},	{0xFD15, "dec d"},	{0xFD16, "ld d,n"},			{0xFD19, "add iy,de"},			{0xFD1C, "inc e"},	{0xFD1D, "dec e"},	{0xFD1E, "ld e,n"},	
{0xFD21, "ld iy,nn"},	{0xFD22, "ld (nn),iy"},	{0xFD23, "inc iy"},	{0xFD24, "inc iyh"},	{0xFD25, "dec iyh"},	{0xFD26, "ld iyh,n"},			{0xFD29, "add iy,iy"},	{0xFD2A, "ld iy,(nn)"},	{0xFD2B, "dec iy"},	{0xFD2C, "inc iyl"},	{0xFD2D, "dec iyl"},	{0xFD2E, "ld iyl,n"},	
{0xFD34, "inc (iy+o)"},	{0xFD35, "dec (iy+o)"},	{0xFD36, "ld (iy+o),n"},			{0xFD39, "add iy,sp"},			{0xFD3C, "inc a"},	{0xFD3D, "dec a"},	{0xFD3E, "ld a,n"},	
{0xFD40, "ld b,b"},	{0xFD41, "ld b,c"},	{0xFD42, "ld b,d"},	{0xFD43, "ld b,e"},	{0xFD44, "ld b,iyh"},	{0xFD45, "ld b,iyl"},	{0xFD46, "ld b,(iy+o)"},	{0xFD47, "ld b,a"},	{0xFD48, "ld c,b"},	{0xFD49, "ld c,c"},	{0xFD4A, "ld c,d"},	{0xFD4B, "ld c,e"},	{0xFD4C, "ld c,iyh"},	{0xFD4D, "ld c,iyl"},	{0xFD4E, "ld c,(iy+o)"},	{0xFD4F, "ld c,a"},
{0xFD50, "ld d,b"},	{0xFD51, "ld d,c"},	{0xFD52, "ld d,d"},	{0xFD53, "ld d,e"},	{0xFD54, "ld d,iyh"},	{0xFD55, "ld d,iyl"},	{0xFD56, "ld d,(iy+o)"},	{0xFD57, "ld d,a"},	{0xFD58, "ld e,b"},	{0xFD59, "ld e,c"},	{0xFD5A, "ld e,d"},	{0xFD5B, "ld e,e"},	{0xFD5C, "ld e,iyh"},	{0xFD5D, "ld e,iyl"},	{0xFD5E, "ld e,(iy+o)"},	{0xFD5F, "ld e,a"},
{0xFD60, "ld iyh,b"},	{0xFD61, "ld iyh,c"},	{0xFD62, "ld iyh,d"},	{0xFD63, "ld iyh,e"},	{0xFD64, "ld iyh,iyh"},	{0xFD65, "ld iyh,iyl"},	{0xFD66, "ld h,(iy+o)"},	{0xFD67, "ld iyh,a"},	{0xFD68, "ld iyl,b"},	{0xFD69, "ld iyl,c"},	{0xFD6A, "ld iyl,d"},	{0xFD6B, "ld iyl,e"},	{0xFD6C, "ld iyl,iyh"},	{0xFD6D, "ld iyl,iyl"},	{0xFD6E, "ld l,(iy+o)"},	{0xFD6F, "ld iyl,a"},
{0xFD70, "ld (iy+o),b"},	{0xFD71, "ld (iy+o),c"},	{0xFD72, "ld (iy+o),d"},	{0xFD73, "ld (iy+o),e"},	{0xFD74, "ld (iy+o),h"},	{0xFD75, "ld (iy+o),l"},		{0xFD77, "ld (iy+o),a"},	{0xFD78, "ld a,b"},	{0xFD79, "ld a,c"},	{0xFD7A, "ld a,d"},	{0xFD7B, "ld a,e"},	{0xFD7C, "ld a,iyh"},	{0xFD7D, "ld a,iyl"},	{0xFD7E, "ld a,(iy+o)"},	{0xFD7F, "ld a,a"},
{0xFD80, "add a,b"},	{0xFD81, "add a,c"},	{0xFD82, "add a,d"},	{0xFD83, "add a,e"},	{0xFD84, "add a,iyh"},	{0xFD85, "add a,iyl"},	{0xFD86, "add a,(iy+o)"},	{0xFD87, "add a,a"},	{0xFD88, "adc a,b"},	{0xFD89, "adc a,c"},	{0xFD8A, "adc a,d"},	{0xFD8B, "adc a,e"},	{0xFD8C, "adc a,iyh"},	{0xFD8D, "adc a,iyl"},	{0xFD8E, "adc a,(iy+o)"},	{0xFD8F, "adc a,a"},
{0xFD90, "sub b"},	{0xFD91, "sub c"},	{0xFD92, "sub d"},	{0xFD93, "sub e"},	{0xFD94, "sub iyh"},	{0xFD95, "sub iyl"},	{0xFD96, "sub (iy+o)"},	{0xFD97, "sub a"},	{0xFD98, "sbc a,b"},	{0xFD99, "sbc a,c"},	{0xFD9A, "sbc a,d"},	{0xFD9B, "sbc a,e"},	{0xFD9C, "sbc a,iyh"},	{0xFD9D, "sbc a,iyl"},	{0xFD9E, "sbc a,(iy+o)"},	{0xFD9F, "sbc a,a"},
{0xFDA0, "and b"},	{0xFDA1, "and c"},	{0xFDA2, "and d"},	{0xFDA3, "and e"},	{0xFDA4, "and iyh"},	{0xFDA5, "and iyl"},	{0xFDA6, "and (iy+o)"},	{0xFDA7, "and a"},	{0xFDA8, "xor b"},	{0xFDA9, "xor c"},	{0xFDAA, "xor d"},	{0xFDAB, "xor e"},	{0xFDAC, "xor iyh"},	{0xFDAD, "xor iyl"},	{0xFDAE, "xor (iy+o)"},	{0xFDAF, "xor a"},
{0xFDB0, "or b"},	{0xFDB1, "or c"},	{0xFDB2, "or d"},	{0xFDB3, "or e"},	{0xFDB4, "or iyh"},	{0xFDB5, "or iyl"},	{0xFDB6, "or (iy+o)"},	{0xFDB7, "or a"},	{0xFDB8, "cp b"},	{0xFDB9, "cp c"},	{0xFDBA, "cp d"},	{0xFDBB, "cp e"},	{0xFDBC, "cp iyh"},	{0xFDBD, "cp iyl"},	{0xFDBE, "cp (iy+o)"},	{0xFDBF, "cp a"},
{0xFDE1, "pop iy"},		{0xFDE3, "ex (sp),iy"},		{0xFDE5, "push iy"},				{0xFDE9, "jp (iy)"},						
{0xFDF9, "ld sp,iy"}	
};



// ix-bit
const std::unordered_map<u8, std::string> hmInstruccionesDDCB = {
{0x00, "rlc (ix+o),b"},	{0x01, "rlc (ix+o),c"},	{0x02, "rlc (ix+o),d"},	{0x03, "rlc (ix+o),e"},	{0x04, "rlc (ix+o),h"},	{0x05, "rlc (ix+o),l"},	{0x06, "rlc (ix+o)"},	{0x07, "rlc (ix+o),a"},	{0x08, "rrc (ix+o),b"},	{0x09, "rrc (ix+o),c"},	{0x0A, "rrc (ix+o),d"},	{0x0B, "rrc (ix+o),e"},	{0x0C, "rrc (ix+o),h"},	{0x0D, "rrc (ix+o),l"},	{0x0E, "rrc (ix+o)"},	{0x0F, "rrc (ix+o),a"},
{0x10, "rl (ix+o),b"},	{0x11, "rl (ix+o),c"},	{0x12, "rl (ix+o),d"},	{0x13, "rl (ix+o),e"},	{0x14, "rl (ix+o),h"},	{0x15, "rl (ix+o),l"},	{0x16, "rl (ix+o)"},	{0x17, "rl (ix+o),a"},	{0x18, "rr (ix+o),b"},	{0x19, "rr (ix+o),c"},	{0x1A, "rr (ix+o),d"},	{0x1B, "rr (ix+o),e"},	{0x1C, "rr (ix+o),h"},	{0x1D, "rr (ix+o),l"},	{0x1E, "rr (ix+o)"},	{0x1F, "rr (ix+o),a"},
{0x20, "sla (ix+o),b"},	{0x21, "sla (ix+o),c"},	{0x22, "sla (ix+o),d"},	{0x23, "sla (ix+o),e"},	{0x24, "sla (ix+o),h"},	{0x25, "sla (ix+o),l"},	{0x26, "sla (ix+o)"},	{0x27, "sla (ix+o),a"},	{0x28, "sra (ix+o),b"},	{0x29, "sra (ix+o),c"},	{0x2A, "sra (ix+o),d"},	{0x2B, "sra (ix+o),e"},	{0x2C, "sra (ix+o),h"},	{0x2D, "sra (ix+o),l"},	{0x2E, "sra (ix+o)"},	{0x2F, "sra (ix+o),a"},
{0x30, "sll (ix+o),b"},	{0x31, "sll (ix+o),c"},	{0x32, "sll (ix+o),d"},	{0x33, "sll (ix+o),e"},	{0x34, "sll (ix+o),h"},	{0x35, "sll (ix+o),l"},	{0x36, "sll (ix+o)"},	{0x37, "sll (ix+o),a"},	{0x38, "srl (ix+o),b"},	{0x39, "srl (ix+o),c"},	{0x3A, "srl (ix+o),d"},	{0x3B, "srl (ix+o),e"},	{0x3C, "srl (ix+o),h"},	{0x3D, "srl (ix+o),l"},	{0x3E, "srl (ix+o)"},	{0x3F, "srl (ix+o),a"},
{0x40, "bit 0,(ix+o)"},	{0x41, "bit 0,(ix+o)"},	{0x42, "bit 0,(ix+o)"},	{0x43, "bit 0,(ix+o)"},	{0x44, "bit 0,(ix+o)"},	{0x45, "bit 0,(ix+o)"},	{0x46, "bit 0,(ix+o)"},	{0x47, "bit 0,(ix+o)"},	{0x48, "bit 1,(ix+o)"},	{0x49, "bit 1,(ix+o)"},	{0x4A, "bit 1,(ix+o)"},	{0x4B, "bit 1,(ix+o)"},	{0x4C, "bit 1,(ix+o)"},	{0x4D, "bit 1,(ix+o)"},	{0x4E, "bit 1,(ix+o)"},	{0x4F, "bit 1,(ix+o)"},
{0x50, "bit 2,(ix+o)"},	{0x51, "bit 2,(ix+o)"},	{0x52, "bit 2,(ix+o)"},	{0x53, "bit 2,(ix+o)"},	{0x54, "bit 2,(ix+o)"},	{0x55, "bit 2,(ix+o)"},	{0x56, "bit 2,(ix+o)"},	{0x57, "bit 2,(ix+o)"},	{0x58, "bit 3,(ix+o)"},	{0x59, "bit 3,(ix+o)"},	{0x5A, "bit 3,(ix+o)"},	{0x5B, "bit 3,(ix+o)"},	{0x5C, "bit 3,(ix+o)"},	{0x5D, "bit 3,(ix+o)"},	{0x5E, "bit 3,(ix+o)"},	{0x5F, "bit 3,(ix+o)"},
{0x60, "bit 4,(ix+o)"},	{0x61, "bit 4,(ix+o)"},	{0x62, "bit 4,(ix+o)"},	{0x63, "bit 4,(ix+o)"},	{0x64, "bit 4,(ix+o)"},	{0x65, "bit 4,(ix+o)"},	{0x66, "bit 4,(ix+o)"},	{0x67, "bit 4,(ix+o)"},	{0x68, "bit 5,(ix+o)"},	{0x69, "bit 5,(ix+o)"},	{0x6A, "bit 5,(ix+o)"},	{0x6B, "bit 5,(ix+o)"},	{0x6C, "bit 5,(ix+o)"},	{0x6D, "bit 5,(ix+o)"},	{0x6E, "bit 5,(ix+o)"},	{0x6F, "bit 5,(ix+o)"},
{0x70, "bit 6,(ix+o)"},	{0x71, "bit 6,(ix+o)"},	{0x72, "bit 6,(ix+o)"},	{0x73, "bit 6,(ix+o)"},	{0x74, "bit 6,(ix+o)"},	{0x75, "bit 6,(ix+o)"},	{0x76, "bit 6,(ix+o)"},	{0x77, "bit 6,(ix+o)"},	{0x78, "bit 7,(ix+o)"},	{0x79, "bit 7,(ix+o)"},	{0x7A, "bit 7,(ix+o)"},	{0x7B, "bit 7,(ix+o)"},	{0x7C, "bit 7,(ix+o)"},	{0x7D, "bit 7,(ix+o)"},	{0x7E, "bit 7,(ix+o)"},	{0x7F, "bit 7,(ix+o)"},
{0x80, "res 0,(ix+o),b"},	{0x81, "res 0,(ix+o),c"},	{0x82, "res 0,(ix+o),d"},	{0x83, "res 0,(ix+o),e"},	{0x84, "res 0,(ix+o),h"},	{0x85, "res 0,(ix+o),l"},	{0x86, "res 0,(ix+o)"},	{0x87, "res 0,(ix+o),a"},	{0x88, "res 1,(ix+o),b"},	{0x89, "res 1,(ix+o),c"},	{0x8A, "res 1,(ix+o),d"},	{0x8B, "res 1,(ix+o),e"},	{0x8C, "res 1,(ix+o),h"},	{0x8D, "res 1,(ix+o),l"},	{0x8E, "res 1,(ix+o)"},	{0x8F, "res 1,(ix+o),a"},
{0x90, "res 2,(ix+o),b"},	{0x91, "res 2,(ix+o),c"},	{0x92, "res 2,(ix+o),d"},	{0x93, "res 2,(ix+o),e"},	{0x94, "res 2,(ix+o),h"},	{0x95, "res 2,(ix+o),l"},	{0x96, "res 2,(ix+o)"},	{0x97, "res 2,(ix+o),a"},	{0x98, "res 3,(ix+o),b"},	{0x99, "res 3,(ix+o),c"},	{0x9A, "res 3,(ix+o),d"},	{0x9B, "res 3,(ix+o),e"},	{0x9C, "res 3,(ix+o),h"},	{0x9D, "res 3,(ix+o),l"},	{0x9E, "res 3,(ix+o)"},	{0x9F, "res 3,(ix+o),a"},
{0xA0, "res 4,(ix+o),b"},	{0xA1, "res 4,(ix+o),c"},	{0xA2, "res 4,(ix+o),d"},	{0xA3, "res 4,(ix+o),e"},	{0xA4, "res 4,(ix+o),h"},	{0xA5, "res 4,(ix+o),l"},	{0xA6, "res 4,(ix+o)"},	{0xA7, "res 4,(ix+o),a"},	{0xA8, "res 5,(ix+o),b"},	{0xA9, "res 5,(ix+o),c"},	{0xAA, "res 5,(ix+o),d"},	{0xAB, "res 5,(ix+o),e"},	{0xAC, "res 5,(ix+o),h"},	{0xAD, "res 5,(ix+o),l"},	{0xAE, "res 5,(ix+o)"},	{0xAF, "res 5,(ix+o),a"},
{0xB0, "res 6,(ix+o),b"},	{0xB1, "res 6,(ix+o),c"},	{0xB2, "res 6,(ix+o),d"},	{0xB3, "res 6,(ix+o),e"},	{0xB4, "res 6,(ix+o),h"},	{0xB5, "res 6,(ix+o),l"},	{0xB6, "res 6,(ix+o)"},	{0xB7, "res 6,(ix+o),a"},	{0xB8, "res 7,(ix+o),b"},	{0xB9, "res 7,(ix+o),c"},	{0xBA, "res 7,(ix+o),d"},	{0xBB, "res 7,(ix+o),e"},	{0xBC, "res 7,(ix+o),h"},	{0xBD, "res 7,(ix+o),l"},	{0xBE, "res 7,(ix+o)"},	{0xBF, "res 7,(ix+o),a"},
{0xC0, "set 0,(ix+o),b"},	{0xC1, "set 0,(ix+o),c"},	{0xC2, "set 0,(ix+o),d"},	{0xC3, "set 0,(ix+o),e"},	{0xC4, "set 0,(ix+o),h"},	{0xC5, "set 0,(ix+o),l"},	{0xC6, "set 0,(ix+o)"},	{0xC7, "set 0,(ix+o),a"},	{0xC8, "set 1,(ix+o),b"},	{0xC9, "set 1,(ix+o),c"},	{0xCA, "set 1,(ix+o),d"},	{0xCB, "set 1,(ix+o),e"},	{0xCC, "set 1,(ix+o),h"},	{0xCD, "set 1,(ix+o),l"},	{0xCE, "set 1,(ix+o)"},	{0xCF, "set 1,(ix+o),a"},
{0xD0, "set 2,(ix+o),b"},	{0xD1, "set 2,(ix+o),c"},	{0xD2, "set 2,(ix+o),d"},	{0xD3, "set 2,(ix+o),e"},	{0xD4, "set 2,(ix+o),h"},	{0xD5, "set 2,(ix+o),l"},	{0xD6, "set 2,(ix+o)"},	{0xD7, "set 2,(ix+o),a"},	{0xD8, "set 3,(ix+o),b"},	{0xD9, "set 3,(ix+o),c"},	{0xDA, "set 3,(ix+o),d"},	{0xDB, "set 3,(ix+o),e"},	{0xDC, "set 3,(ix+o),h"},	{0xDD, "set 3,(ix+o),l"},	{0xDE, "set 3,(ix+o)"},	{0xDF, "set 3,(ix+o),a"},
{0xE0, "set 4,(ix+o),b"},	{0xE1, "set 4,(ix+o),c"},	{0xE2, "set 4,(ix+o),d"},	{0xE3, "set 4,(ix+o),e"},	{0xE4, "set 4,(ix+o),h"},	{0xE5, "set 4,(ix+o),l"},	{0xE6, "set 4,(ix+o)"},	{0xE7, "set 4,(ix+o),a"},	{0xE8, "set 5,(ix+o),b"},	{0xE9, "set 5,(ix+o),c"},	{0xEA, "set 5,(ix+o),d"},	{0xEB, "set 5,(ix+o),e"},	{0xEC, "set 5,(ix+o),h"},	{0xED, "set 5,(ix+o),l"},	{0xEE, "set 5,(ix+o)"},	{0xEF, "set 5,(ix+o),a"},
{0xF0, "set 6,(ix+o),b"},	{0xF1, "set 6,(ix+o),c"},	{0xF2, "set 6,(ix+o),d"},	{0xF3, "set 6,(ix+o),e"},	{0xF4, "set 6,(ix+o),h"},	{0xF5, "set 6,(ix+o),l"},	{0xF6, "set 6,(ix+o)"},	{0xF7, "set 6,(ix+o),a"},	{0xF8, "set 7,(ix+o),b"},	{0xF9, "set 7,(ix+o),c"},	{0xFA, "set 7,(ix+o),d"},	{0xFB, "set 7,(ix+o),e"},	{0xFC, "set 7,(ix+o),h"},	{0xFD, "set 7,(ix+o),l"},	{0xFE, "set 7,(ix+o)"},	{0xFF, "set 7,(ix+o),a"}
};

// iy-bit
const std::unordered_map<u8, std::string> hmInstruccionesFDCB = {
{0x00, "rlc (iy+o),b"},	{0x01, "rlc (iy+o),c"},	{0x02, "rlc (iy+o),d"},	{0x03, "rlc (iy+o),e"},	{0x04, "rlc (iy+o),h"},	{0x05, "rlc (iy+o),l"},	{0x06, "rlc (iy+o)"},	{0x07, "rlc (iy+o),a"},	{0x08, "rrc (iy+o),b"},	{0x09, "rrc (iy+o),c"},	{0x0A, "rrc (iy+o),d"},	{0x0B, "rrc (iy+o),e"},	{0x0C, "rrc (iy+o),h"},	{0x0D, "rrc (iy+o),l"},	{0x0E, "rrc (iy+o)"},	{0x0F, "rrc (iy+o),a"},
{0x10, "rl (iy+o),b"},	{0x11, "rl (iy+o),c"},	{0x12, "rl (iy+o),d"},	{0x13, "rl (iy+o),e"},	{0x14, "rl (iy+o),h"},	{0x15, "rl (iy+o),l"},	{0x16, "rl (iy+o)"},	{0x17, "rl (iy+o),a"},	{0x18, "rr (iy+o),b"},	{0x19, "rr (iy+o),c"},	{0x1A, "rr (iy+o),d"},	{0x1B, "rr (iy+o),e"},	{0x1C, "rr (iy+o),h"},	{0x1D, "rr (iy+o),l"},	{0x1E, "rr (iy+o)"},	{0x1F, "rr (iy+o),a"},
{0x20, "sla (iy+o),b"},	{0x21, "sla (iy+o),c"},	{0x22, "sla (iy+o),d"},	{0x23, "sla (iy+o),e"},	{0x24, "sla (iy+o),h"},	{0x25, "sla (iy+o),l"},	{0x26, "sla (iy+o)"},	{0x27, "sla (iy+o),a"},	{0x28, "sra (iy+o),b"},	{0x29, "sra (iy+o),c"},	{0x2A, "sra (iy+o),d"},	{0x2B, "sra (iy+o),e"},	{0x2C, "sra (iy+o),h"},	{0x2D, "sra (iy+o),l"},	{0x2E, "sra (iy+o)"},	{0x2F, "sra (iy+o),a"},
{0x30, "sll (iy+o),b"},	{0x31, "sll (iy+o),c"},	{0x32, "sll (iy+o),d"},	{0x33, "sll (iy+o),e"},	{0x34, "sll (iy+o),h"},	{0x35, "sll (iy+o),l"},	{0x36, "sll (iy+o)"},	{0x37, "sll (iy+o),a"},	{0x38, "srl (iy+o),b"},	{0x39, "srl (iy+o),c"},	{0x3A, "srl (iy+o),d"},	{0x3B, "srl (iy+o),e"},	{0x3C, "srl (iy+o),h"},	{0x3D, "srl (iy+o),l"},	{0x3E, "srl (iy+o)"},	{0x3F, "srl (iy+o),a"},
{0x40, "bit 0,(iy+o)"},	{0x41, "bit 0,(iy+o)"},	{0x42, "bit 0,(iy+o)"},	{0x43, "bit 0,(iy+o)"},	{0x44, "bit 0,(iy+o)"},	{0x45, "bit 0,(iy+o)"},	{0x46, "bit 0,(iy+o)"},	{0x47, "bit 0,(iy+o)"},	{0x48, "bit 1,(iy+o)"},	{0x49, "bit 1,(iy+o)"},	{0x4A, "bit 1,(iy+o)"},	{0x4B, "bit 1,(iy+o)"},	{0x4C, "bit 1,(iy+o)"},	{0x4D, "bit 1,(iy+o)"},	{0x4E, "bit 1,(iy+o)"},	{0x4F, "bit 1,(iy+o)"},
{0x50, "bit 2,(iy+o)"},	{0x51, "bit 2,(iy+o)"},	{0x52, "bit 2,(iy+o)"},	{0x53, "bit 2,(iy+o)"},	{0x54, "bit 2,(iy+o)"},	{0x55, "bit 2,(iy+o)"},	{0x56, "bit 2,(iy+o)"},	{0x57, "bit 2,(iy+o)"},	{0x58, "bit 3,(iy+o)"},	{0x59, "bit 3,(iy+o)"},	{0x5A, "bit 3,(iy+o)"},	{0x5B, "bit 3,(iy+o)"},	{0x5C, "bit 3,(iy+o)"},	{0x5D, "bit 3,(iy+o)"},	{0x5E, "bit 3,(iy+o)"},	{0x5F, "bit 3,(iy+o)"},
{0x60, "bit 4,(iy+o)"},	{0x61, "bit 4,(iy+o)"},	{0x62, "bit 4,(iy+o)"},	{0x63, "bit 4,(iy+o)"},	{0x64, "bit 4,(iy+o)"},	{0x65, "bit 4,(iy+o)"},	{0x66, "bit 4,(iy+o)"},	{0x67, "bit 4,(iy+o)"},	{0x68, "bit 5,(iy+o)"},	{0x69, "bit 5,(iy+o)"},	{0x6A, "bit 5,(iy+o)"},	{0x6B, "bit 5,(iy+o)"},	{0x6C, "bit 5,(iy+o)"},	{0x6D, "bit 5,(iy+o)"},	{0x6E, "bit 5,(iy+o)"},	{0x6F, "bit 5,(iy+o)"},
{0x70, "bit 6,(iy+o)"},	{0x71, "bit 6,(iy+o)"},	{0x72, "bit 6,(iy+o)"},	{0x73, "bit 6,(iy+o)"},	{0x74, "bit 6,(iy+o)"},	{0x75, "bit 6,(iy+o)"},	{0x76, "bit 6,(iy+o)"},	{0x77, "bit 6,(iy+o)"},	{0x78, "bit 7,(iy+o)"},	{0x79, "bit 7,(iy+o)"},	{0x7A, "bit 7,(iy+o)"},	{0x7B, "bit 7,(iy+o)"},	{0x7C, "bit 7,(iy+o)"},	{0x7D, "bit 7,(iy+o)"},	{0x7E, "bit 7,(iy+o)"},	{0x7F, "bit 7,(iy+o)"},
{0x80, "res 0,(iy+o),b"},	{0x81, "res 0,(iy+o),c"},	{0x82, "res 0,(iy+o),d"},	{0x83, "res 0,(iy+o),e"},	{0x84, "res 0,(iy+o),h"},	{0x85, "res 0,(iy+o),l"},	{0x86, "res 0,(iy+o)"},	{0x87, "res 0,(iy+o),a"},	{0x88, "res 1,(iy+o),b"},	{0x89, "res 1,(iy+o),c"},	{0x8A, "res 1,(iy+o),d"},	{0x8B, "res 1,(iy+o),e"},	{0x8C, "res 1,(iy+o),h"},	{0x8D, "res 1,(iy+o),l"},	{0x8E, "res 1,(iy+o)"},	{0x8F, "res 1,(iy+o),a"},
{0x90, "res 2,(iy+o),b"},	{0x91, "res 2,(iy+o),c"},	{0x92, "res 2,(iy+o),d"},	{0x93, "res 2,(iy+o),e"},	{0x94, "res 2,(iy+o),h"},	{0x95, "res 2,(iy+o),l"},	{0x96, "res 2,(iy+o)"},	{0x97, "res 2,(iy+o),a"},	{0x98, "res 3,(iy+o),b"},	{0x99, "res 3,(iy+o),c"},	{0x9A, "res 3,(iy+o),d"},	{0x9B, "res 3,(iy+o),e"},	{0x9C, "res 3,(iy+o),h"},	{0x9D, "res 3,(iy+o),l"},	{0x9E, "res 3,(iy+o)"},	{0x9F, "res 3,(iy+o),a"},
{0xA0, "res 4,(iy+o),b"},	{0xA1, "res 4,(iy+o),c"},	{0xA2, "res 4,(iy+o),d"},	{0xA3, "res 4,(iy+o),e"},	{0xA4, "res 4,(iy+o),h"},	{0xA5, "res 4,(iy+o),l"},	{0xA6, "res 4,(iy+o)"},	{0xA7, "res 4,(iy+o),a"},	{0xA8, "res 5,(iy+o),b"},	{0xA9, "res 5,(iy+o),c"},	{0xAA, "res 5,(iy+o),d"},	{0xAB, "res 5,(iy+o),e"},	{0xAC, "res 5,(iy+o),h"},	{0xAD, "res 5,(iy+o),l"},	{0xAE, "res 5,(iy+o)"},	{0xAF, "res 5,(iy+o),a"},
{0xB0, "res 6,(iy+o),b"},	{0xB1, "res 6,(iy+o),c"},	{0xB2, "res 6,(iy+o),d"},	{0xB3, "res 6,(iy+o),e"},	{0xB4, "res 6,(iy+o),h"},	{0xB5, "res 6,(iy+o),l"},	{0xB6, "res 6,(iy+o)"},	{0xB7, "res 6,(iy+o),a"},	{0xB8, "res 7,(iy+o),b"},	{0xB9, "res 7,(iy+o),c"},	{0xBA, "res 7,(iy+o),d"},	{0xBB, "res 7,(iy+o),e"},	{0xBC, "res 7,(iy+o),h"},	{0xBD, "res 7,(iy+o),l"},	{0xBE, "res 7,(iy+o)"},	{0xBF, "res 7,(iy+o),a"},
{0xC0, "set 0,(iy+o),b"},	{0xC1, "set 0,(iy+o),c"},	{0xC2, "set 0,(iy+o),d"},	{0xC3, "set 0,(iy+o),e"},	{0xC4, "set 0,(iy+o),h"},	{0xC5, "set 0,(iy+o),l"},	{0xC6, "set 0,(iy+o)"},	{0xC7, "set 0,(iy+o),a"},	{0xC8, "set 1,(iy+o),b"},	{0xC9, "set 1,(iy+o),c"},	{0xCA, "set 1,(iy+o),d"},	{0xCB, "set 1,(iy+o),e"},	{0xCC, "set 1,(iy+o),h"},	{0xCD, "set 1,(iy+o),l"},	{0xCE, "set 1,(iy+o)"},	{0xCF, "set 1,(iy+o),a"},
{0xD0, "set 2,(iy+o),b"},	{0xD1, "set 2,(iy+o),c"},	{0xD2, "set 2,(iy+o),d"},	{0xD3, "set 2,(iy+o),e"},	{0xD4, "set 2,(iy+o),h"},	{0xD5, "set 2,(iy+o),l"},	{0xD6, "set 2,(iy+o)"},	{0xD7, "set 2,(iy+o),a"},	{0xD8, "set 3,(iy+o),b"},	{0xD9, "set 3,(iy+o),c"},	{0xDA, "set 3,(iy+o),d"},	{0xDB, "set 3,(iy+o),e"},	{0xDC, "set 3,(iy+o),h"},	{0xDD, "set 3,(iy+o),l"},	{0xDE, "set 3,(iy+o)"},	{0xDF, "set 3,(iy+o),a"},
{0xE0, "set 4,(iy+o),b"},	{0xE1, "set 4,(iy+o),c"},	{0xE2, "set 4,(iy+o),d"},	{0xE3, "set 4,(iy+o),e"},	{0xE4, "set 4,(iy+o),h"},	{0xE5, "set 4,(iy+o),l"},	{0xE6, "set 4,(iy+o)"},	{0xE7, "set 4,(iy+o),a"},	{0xE8, "set 5,(iy+o),b"},	{0xE9, "set 5,(iy+o),c"},	{0xEA, "set 5,(iy+o),d"},	{0xEB, "set 5,(iy+o),e"},	{0xEC, "set 5,(iy+o),h"},	{0xED, "set 5,(iy+o),l"},	{0xEE, "set 5,(iy+o)"},	{0xEF, "set 5,(iy+o),a"},
{0xF0, "set 6,(iy+o),b"},	{0xF1, "set 6,(iy+o),c"},	{0xF2, "set 6,(iy+o),d"},	{0xF3, "set 6,(iy+o),e"},	{0xF4, "set 6,(iy+o),h"},	{0xF5, "set 6,(iy+o),l"},	{0xF6, "set 6,(iy+o)"},	{0xF7, "set 6,(iy+o),a"},	{0xF8, "set 7,(iy+o),b"},	{0xF9, "set 7,(iy+o),c"},	{0xFA, "set 7,(iy+o),d"},	{0xFB, "set 7,(iy+o),e"},	{0xFC, "set 7,(iy+o),h"},	{0xFD, "set 7,(iy+o),l"},	{0xFE, "set 7,(iy+o)"},	{0xFF, "set 7,(iy+o),a"}
};




std::string byteToHexString(BYTE b) {
    unsigned char h[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string s;
    s += h[b >> 4];
    s += h[b & 0x0F];
    return s;
}

std::string intToStr(i8 n) {
    std::stringstream ss;
    ss << n; // Se convierte el entero a hexadecimal con cuatro dígitos
    return ss.str();
}


// actualiza asm_instruccion y asm_opcodes
void Z80:: decodeInstructionAtPC() {
	asm_opcodes = "";
	asm_instruccion = "???";
	u16 opcodes;
    BYTE opcode;
	SBYTE offset;
	size_t posparam;
	u16 pc = regPC.w;
	
	memoria->readByte(pc++, &opcode);
	asm_opcodes = byteToHexString(opcode);
	
	if (hmInstrucciones.count(opcode) > 0) {
		asm_instruccion = hmInstrucciones.at(opcode); //hmInstrucciones[opcode];
	}
	else {
		// dos bytes
		opcodes = opcode;
		memoria->readByte(pc++, &opcode);
		opcodes = (opcodes << 8) | opcode; 
		asm_opcodes += byteToHexString(opcode);
		
		if (hmInstrucciones.count(opcodes) > 0) {
			asm_instruccion = hmInstrucciones.at(opcode);
		}
		else if (opcodes == 0xDDCB || opcodes == 0xFDCB) {
			// leer desplazamiento
			memoria->readByte(pc++, &opcode);
			asm_opcodes += byteToHexString(opcode);
			offset = (SBYTE) opcode;
			// leer el ultimo byte
			memoria->readByte(pc--, &opcode);
			asm_opcodes += byteToHexString(opcode);
			
			if (opcodes == 0xDDCB) {
				asm_instruccion = hmInstruccionesDDCB.at(opcode);
			}
			else { // 0xFDCB
				asm_instruccion = hmInstruccionesFDCB.at(opcode);
			}
			posparam = asm_instruccion.find("+o");
			std::string s = std::to_string(offset);
			if (offset >= 0) asm_instruccion.replace(posparam+1, 1, s);
			else asm_instruccion.replace(posparam, 2, s); // sustituye tambien el +
			
			return;
		}
	}
    if ((posparam = asm_instruccion.find("nn")) != std::string::npos) {
        WORD w;
        memoria->readWord(pc, &w);
        std::string s = byteToHexString(w.b.h) + byteToHexString(w.b.l);
        asm_opcodes += byteToHexString(w.b.l) + byteToHexString(w.b.h);
        asm_instruccion.replace(posparam, 2, s);
    }	
    else if ((posparam = asm_instruccion.find(",o")) != std::string::npos) { // "jr condic,o" o "ix+o"
        memoria->readByte(pc++, &opcode);
        asm_opcodes += byteToHexString(opcode);
        offset = (i8) opcode;
        std::string s = std::to_string(offset);
        if (offset >= 0)  s = "+" + s;
        asm_instruccion.replace(posparam+1, 1, s);
    }
    else if ((posparam = asm_instruccion.find("+o")) != std::string::npos) {
        memoria->readByte(pc++, &opcode);
        asm_opcodes += byteToHexString(opcode);
        offset = (i8) opcode;
        std::string s = std::to_string(offset);
        if (offset >= 0) asm_instruccion.replace(posparam+1, 1, s);
        else             asm_instruccion.replace(posparam, 2, s);
    }
	else if ((posparam = asm_instruccion.find(" o")) != std::string::npos) {
        memoria->readByte(pc++, &opcode);
        asm_opcodes += byteToHexString(opcode);
        offset = (i8) opcode;
        std::string s = std::to_string(offset);
        if (offset >= 0)  s = "+" + s;
        asm_instruccion.replace(posparam+1, 1, s);
    }
	else if ((posparam = asm_instruccion.find(" n")) != std::string::npos  
            //&&  asm_instruccion.find("jr") == std::string::npos  
            //&&  asm_instruccion.find("jp") == std::string::npos
            &&  asm_instruccion.find("nc") == std::string::npos
            &&  asm_instruccion.find("nz") == std::string::npos
            ) {
        memoria->readByte(pc++, &opcode);
        std::string s = byteToHexString(opcode);
        asm_opcodes += s;
        asm_instruccion.replace(posparam+1, 1, s);
    }
    if ((posparam = asm_instruccion.find(",n")) != std::string::npos) {
        memoria->readByte(pc, &opcode);
        std::string s = byteToHexString(opcode);
        asm_opcodes += s;
        asm_instruccion.replace(posparam+1, 1, s);
    }
}


std::string& Z80:: getInstructionOpcodes() {
	if (lastPC != PC)  decodeInstructionAtPC();
	
	lastPC = PC;
    return asm_opcodes;
}

std::string& Z80:: getInstructionAsm() {
	if (lastPC != PC)  decodeInstructionAtPC();

	lastPC = PC;
    return asm_instruccion;
}


void Z80:: printInstructionBytes() {
	if (lastPC != PC)  decodeInstructionAtPC();
	
    printf("%04X: ", PC);
    printf("%-8s ", asm_opcodes.c_str());
}

void Z80:: printInstructionASM() {
	if (lastPC != PC)  decodeInstructionAtPC();

    printf("%04X: ", PC);
    printf("%-13s", asm_instruccion.c_str());
}


//-----------------------------------------------------------------
#endif