
#include <iostream>
#include <stdio.h>
#include "log.h"

void pausa() {
    printf("<pausa>");
    std::cin.ignore();
}

bool print_io = false;
int print_traza = 0;

bool log_teclaPulsada = false;
bool log_instAlcanzada = false;
