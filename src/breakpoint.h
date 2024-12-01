#pragma once


#include <list>
#include "tipos.h"
#include "memory.h"


// r/w memoria
void bp_add_read(u16 dir);
void bp_add_write(u16 dir);
// i/o puertos
void bp_add_in(u16 puerto);
void bp_add_out(u16 puerto);
// pc
void bp_add_pc(u16 pc);

bool bp_test(std::list<u16>& lista, u16 dato);

void bp_test_in(u16 puerto);
void bp_test_out(u16 puerto);

bool bp_test_write(u16 dir);
void bp_test_read(u16 dir);

bool bp_test_pc(u16 pc);

bool bp_paso_a_paso();

void bp_preguntar(u16 pc, Memory* mem);

void bp_setPrint(bool b);
bool bp_getPrint();
