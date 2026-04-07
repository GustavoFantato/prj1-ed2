#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "structs.h"

// Protótipos das funções para cada funcionalidade

void createTable(char *arquivoEntrada, char *arquivoSaida);

void listTable(char *arquivoEntrada);

void listTableWhere(char *arquivoEntrada, int n);

void listTableRRN(char *arquivoEntrada, int RRN);

#endif