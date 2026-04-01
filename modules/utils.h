#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"

void binarioNaTela(char *arquivo);

void ScanQuoteString(char *str);

char *readCSVLine(char line[], int line_size, FILE *csvFile);

char *parseCSVField(char *line, int index);

int verifyIfNullField(char *field);

void switchDataRecord(DataRecord *data, int i, char *field);

void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd);

void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares);




#endif