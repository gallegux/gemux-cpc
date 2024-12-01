#include <list>
#include <algorithm> // para find
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include "tipos.h"
#include "memory.h"


std::list<u16> l_ins;
std::list<u16> l_outs;
std::list<u16> l_reads;
std::list<u16> l_writes;
std::list<u16> l_pcs;


bool _bp_paso_a_paso = false;
bool bp_habilitado = true;
bool bp_pintar = false;

bool bp_paso_a_paso() { return _bp_paso_a_paso; }

void bp_add_read(u16 posicion) {
	l_reads.push_back(posicion);
}

void bp_add_write(u16 posicion) {
	l_writes.push_back(posicion);
}

void bp_add_in(u16 puerto) {
	l_ins.push_back(puerto);
}

void bp_add_out(u16 puerto) {
	l_outs.push_back(puerto);
}

void bp_add_pc(u16 pc) {
	l_pcs.push_back(pc);
}


bool bp_test(std::list<u16>& lista, u16 dato) {
	auto pos = std::find(lista.begin(), lista.end(), dato);
	return (pos != lista.end());
}

void bp_test_in(u16 puerto) {
	if (bp_habilitado && bp_test(l_ins, puerto)) _bp_paso_a_paso = true;
}

void bp_test_out(u16 puerto) {
	if (bp_habilitado && bp_test(l_outs, puerto)) _bp_paso_a_paso = true;
}

bool bp_test_write(u16 dir) {
	return (bp_habilitado && bp_test(l_writes, dir));
}

void bp_test_read(u16 dir) {
	if (bp_habilitado && bp_test(l_reads, dir)) _bp_paso_a_paso = true;
}

bool bp_test_pc(u16 pc) {
	return (bp_habilitado && bp_test(l_pcs, pc));
}



void bp_preguntar(u16 pc, Memory* mem) {
	if (_bp_paso_a_paso) {
		printf("[ Run | Paso | Grabar ]  ?  ");
		char caracter;
		std::stringstream  ss;
		ss << "memoria_" << std::hex << std::setw(4) << std::setfill('0') << pc << ".bin";
		std::string result = ss.str();

		std::cin >> caracter;
		switch (caracter) {
			case 'r':
				_bp_paso_a_paso = false;
				bp_pintar = false;
				break;
			case 'p':
				_bp_paso_a_paso = true;
				bp_pintar = true;
				break;
			case 'g':
				mem->grabarRam(result);
				break;
		}
	}
	else if (bp_test_pc(pc)) {
		_bp_paso_a_paso = true;
		bp_preguntar(pc,mem);
	}
}


void bp_setPrint(bool b) {
	bp_pintar = b;
}

bool bp_getPrint() { return bp_pintar; }
