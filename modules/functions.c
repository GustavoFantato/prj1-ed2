#include "functions.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LINE_SIZE 1200 


/*
# FUNCIONALIDADE [1] - Criacao do arquivo binario a partir do CSV #

-> Leitura de varios registros obtidos a partir de um arquivo de entrada .csv e armazenamento desses registros em um arquivo de dados binário
*/
void createTable(char *arquivoEntrada, char *arquivoSaida) {

    // Abertura dos arquivos
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

    char line[LINE_SIZE]; // Buffer para leitura de cada linha do CSV

    // Inicializacao dos campos do cabecalho
    HeaderRecord header;
    header.status = '0';
    header.topo = -1;
    header.proxRRN = 0;
    header.nroEstacoes = 0;
    header.nroParesEstacao = 0;

    // Escrita dos campos do cabecalho
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    // A primeira linha do arquivo deve ser ignorada, pois eh o cabecalho do CSV
    readCSVLine(line, LINE_SIZE, csvFile); 
    
    // Inicializacao das variaveis para controle de quantidade de estacoes diferentes e pares
    char **diffStationNames = NULL; // Ponteiro duplo pois trata-se de um array dinamico de strings
    Par *listPares = NULL;

    // readCSVLine eh uma funcao que le a linha do CSV. Retorna NULL quando chega ao fim do arquivo
    while (readCSVLine(line, LINE_SIZE, csvFile) != NULL)   {

        // Inicializacao dos campos do registro de dados
        DataRecord data;
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

        // Cria-se copias da linha, para evitar que a strsep modifique o buffer original
        char *lineCopy = strdup(line);
        char *ptr = lineCopy; // Ponteiro para iterar sobre a linha

        for (int i = 0; i < REGISTER_QTD; i++){ // Loop para ler cada field da linha
            char *field = strsep(&ptr, ",");
            if (field != NULL) {
                field[strcspn(field, "\r\n")] = '\0'; // encontra o indice onde possa haver um \r ou \n e substitui por \0
                switchDataRecord(&data, i, field); // switch para associar o indice com o campo lido no momento, atribuindo esse valor a struct do datarecord
            }        
        }

        // Escrita do registro de dados no arquivo bin
        fwrite(&data.removido, sizeof(char), 1, binFile);
        fwrite(&data.proximo, sizeof(int), 1, binFile);
        fwrite(&data.codEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinha, sizeof(int), 1, binFile);
        fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
  

        // Nao se deve escrever nada caso o tamanho seja 0. Sera tratado, no fim, na escrita do lixo com '$'
        fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if(data.tamNomeEstacao > 0) {
            fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        }
        
        fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if(data.tamNomeLinha > 0) {
            fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        }

        // Tratamento do lixo, escrevendo '$' nos bytes nao utilizados
        int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        for(int i = 0; i < garbageBytes; i++){
            fputc('$', binFile); 
        }

        // Verificacao das estacoes e pares diferentes, para controle do cabecalho
        verifyIfDiffStation(data.nomeEstacao, &diffStationNames, &header.nroEstacoes);
        if(data.codProxEstacao != -1){
        verifyIfDiffPair(data.codEstacao, data.codProxEstacao, &listPares, &header.nroParesEstacao);
        }
        
        // Soma o proximo RRN, porque o registro foi escrito no arquivo 
        header.proxRRN++;

        // Liberacao das memorias alocadas
        free(data.nomeEstacao); 
        if (data.nomeLinha != NULL) free(data.nomeLinha);
        free(lineCopy);  
    }

    // Atualizacao do header
    header.status = '1';

    // Posiciona o ponteiro no inicio para atualizar o header, ja que a quantidade de estacoes e pares diferentes so foi definida apos a leitura de todo o CSV
    fseek(binFile, 0, SEEK_SET); 
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    // Liberacao das memorias alocadas para controle de estacoes e pares
    for (int i = 0; i < header.nroEstacoes; i++) {
        free(diffStationNames[i]); // cada string do vetor
    }
    free(diffStationNames); // o vetor em si
    if (listPares != NULL) free(listPares);

    // Fechamento dos arquivos
    fclose(binFile);
    fclose(csvFile);   

    // Print da soma dos binarios
    binarioNaTela(arquivoSaida);
}

/*
# FUNCIONALIDADE [2] - Listagem de todos os registros nao removidos #

-> Recuperacao dos dados de TODOS os registros armazenados em um arquivo de dados de entrada, mostrando os dados de forma organizada na saída padrão (dada graficamente no arquivo de instrucoes do projeto) para permitir a distincao dos campos e registros. 
-> Registros marcados como logicamente removidos nao devem ser exibidos 

*/

void listTable(char *arquivoEntrada) {
    
    // Abertura do arquivo binario para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Leitura do status para verificar se o arquivo eh consistente
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Posiciona o ponteiro apos o header, no inicio dos registros
    fseek(binFile, 16, SEEK_CUR);

    int foundReg = 0; // bool para controle de registro encontrado ou nao
    char removed; 

    // Enquanto nao chegar ao fim do arquivo, le-se o campo de removido para verificar se o registro esta ativo ou nao.
    while (fread(&removed, sizeof(char), 1, binFile) == 1) { 
        
        // Se == 1, eh porque foi removido
        if (removed == '1') {
            fseek(binFile, 79, SEEK_CUR); 
            continue; // Pula para o proximo registro
        }

        // Criacao do data record
        foundReg = 1; // Registro encontrado, sera utilizado posteriormente
        
        // Inicializacao do data record, atribuindo o valor do campo removido que ja foi lido
        DataRecord data;
        data.removido = removed; 
        
        // Leitura dos demais campos, seguindo a ordem definida no projeto
        fread(&data.proximo, sizeof(int), 1, binFile);
        fread(&data.codEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinha, sizeof(int), 1, binFile);
        fread(&data.codProxEstacao, sizeof(int), 1, binFile);
        fread(&data.distProxEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fread(&data.codEstIntegra, sizeof(int), 1, binFile);

        // Se tamanho eh 0, nao deve ler o nomeEstacao, apenas atribuir NULL
        fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if (data.tamNomeEstacao > 0) {
            // Aloca memoria ao nomeEstacao de acordo com o tamanho lido + 1, para o '\0' do final
            data.nomeEstacao = malloc((data.tamNomeEstacao + 1) * sizeof(char));
            fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
            data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
        } else {
            data.nomeEstacao = NULL;
        }

        // Se tamanho eh 0, nao deve ler o nomeLinha, apenas atribuir NULL
        fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if (data.tamNomeLinha > 0) {
            // Aloca memoria o nomeLinha de acordo com o tamanho lido + 1, para o '\0' do final
            data.nomeLinha = malloc((data.tamNomeLinha + 1) * sizeof(char));
            fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
            data.nomeLinha[data.tamNomeLinha] = '\0';
        } else {
            data.nomeLinha = NULL;
        }

        // Guardar a quantidade de bytes de lixo
        int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR); // Posiciona o ponteiro para o inicio do proximo registro, pulando o lixo

        // Prints dos campos do registro
        printf("%d ", data.codEstacao);
        if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
        if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
        if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
        if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
        if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
        if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
        if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);

        // Liberacao das memorias alocadas dos campos variaveis
        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    }

    // Se o registros nao foi encontrado, printa a mensagem
    if (!foundReg) {
        printf("Registro inexistente.\n");
    }

    // Fechamento do arquivo
    fclose(binFile);
}

/*
# FUNCIONALIDADE [3] - Recuperacao dos dados dos registros que satisfacam um criteiro de busca, determinado pelo usuario
-> Qualquer campo pode ser usado como forma de busca
-> A busca deve ser feita considerando um ou mais campos 
-> Essa funcao pode retornar 0 registros (quando nenhum registro satisfaz o criterio), 1 registro (quando apenas um satisfaz) ou mais registros (quando um ou mais registros satisfazem o criterio)
-> Registros marcados como logicamente removidos nao devem ser exibidos
*/


void listTableWhere(char *arquivoEntrada, int n) {
    
    // Abertura do arquivo binario para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verificacao se o status do arquivo eh '1' (consistente)
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Loop para ler os n casos de busca
    for (int i = 0; i < n; i++) {
        
        // m eh o numero de campos a serem comparados para a busca
        int m;
        scanf("%d", &m); 

        // Campos e valores a serem comparados 
        char campos[8][30];
        char valores[8][100];
        
        // Loop para ler os campos e valores, de acordo com o 'm' lido 
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j]); 

            // Se o campo for nomeEstacao ou nomeLinha, utiliza-se a funcao dada ScanQuoteString()
            if (strcmp(campos[j], "nomeEstacao") == 0 || strcmp(campos[j], "nomeLinha") == 0) {
                ScanQuoteString(valores[j]); 
            } 
            else { 
                scanf("%s", valores[j]);

                // Se nulo, coloca a string como vazia para facilitar a comparacao mais pra frente
                if (strcmp(valores[j], "NULO") == 0) {
                    strcpy(valores[j], "");
                }
            }
        }

        // Posiciona o ponteiro no inicio dos registros, apos o header
        fseek(binFile, HEADER_SIZE, SEEK_SET);


        int foundReg = 0; // Bool para controle de registro encontrado ou nao
        char removed; // armazenar o campo se o registro foi removido ou nao


        // Enquanto nao chegar ao fim do arquivo, le-se o campo de removido para verificar se o registro esta ativo ou nao.    
        while (fread(&removed, sizeof(char), 1, binFile) == 1) {
            
            // Se o campo foi removido pula para o proximo registro
            if (removed == '1') {
                fseek(binFile, 79, SEEK_CUR);
                continue;
            }

            // Criacao do data record, atribuindo o valor do campo removido que ja foi lido
            DataRecord data;
            data.removido = removed;

            // Inicializacao dos demais campos do data record, lendo os valores do arquivo binario seguindo a ordem definida no projeto
            fread(&data.proximo, sizeof(int), 1, binFile);
            fread(&data.codEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinha, sizeof(int), 1, binFile);
            fread(&data.codProxEstacao, sizeof(int), 1, binFile);
            fread(&data.distProxEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
            fread(&data.codEstIntegra, sizeof(int), 1, binFile);

            // Se tamanho eh 0, nao deve ler o nomeEstacao, apenas atribuir NULL
            fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
            if (data.tamNomeEstacao > 0) {
                // Aloca memoria ao nomeEstacao de acordo com o tamanho lido + 1, para o '\0' do final
                data.nomeEstacao = malloc((data.tamNomeEstacao + 1) * sizeof(char));
                fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
                data.nomeEstacao[data.tamNomeEstacao] = '\0';
            } else {
                data.nomeEstacao = NULL;
            }

            // Se tamanho eh 0, nao deve ler o nomeLinha, apenas atribuir NULL
            fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
            if (data.tamNomeLinha > 0) {
                // Aloca memoria o nomeLinha de acordo com o tamanho lido + 1, para o '\0' do final
                data.nomeLinha = malloc((data.tamNomeLinha + 1) * sizeof(char));
                fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
                data.nomeLinha[data.tamNomeLinha] = '\0';
            } else {
                data.nomeLinha = NULL;
            }

            // Guardar a quantidade de bytes de lixo, para posicionar o ponteiro no inicio do proximo registro
            int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
            fseek(binFile, garbageBytes, SEEK_CUR);

            int match = 1; 

            // Loop para comparar os campos lidos com os campos de busca, verificando se o registro atual do arquivo binario corresponde aos criterios de busca
            for (int f = 0; f < m; f++) {
                
                if (strcmp(campos[f], "codEstacao") == 0) {
                    if (data.codEstacao != getValue(valores[f])) match = 0;
                } 
                else if (strcmp(campos[f], "codLinha") == 0) {
                    if (data.codLinha != getValue(valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "codProxEstacao") == 0) {
                    if (data.codProxEstacao != getValue(valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "distProxEstacao") == 0) {
                    if (data.distProxEstacao != getValue(valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "codLinhaIntegra") == 0) {
                    if (data.codLinhaIntegra != getValue(valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "codEstIntegra") == 0) {
                    if (data.codEstIntegra != getValue(valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "nomeEstacao") == 0) {
                    if (!checkStringMatch(data.nomeEstacao, valores[f])) match = 0;
                }
                else if (strcmp(campos[f], "nomeLinha") == 0) {
                    if (!checkStringMatch(data.nomeLinha, valores[f])) match = 0;
                }
                
                // Se nao der match, quebra o loop, evitando comparacoes desnecessarias
                if (match == 0) break; 
            }

            // Se deu match, registro foi encontrado, printa os campos do registro
            if (match == 1) {
                foundReg = 1;
                
                printf("%d ", data.codEstacao);
                if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
                if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
                if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
                if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
                if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
                if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
                if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);
            }

            // Liberacao das memorias dos campos variaveis
            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);
        }

        // Se nao encontrou registro, printa a mensagem
        if (!foundReg) {
            printf("Registro inexistente.\n");
        }

        // Formatacao adequada de print do '\n' para o runCodes
        if (!(i == n-1)){
            printf("\n");
        }
    }

    // Fecha o arquivo
    fclose(binFile);
}

/*
# FUNCIONALIDADE [4] - Listagem de um registro a partir do RRN #
-> Recuperacao dos dados de um registro de um arquivo a partir da identificacao do RRN do registro desejado.
-> Registros marcados como logicamente removidos nao devem ser exibidos
*/

void listTableRRN(char *arquivoEntrada, int RRN) {
    
    // Abertura do arquivo binario para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verificacao se o status do arquivo eh '1' (consistente)
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Calculo do byte offset a partir do RRN, para colocar o ponteiro no inicio do registro a ser buscado
    int byteOffset = HEADER_SIZE + (RRN * REGISTER_SIZE);
    fseek(binFile, byteOffset, SEEK_SET);

    // Leitura e verificacao se o registro foi removido ou nao
    char removed;
    if (fread(&removed, sizeof(char), 1, binFile) != 1 || removed == '1') {
        printf("Registro inexistente.\n");
        fclose(binFile);
        return;
    }

    // Criacao do data record, atribuindo o valor do campo removido que ja foi lido
    DataRecord data;
    data.removido = removed; 
    
    // Leitura dos demais campos, seguindo a ordem definida no projeto
    fread(&data.proximo, sizeof(int), 1, binFile);
    fread(&data.codEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinha, sizeof(int), 1, binFile);
    fread(&data.codProxEstacao, sizeof(int), 1, binFile);
    fread(&data.distProxEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
    fread(&data.codEstIntegra, sizeof(int), 1, binFile);

    // Se tamanho eh 0, nao deve ler o nomeEstacao, apenas atribuir NULL
    fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
    if (data.tamNomeEstacao > 0) {
        // Aloca memoria ao nomeEstacao de acordo com o tamanho lido + 1, para o '\0' do final
        data.nomeEstacao = malloc((data.tamNomeEstacao + 1) * sizeof(char));
        fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
    } else {
        data.nomeEstacao = NULL;
    }

    // Se tamanho eh 0, nao deve ler o nomeLinha, apenas atribuir NULL
    fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
    if (data.tamNomeLinha > 0) {
        // Aloca memoria o nomeLinha de acordo com o tamanho lido + 1, para o '\0' do final
        data.nomeLinha = malloc((data.tamNomeLinha + 1) * sizeof(char));
        fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        data.nomeLinha[data.tamNomeLinha] = '\0';
    } else {
        data.nomeLinha = NULL;
    }

    // Print dos campos do registro encontrado
    printf("%d ", data.codEstacao);
    if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
    if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
    if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
    if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
    if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
    if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
    if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);

    // Liberacao das memorias alocadas dos campos variaveis
    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
    if (data.nomeLinha != NULL) free(data.nomeLinha);

    // Fecha o arquivo
    fclose(binFile);
}