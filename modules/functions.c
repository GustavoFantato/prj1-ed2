#include "functions.h"
#include "utils.h"
#include <string.h>

#define LINE_SIZE 1200 // Tamanho maximo de uma linha do arquivo .csv

// Descricao das funcoes utilizadas no programa // 

/*
# FUNCIONALIDADE [1] - createTable() # 

-> Leitura de varios registros obtidos a partir de um arquivo de entrada .csv (que esta em /data) e armazenamento desses registros em um arquivo de dados binário
-> Deve-se usar a funcao binarioNaTela (dada em utils.h). Ela deve ser usada depois que o arquivo é fechado e antes de terminar a execucao da funcionalidade

ENTRADA: 1 arquivoEntrada.csv e arquivoSaida.bin
 | - arquivoEntrada.csv: contem os valores dos campos dos registros a serem armazenados no arquivo binario
 | - arquivoSaida.bin: arquivo binario gerado conforme as instrucoes dadas no projeto

SAIDA: 
 | Caso tudo ocorra bem: a saida deve ser o arquivo no formato binario usando a funcao fornecida binarioNaTela.
 | Caso ocorra um erro: "Falha no processamento do arquivo."
 
 EXEMPLO DE EXECUCAO:
 ./programaTrab
 1 estacao.csv estacao.bin
*/

    void createTable (char *arquivoEntrada, char *arquivoSaida){

        // Abrir os arquivos de entrada e saida

        FILE *csvFile = fopen(arquivoEntrada, "r"); // Abre para leitura
        if (csvFile == NULL) { // Verifica se abriu corretamente
            printf("Falha no processamento do arquivo.\n");
            return;
        }

        FILE *binFile = fopen(arquivoSaida, "wb"); // Abre para escrita, se ja existe, sobrescreve
        if (binFile == NULL) { // Verifica se abriu corretamente
            printf("Falha no processamento do arquivo.\n");
            fclose(csvFile); // Se o segundo arquivo falhou, temos que fechar o primeiro, ja que ele passou pela verificacao
            return;
        }

        // Leitura do arquivo .csv e escrita no arquivo .bin
        char line[LINE_SIZE];

        // Cria-se a struct do cabecalho e inicializa seus campos
        HeaderRecord header;
        header.status = '0';
        header.topo = -1;
        header.proxRRN = 0;
        header.nroEstacoes = 0;
        header.nroParesEstacao = 0;

        // Escreve o cabecalho inicial no .bin
        fwrite(&header.status, sizeof(char), 1, binFile);
        fwrite(&header.topo, sizeof(int), 1, binFile);
        fwrite(&header.proxRRN, sizeof(int), 1, binFile);
        fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
        fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

        // Leitura das linhas
        readCSVLine(line, LINE_SIZE, csvFile); // Devemos ignorar a primeira linha, ja que se trata do cabecalho do arquivo
        
        char **diffStationNames = NULL; // matriz para armazenar cada nome de estacao 
        Par *listPares = NULL; // struct que armazena os codigos de origem e destino
        

        // int contReg = 0; // DEBUG

        // Leitura e escrita dos multiplos registros 
        while (readCSVLine(line, LINE_SIZE, csvFile) != NULL) {

            DataRecord data; // Criacao da struct do registro de dados
            memset(&data, 0, sizeof(DataRecord)); // Inicializacao da struct de dados com 0 para evitar lixo

            data.removido = '0'; // a principio, nenhum registro esta logicamente removido
            data.proximo = -1; // inicializa-se com -1

            char *lineCopy = strdup(line); // Criar uma copia da linha para evitar modificar o original
            char *ptr = lineCopy; // Este ponteiro vai "caminhar" pela linha

            for (int i = 0; i < REGISTER_QTD; i++){  // loop para ler os fields da linha

                char *field = custom_strsep(&ptr, ",");

                if (field != NULL) {
                // Limpa o \n apenas se for o último campo ou tiver quebras
                field[strcspn(field, "\r\n")] = '\0';
                switchDataRecord(&data, i, field);
                }        
            }

            // Escrita do registro de dados no arquivo .bin
            fwrite(&data.removido, sizeof(char), 1, binFile);
            fwrite(&data.proximo, sizeof(int), 1, binFile);
            fwrite(&data.codEstacao, sizeof(int), 1, binFile);
            fwrite(&data.codLinha, sizeof(int), 1, binFile);
            fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
            fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
            fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
            fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
            fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);

            if(data.tamNomeEstacao > 0){ // Escreve apenas se o tamNomeEstacao nao for 0
            fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
            }
            fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
            if(data.tamNomeLinha > 0) { // Escreve apenas se o tamNomeLinha nao for 0
            fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
            }


            // Calculando os lixos da memoria e escrevendo '$' no lugar
            int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha); // calcula a quantidade de bytes de lixo

            for(int i = 0; i < garbageBytes; i++){
                fputc('$', binFile); 
            }
        

            // Verificacoes das diferentes estacoes e pares

            verifyIfDiffStation(data.nomeEstacao, &diffStationNames, &header.nroEstacoes); // Verifica se eh uma estacao diferente e ja incrementa se for
            

            verifyIfDiffPair(data.codEstacao, data.codProxEstacao, &listPares, &header.nroParesEstacao);
            header.proxRRN++;

            // Liberacao das memorias alocadas
            free(data.nomeEstacao); 
            if (data.nomeLinha != NULL){ // Liberar a memoria alocada caso ele nao seja nulo
            free(data.nomeLinha);
            }
            free(lineCopy);  
        }

        // printf("REGISTROS LIDOS: %d\n", header.proxRRN); // DEBUG
        // printf("Estacoes unicas: %d\n", header.nroEstacoes); // DEBUG

        // Arquivo finalizado. Atualizar header status e fechar o arquivo
        header.status = '1';
        fseek(binFile,0,SEEK_SET);
        fwrite(&header.status, sizeof(char), 1, binFile);
        fwrite(&header.topo, sizeof(int), 1, binFile);
        fwrite(&header.proxRRN, sizeof(int), 1, binFile);
        fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
        fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

        // Limpeza de memoria restante
        for (int i = 0; i < header.nroEstacoes; i++) {
        free(diffStationNames[i]);
        }
        free(diffStationNames);
        if (listPares != NULL){free(listPares);}

        fclose(binFile);
        fclose(csvFile);   

        binarioNaTela(arquivoSaida);
    }

/*
# FUNCIONALIDADE [2] - listTable() #
-> Recuperacao dos dados de TODOS os registros armazenados em um arquivo de dados de entrada, mostrando os dados de forma organizada na saída padrão (dada graficamente no arquivo de instrucoes do projeto) para permitir a distincao dos campos e registros. 
-> O tratamento do 'lixo' deve ser feito de forma a permitir a exibicao apropriada dos dados. 
-> Registros marcados como logicamente removidos nao devem ser exibidos 

ENTRADA: 2 arquivoEntrada.bin
 | - arquivoEntrada.bin: eh o arquivo binario gerado pela funcionalidade [1], conforme as especificacoes descritas neste trabalho pratico 

 SAIDA CASO SEJA EXECUTADO COM SUCESSO: em uma unica linha, cada registro deve ser mostrado
 | - os campos devem ser mostrados de forma sequencial separado por um espaco em branco. 
 | - campos de tamanho fixo com valor nulo: ao inves de exibir -1, escreva NULO
 | - campos de tamanho variavel com valor nulo: escreva NULO
 | Ordem de exibicao dos dados: codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEstacao, distProxEstacao, codLinhaIntegra, codEstIntegra

 SAIDA CASO NAO EXISTAM REGISTROS: 
 | - "Registro inexistente."
 
 SAIDA CASO ALGUM ERRO SEJA ENCONTRADO: 
 | - "Falha no processamento do arquivo."

 EXEMPLO DE EXECUCAO:
 ./programaTrab
 2 estacao.bin
 *saida: 
 1 Tucuruvi 1 Azul 2 992 NULO NULO
 2 Parada Inglesa 1 Azul 3 1057 4 55
 ...
*/

void listTable(char *arquivoEntrada) {
    // 1. Abre o arquivo binário para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // 2. Lê o status do arquivo (1 byte)
    char status;
    fread(&status, sizeof(char), 1, binFile);

    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // 3. Pula o restante do cabeçalho (16 bytes restantes dos 17 totais)
    fseek(binFile, 16, SEEK_CUR);

    int encontrouRegistro = 0; 
    char removido;

    // 4. O Loop de Leitura (O "Cursor" da nossa tabela)
    while (fread(&removido, sizeof(char), 1, binFile) == 1) {
        
        if (removido == '1') {
            // Se está removido, pula o restante do registro (80 - 1 = 79 bytes)
            fseek(binFile, 79, SEEK_CUR);
            continue; 
        }

        encontrouRegistro = 1;
        DataRecord data;
        data.removido = removido; 
        
        // A) Ler os próximos campos de tamanho fixo
        fread(&data.proximo, sizeof(int), 1, binFile);
        fread(&data.codEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinha, sizeof(int), 1, binFile);
        fread(&data.codProxEstacao, sizeof(int), 1, binFile);
        fread(&data.distProxEstacao, sizeof(int), 1, binFile);
        fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fread(&data.codEstIntegra, sizeof(int), 1, binFile);

        // B) Ler o tamNomeEstacao e a string nomeEstacao
        fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if (data.tamNomeEstacao > 0) {
            data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
            fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
            data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
        } else {
            data.nomeEstacao = NULL;
        }

        // C) Ler o tamNomeLinha e a string nomeLinha
        fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if (data.tamNomeLinha > 0) {
            data.nomeLinha = (char *)malloc((data.tamNomeLinha + 1) * sizeof(char));
            fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
            data.nomeLinha[data.tamNomeLinha] = '\0';
        } else {
            data.nomeLinha = NULL;
        }

        // D) Pular os bytes de lixo ('$') para alinhar o ponteiro pro próximo while
        int garbageBytes = REGISTER_SIZE - (FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR); 

        // E) Imprimir os dados formatados tratando os NULOS
        printf("%d ", data.codEstacao);

        if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao);
        else printf("NULO ");

        if (data.codLinha == -1) printf("NULO ");
        else printf("%d ", data.codLinha);

        if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha);
        else printf("NULO ");

        if (data.codProxEstacao == -1) printf("NULO ");
        else printf("%d ", data.codProxEstacao);

        if (data.distProxEstacao == -1) printf("NULO ");
        else printf("%d ", data.distProxEstacao);

        if (data.codLinhaIntegra == -1) printf("NULO ");
        else printf("%d ", data.codLinhaIntegra);

        if (data.codEstIntegra == -1) printf("NULO\n");
        else printf("%d\n", data.codEstIntegra);

        // F) Liberar a memória das strings alocadas nesta iteração
        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    }

    // 5. Finalização
    if (!encontrouRegistro) {
        printf("Registro inexistente.\n");
    }

    fclose(binFile);
}


/*
# FUNCIONALIDADE [3] - listTableWhere() #
-> Recuperacao dos dados dos registros que satisfacam um criteiro de busca, determinado pelo usuario.
-> Qualquer campo pode ser usado como forma de busca
-> A busca deve ser feita considerando um ou mais campos 
-> POR EXEMPLO: a busca pode ser feita considerando somente o campo codEstacao ou considerando os campos nomeEstacao e nomeLinha
-> Essa funcao pode retornar 0 registros (quando nenhum registro satisfaz o criterio), 1 registro (quando apenas um satisfaz) ou mais registros (quando um ou mais registros satisfazem o criterio)
-> Os valores do tipo string devem ser especificados entre aspas duplas (")
-> Para a manipulacao de strings com aspas duplas, pode-se usar a funcao scan_quote_string (dada em utils.h) 
-> Para a busca por campos nulos, deve-se especificar o valor NULO
-> Registros marcados como logicamente removidos nao devem ser exibidos


ENTRADA: 3 arquivoEntrada.bin n
m1 nomeCampo1 valorCampo1 ... nomeCampom1 valorCampom1
m2 nomeCampo1 valorCampo1 ... nomeCampom2 valorCampom2
...
mn nomeCampo1 valorCampo1 ... nomeCampomn valorCampomn
 | - arquivoEntrada.bin: eh o arquivo binario gerado pela funcionalidade [1], o qual foi gerado conforme as especificacoes descritas neste trabalho pratico 
 | - n: eh a quantidade de vezes que a busca deve acontecer
 | - m: eh a quantidade de vezes que o par "nome do campo" e "valor do campo" pode repetir em uma busca. Deve ser deixado um espaco em branco entre nomeCampo e valorCampo. Os valores dos campos do tipo string devem ser especificados entre aspas duplas ("). Buscas por campos nulos devem ser especificadas usando o valor NULO

 SAIDA CASO SEJA EXECUTADO COM SUCESSO: cada registro deve ser mostrado em uma unica linha e os campos devem ser mostrados de forma sequencial separado por um espaco em branco.
  | campos de tamanho fixo que tiverem o valor nulo: ao inves de exibir -1 , escreva NULO
  | campos de tamanho variavel que tiverem o valor nulo: escreva NULO
  | Ordem de exibicao dos dados: codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEstacao, distProxEstacao, codLinhaIntegra, codEstIntegra.

  SAIDA CASO NAO SEJA ENCONTRADO O REGISTRO QUE CONTEM O VALOR DO CAMPO OU O CAMPO PERTENCE A UM REGISTRO QUE ESTEJA REMOVIDO: 
  | - "Registro inexistente."

  SAIDA CASO OCORRA ALGUM ERRO:
  | - "Falha no processamento do arquivo."

 EXEMPLO DE EXECUCAO:
 ./programaTrab
 3 estacao.bin 1
 1 nomeEstacao "Luz" ---> essa linha eh o criterio de busca, ou seja, o nome do campo a ser buscado eh nomeEstacao e o valor a ser buscado eh "Luz" <---
 9 Luz 1 Azul 10 NULO 4 55
 55 Luz 4 Amarela 56 1257 1 9
 111 Luz 7 Rubi 112 NULO NULO NULO
 ... 
*/

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

    // O Loop Principal: Roda 'n' vezes, uma para cada busca solicitada pelo usuário
    for (int i = 0; i < n; i++) {
        
        int m;
        scanf("%d", &m); // Lê quantos filtros essa busca vai ter

        // Arrays para guardar os nomes dos campos e os valores que o usuário vai digitar
        char campos[8][30];
        char valores[8][100];

        // 1. CAPTURA DOS FILTROS
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j]); // Lê o nome do campo (ex: "codLinha")
            ScanQuoteString(valores[j]); // Lê o valor tratando as aspas e o NULO
        }

        // 2. PREPARAR O FULL TABLE SCAN
        // Como o usuário pode fazer várias buscas (n > 1), precisamos garantir que 
        // a "agulha" de leitura do arquivo volte para o início dos dados (Byte 17) a cada nova busca!
        fseek(binFile, HEADER_SIZE, SEEK_SET);

        int encontrouRegistro = 0;
        char removido;

        // 3. A VARREDURA O(n) (Idêntico à Funcionalidade 2)
        while (fread(&removido, sizeof(char), 1, binFile) == 1) {
            if (removido == '1') {
                fseek(binFile, 79, SEEK_CUR);
                continue;
            }

            DataRecord data;
            data.removido = removido;

            // Lendo os campos fixos
            fread(&data.proximo, sizeof(int), 1, binFile);
            fread(&data.codEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinha, sizeof(int), 1, binFile);
            fread(&data.codProxEstacao, sizeof(int), 1, binFile);
            fread(&data.distProxEstacao, sizeof(int), 1, binFile);
            fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
            fread(&data.codEstIntegra, sizeof(int), 1, binFile);

            // Lendo nomeEstacao
            fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
            if (data.tamNomeEstacao > 0) {
                data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
                fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
                data.nomeEstacao[data.tamNomeEstacao] = '\0';
            } else {
                data.nomeEstacao = NULL;
            }

            // Lendo nomeLinha
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

            // 4. O TESTE DE MATCH (O nosso WHERE dinâmico)
            int match = 1; // Assumimos que a linha serve, até provar o contrário

            for (int f = 0; f < m; f++) {
                // A função ScanQuoteString converte a palavra NULO para uma string vazia "". 
                // Então usamos o operador ternário para transformar "" em -1 para os inteiros.

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
                    // Para strings, comparamos usando strcmp
                    if (strcmp(valores[f], "") == 0) { 
                        if (data.nomeEstacao != NULL) match = 0; // Buscou NULO, mas tem dado
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
                
                // Se um dos filtros já reprovou o registro, não precisamos testar os outros
                if (match == 0) break; 
            }

            // 5. SE DEU MATCH EM TUDO, IMPRIME!
            if (match == 1) {
                encontrouRegistro = 1;
                
                printf("%d ", data.codEstacao);
                if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao);
                else printf("NULO ");

                if (data.codLinha == -1) printf("NULO ");
                else printf("%d ", data.codLinha);

                if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha);
                else printf("NULO ");

                if (data.codProxEstacao == -1) printf("NULO ");
                else printf("%d ", data.codProxEstacao);

                if (data.distProxEstacao == -1) printf("NULO ");
                else printf("%d ", data.distProxEstacao);

                if (data.codLinhaIntegra == -1) printf("NULO ");
                else printf("%d ", data.codLinhaIntegra);

                if (data.codEstIntegra == -1) printf("NULO\n");
                else printf("%d\n", data.codEstIntegra);
            }

            // Limpa a memória pra próxima iteração do while
            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);
        }

        // Se chegou no fim do arquivo e o encontrouRegistro ainda é 0, a busca falhou
        if (!encontrouRegistro) {
            printf("Registro inexistente.\n");
        }
    }

    fclose(binFile);
}

/*
# FUNCIONALIDADE [4] - listTableRRN() #
-> Recuperacao dos dados de um registro de um arquivo a partir da identificacao do RRN do registro desejado.
-> Por exemplo, pode-se solicitar a recuperacao dos dados do registro de RRN = 2 ou do registro de RRN = 4. Essa funcionalidade pode retornar 0 registros (quando nenhum satisfaz ao criterio de busca) ou 1 registro (quando apenas um satisfaz).
-> Registros marcados como logicamente removidos nao devem ser exibidos

ENTRADA: 4 arquivoEntrada.bin RRN
 | - arquivoEntrada.bin: eh o arquivo binario gerado pela funcionalidade [1], o qual foi gerado conforme as especificacoes descritas neste trabalho pratico 
 | - RRN: eh o numero do registro a ser recuperado. O RRN do primeiro registro do arquivo é 0, o RRN do segundo registro é 1, e assim por diante. O RRN deve ser um numero inteiro nao negativo.

 SAIDA CASO SEJA EXECUTADO: 
 | - Caso a funcionalidade retornar 0 registros: "Registro inexistente."
 | - Quando a funcionalidade retornar 1 registro: os valores de seus campos devem ser mostrados de forma sequencial separado por um espaco em branco
 | - Campos de tamanho fixo que tiverem valor nulo: ao inves de exibir -1, escreva NULO
 | - Campos de tamanho variavel que tiverem valor nulo: escreva NULO
 | - Ordem de exibicao dos dados: codEstacao, nomeEstacao, codLinha, nomeLinha, codProxEstacao, distProxEstacao, codLinhaIntegra, codEstIntegra

 SAIDA CASO NAO SEJA ENCONTRADO O REGISTRO QUE CONTEM O VALOR DO CAMPO OU O CAMPO PERTENCE A UM REGISTRO QUE ESTEJA REMOVIDO: 
 | - "Registro inexistente."
 
 SAIDA CASO OCORRA ALGUM ERRO:
 | - "Falha no processamento do arquivo."

 EXEMPLO DE EXECUCAO:
 ./programaTrab
 4 estacao.bin 1
 9 Luz 1 Azul 10 NULO 4 55

*/

void listTableRRN(char *arquivoEntrada, int RRN) {
    // 1. Abre o arquivo binário para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // 2. Lê o status do arquivo
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // 3. Calcula o byte offset e pula direto para o registro desejado
    int byteOffset = HEADER_SIZE + (RRN * REGISTER_SIZE);
    fseek(binFile, byteOffset, SEEK_SET);

    // 4. Tenta ler o status de removido. 
    // Se o fread retornar algo diferente de 1, o RRN está além do fim do arquivo.
    char removido;
    if (fread(&removido, sizeof(char), 1, binFile) != 1 || removido == '1') {
        printf("Registro inexistente.\n");
        fclose(binFile);
        return;
    }

    // 5. Se o registro existe e não está removido, extrai e imprime
    DataRecord data;
    data.removido = removido; 
    
    // A) Ler os próximos campos de tamanho fixo
    fread(&data.proximo, sizeof(int), 1, binFile);
    fread(&data.codEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinha, sizeof(int), 1, binFile);
    fread(&data.codProxEstacao, sizeof(int), 1, binFile);
    fread(&data.distProxEstacao, sizeof(int), 1, binFile);
    fread(&data.codLinhaIntegra, sizeof(int), 1, binFile);
    fread(&data.codEstIntegra, sizeof(int), 1, binFile);

    // B) Ler o tamNomeEstacao e a string nomeEstacao
    fread(&data.tamNomeEstacao, sizeof(int), 1, binFile);
    if (data.tamNomeEstacao > 0) {
        data.nomeEstacao = (char *)malloc((data.tamNomeEstacao + 1) * sizeof(char));
        fread(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        data.nomeEstacao[data.tamNomeEstacao] = '\0'; 
    } else {
        data.nomeEstacao = NULL;
    }

    // C) Ler o tamNomeLinha e a string nomeLinha
    fread(&data.tamNomeLinha, sizeof(int), 1, binFile);
    if (data.tamNomeLinha > 0) {
        data.nomeLinha = (char *)malloc((data.tamNomeLinha + 1) * sizeof(char));
        fread(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        data.nomeLinha[data.tamNomeLinha] = '\0';
    } else {
        data.nomeLinha = NULL;
    }

    // E) Imprimir os dados formatados tratando os NULOS
    printf("%d ", data.codEstacao);

    if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao);
    else printf("NULO ");

    if (data.codLinha == -1) printf("NULO ");
    else printf("%d ", data.codLinha);

    if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha);
    else printf("NULO ");

    if (data.codProxEstacao == -1) printf("NULO ");
    else printf("%d ", data.codProxEstacao);

    if (data.distProxEstacao == -1) printf("NULO ");
    else printf("%d ", data.distProxEstacao);

    if (data.codLinhaIntegra == -1) printf("NULO ");
    else printf("%d ", data.codLinhaIntegra);

    if (data.codEstIntegra == -1) printf("NULO\n");
    else printf("%d\n", data.codEstIntegra);

    // F) Liberar a memória das strings alocadas nesta iteração
    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
    if (data.nomeLinha != NULL) free(data.nomeLinha);

    fclose(binFile);
}