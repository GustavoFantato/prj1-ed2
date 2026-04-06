#include "functions.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LINE_SIZE 1200 

// -----------------------------------------------------------------------------
// FUNCIONALIDADE [1] - createTable
// Conversão de CSV para arquivo binário.
// Mapeamento Físico: 
// - Cabeçalho (HeaderRecord): 17 bytes iniciais.
// - Registros (DataRecord): Tamanho fixo de 80 bytes (dados dinâmicos + padding de '$').
// -----------------------------------------------------------------------------
void createTable(char *arquivoEntrada, char *arquivoSaida) {

    FILE *csvFile = fopen(arquivoEntrada, "r");
    if (csvFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *binFile = fopen(arquivoSaida, "wb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(csvFile);
        return;
    }

    char line[LINE_SIZE];

    // Inicialização do cabeçalho no disco (Byte Offset 0 a 16)
    // status '0' indica arquivo inconsistente durante a escrita em disco.
    HeaderRecord header;
    header.status = '0';
    header.topo = -1;
    header.proxRRN = 0;
    header.nroEstacoes = 0;
    header.nroParesEstacao = 0;

    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    readCSVLine(line, LINE_SIZE, csvFile); // Pula linha de metadados do CSV
    
    char **diffStationNames = NULL; 
    Par *listPares = NULL; 

    // Processamento sequencial e I/O dos registros
    while (readCSVLine(line, LINE_SIZE, csvFile) != NULL) {

        DataRecord data;
        memset(&data, 0, sizeof(DataRecord)); 

        data.removido = '0';
        data.proximo = -1;
        data.codEstacao = -1;
        data.codLinha = -1;
        data.codProxEstacao = -1;
        data.distProxEstacao = -1;
        data.codLinhaIntegra = -1;
        data.codEstIntegra = -1;
        data.tamNomeEstacao = 0;
        data.tamNomeLinha = 0;
        data.nomeEstacao = NULL;
        data.nomeLinha = NULL;

        char *lineCopy = strdup(line);
        char *ptr = lineCopy;

        for (int i = 0; i < REGISTER_QTD; i++){ 
            char *field = strsep(&ptr, ",");
            if (field != NULL) {
                field[strcspn(field, "\r\n")] = '\0';
                switchDataRecord(&data, i, field);
            }        
        }

        // Escrita sequencial dos campos fixos (29 bytes do registro físico)
        fwrite(&data.removido, sizeof(char), 1, binFile);
        fwrite(&data.proximo, sizeof(int), 1, binFile);
        fwrite(&data.codEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinha, sizeof(int), 1, binFile);
        fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
        
        // Escrita dos campos de tamanho variável (strings prefixadas por seu tamanho)
        fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if(data.tamNomeEstacao > 0) {
            fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        }
        
        fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if(data.tamNomeLinha > 0) {
            fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        }

        // Padding/Fragmentação interna: Preenchimento com '$' para garantir blocos exatos de 80 bytes
        int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        for(int i = 0; i < garbageBytes; i++){
            fputc('$', binFile); 
        }

        verifyIfDiffStation(data.nomeEstacao, &diffStationNames, &header.nroEstacoes);
        verifyIfDiffPair(data.codEstacao, data.codProxEstacao, &listPares, &header.nroParesEstacao);
        header.proxRRN++;

        free(data.nomeEstacao); 
        if (data.nomeLinha != NULL) free(data.nomeLinha);
        free(lineCopy);  
    }

    // Flush de metadados: Status '1' garante integridade do arquivo binário fechado
    header.status = '1';
    fseek(binFile, 0, SEEK_SET); 
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    for (int i = 0; i < header.nroEstacoes; i++) {
        free(diffStationNames[i]);
    }
    free(diffStationNames);
    if (listPares != NULL) free(listPares);

    fclose(binFile);
    fclose(csvFile);   

    binarioNaTela(arquivoSaida);
}

// -----------------------------------------------------------------------------
// FUNCIONALIDADE [2] - listTable
// Varredura sequencial (Full Table Scan) do arquivo binário.
// Itera fisicamente bloco a bloco de 80 bytes (REGISTER_SIZE).
// -----------------------------------------------------------------------------
void listTable(char *arquivoEntrada) {
    
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Byte Offset 17: Pula os 16 bytes restantes do header e posiciona o ponteiro nos dados
    fseek(binFile, 16, SEEK_CUR);

    int encontrouRegistro = 0; 
    char removido;

    // Leitura contínua até EOF
    while (fread(&removido, sizeof(char), 1, binFile) == 1) {
        
        // Tratamento de fragmentação: Avança o ponteiro de leitura se o registro foi removido
        if (removido == '1') {
            fseek(binFile, 79, SEEK_CUR); // Pula os 79 bytes restantes do bloco físico
            continue; 
        }

        encontrouRegistro = 1;
        DataRecord data;
        data.removido = removido; 
        
        fread(&data.proximo, sizeof(int), 1, binFile);
        fread(&data.codEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinha, sizeof(int), 1, binFile);
        fread(&data.codProxEstacao, sizeof(int), 1, binFile);
        fread(&data.distProxEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fread(&data.codEstIntegra, sizeof(int), 1, binFile);

        fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if (data.tamNomeEstacao > 0) {
            data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
            fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
            data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
        } else {
            data.nomeEstacao = NULL;
        }

        fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if (data.tamNomeLinha > 0) {
            data.nomeLinha = (char *)malloc((data.tamNomeLinha + 1) * sizeof(char));
            fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
            data.nomeLinha[data.tamNomeLinha] = '\0';
        } else {
            data.nomeLinha = NULL;
        }

        // Pulo do lixo de memória para garantir alinhamento do ponteiro para a próxima iteração
        int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR); 

        printf("%d ", data.codEstacao);
        if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
        if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
        if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
        if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
        if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
        if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
        if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);

        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    }

    if (!encontrouRegistro) {
        printf("Registro inexistente.\n");
    }

    fclose(binFile);
}

// -----------------------------------------------------------------------------
// FUNCIONALIDADE [3] - listTableWhere
// Busca parametrizada O(n) iterando sobre múltiplos critérios.
// Aplica full table scans sucessivos retrocedendo o ponteiro do disco a cada loop.
// -----------------------------------------------------------------------------
void listTableWhere(char *arquivoEntrada, int n) {
    
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    for (int i = 0; i < n; i++) {
        
        int m;
        scanf("%d", &m); 

        char campos[8][30];
        char valores[8][100];

        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j]); 
            ScanQuoteString(valores[j]); 
        }

        // Rewind físico: Reposiciona a agulha de leitura logo após os metadados
        fseek(binFile, HEADER_SIZE, SEEK_SET);

        int encontrouRegistro = 0;
        char removido;

        while (fread(&removido, sizeof(char), 1, binFile) == 1) {
            
            if (removido == '1') {
                fseek(binFile, 79, SEEK_CUR);
                continue;
            }

            DataRecord data;
            data.removido = removido;

            fread(&data.proximo, sizeof(int), 1, binFile);
            fread(&data.codEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinha, sizeof(int), 1, binFile);
            fread(&data.codProxEstacao, sizeof(int), 1, binFile);
            fread(&data.distProxEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
            fread(&data.codEstIntegra, sizeof(int), 1, binFile);

            fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
            if (data.tamNomeEstacao > 0) {
                data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
                fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
                data.nomeEstacao[data.tamNomeEstacao] = '\0';
            } else {
                data.nomeEstacao = NULL;
            }

            fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
            if (data.tamNomeLinha > 0) {
                data.nomeLinha = (char *)malloc((data.tamNomeLinha + 1) * sizeof(char));
                fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
                data.nomeLinha[data.tamNomeLinha] = '\0';
            } else {
                data.nomeLinha = NULL;
            }

            int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
            fseek(binFile, garbageBytes, SEEK_CUR);

            // Validação lógica em ram: Verifica compatibilidade com a query
            int match = 1; 

            for (int f = 0; f < m; f++) {
                
                if (strcmp(campos[f], "codEstacao") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.codEstacao != valBusca) match = 0;
                } 
                else if (strcmp(campos[f], "codLinha") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.codLinha != valBusca) match = 0;
                }
                else if (strcmp(campos[f], "codProxEstacao") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.codProxEstacao != valBusca) match = 0;
                }
                else if (strcmp(campos[f], "distProxEstacao") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.distProxEstacao != valBusca) match = 0;
                }
                else if (strcmp(campos[f], "codLinhaIntegra") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.codLinhaIntegra != valBusca) match = 0;
                }
                else if (strcmp(campos[f], "codEstIntegra") == 0) {
                    int valBusca = (strcmp(valores[f], "") == 0) ? -1 : atoi(valores[f]);
                    if (data.codEstIntegra != valBusca) match = 0;
                }
                else if (strcmp(campos[f], "nomeEstacao") == 0) {
                    if (strcmp(valores[f], "") == 0) { 
                        if (data.nomeEstacao != NULL) match = 0; 
                    } else {
                        if (data.nomeEstacao == NULL || strcmp(data.nomeEstacao, valores[f]) != 0) match = 0;
                    }
                }
                else if (strcmp(campos[f], "nomeLinha") == 0) {
                    if (strcmp(valores[f], "") == 0) {
                        if (data.nomeLinha != NULL) match = 0;
                    } else {
                        if (data.nomeLinha == NULL || strcmp(data.nomeLinha, valores[f]) != 0) match = 0;
                    }
                }
                
                if (match == 0) break; 
            }

            if (match == 1) {
                encontrouRegistro = 1;
                
                printf("%d ", data.codEstacao);
                if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
                if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
                if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
                if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
                if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
                if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
                if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);
            }

            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);
        }

        if (!encontrouRegistro) {
            printf("Registro inexistente.\n");
        }

        if (!(i == n-1)){
        printf("\n");
        }
    }

    fclose(binFile);
}

// -----------------------------------------------------------------------------
// FUNCIONALIDADE [4] - listTableRRN
// Recuperação O(1) via Random Access.
// Explora a arquitetura de blocos de tamanho fixo calculando o byte offset exato.
// -----------------------------------------------------------------------------
void listTableRRN(char *arquivoEntrada, int RRN) {
    
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Cálculo Algorítmico do Byte Offset: HEADER (17 bytes) + (RRN * 80 bytes do registro)
    int byteOffset = HEADER_SIZE + (RRN * REGISTER_SIZE);
    fseek(binFile, byteOffset, SEEK_SET);

    char removido;
    if (fread(&removido, sizeof(char), 1, binFile) != 1 || removido == '1') {
        printf("Registro inexistente.\n");
        fclose(binFile);
        return;
    }

    DataRecord data;
    data.removido = removido; 
    
    fread(&data.proximo, sizeof(int), 1, binFile);
    fread(&data.codEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinha, sizeof(int), 1, binFile);
    fread(&data.codProxEstacao, sizeof(int), 1, binFile);
    fread(&data.distProxEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
    fread(&data.codEstIntegra, sizeof(int), 1, binFile);

    fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
    if (data.tamNomeEstacao > 0) {
        data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
        fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
    } else {
        data.nomeEstacao = NULL;
    }

    fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
    if (data.tamNomeLinha > 0) {
        data.nomeLinha = (char *)malloc((data.tamNomeLinha + 1) * sizeof(char));
        fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        data.nomeLinha[data.tamNomeLinha] = '\0';
    } else {
        data.nomeLinha = NULL;
    }

    printf("%d ", data.codEstacao);
    if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
    if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
    if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
    if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
    if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
    if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
    if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);

    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
    if (data.nomeLinha != NULL) free(data.nomeLinha);

    fclose(binFile);
}