#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"

// Funções fornecidas pelos professores
void binarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

// Manipulação e extração de dados do CSV
char *readCSVLine(char line[], int line_size, FILE *csvFile);
char *parseCSVField(char *line, int index);
int verifyIfNullField(char *field);

// Mapeamento de dados da string para a Struct binária
void switchDataRecord(DataRecord *data, int i, char *field);

// Controle de metadados do Cabeçalho (Valores Únicos)
void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd);
void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares);

#endif