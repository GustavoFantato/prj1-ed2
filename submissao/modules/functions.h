#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "structs.h"

// Funcionalidade 1: Importação de dados (CSV -> Binário)
void createTable(char *arquivoEntrada, char *arquivoSaida);

// Funcionalidade 2: Full Table Scan sem filtros (SELECT *)
void listTable(char *arquivoEntrada);

// Funcionalidade 3: Full Table Scan com filtros dinâmicos (SELECT * WHERE)
void listTableWhere(char *arquivoEntrada, int n);

// Funcionalidade 4: Busca direta em O(1) por RRN (SELECT * WHERE id = X)
void listTableRRN(char *arquivoEntrada, int RRN);

#endif