#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"

// Prototipos das funcoes utilitarias

// Utilizadas na funcionalidade [1]
void binarioNaTela(char *arquivo);

char *readCSVLine(char line[], int line_size, FILE *csvFile);

void switchDataRecord(DataRecord *data, int i, char *field);

void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd);

void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares);


// Utilizadas na funcionalidade [3]

void ScanQuoteString(char *str);

int getValue(char *valor);

int checkStringMatch(char *data_string, char *valor_busca);


#endif