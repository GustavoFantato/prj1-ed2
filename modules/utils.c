#include "structs.h"
#include "utils.h"

/*
# UTILIDADES GERAIS - Codigos reutilizaveis para auxiliar na execucao das funcionalidades #
*/


// FUNCOES UTILIZADAS NA FUNCIONALIDADE [1]

// Ao fim da funcionalidade, printa a soma dos bytes do arquivo binario
void binarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

// Leitura de cada linha do arquivo CSV, retornando NULL quando chega ao fim do arquivo
char *readCSVLine(char line[], int line_size, FILE *csvFile){   
    return fgets(line, line_size, csvFile); // Se chega ao fim ja retorna NULL naturalmente
}

// Switch para atribuir os campos do data record de acordo com o index do campo lido do CSV, ja que a ordem dos campos no CSV eh fixa e conhecida
void switchDataRecord(DataRecord *data, int i, char *field){
    switch(i){
        case 0: 
            data->codEstacao = atoi(field);
            break;

        case 1: 
            // Dado no projeto que o nomeEstacao nao aceita valor nulo, por isso nao verifica-se
            data->tamNomeEstacao = strlen(field);
            data->nomeEstacao = strdup(field); // Cria-se uma copia do field para evitar que seja sobrescrito posteriormente
            break;

        case 2: 
            if(verifyIfNullField(field)) data->codLinha = -1; 
            else data->codLinha = atoi(field);
            break;

        case 3: 
            if(verifyIfNullField(field)){
                data->tamNomeLinha = 0; 
                data->nomeLinha = NULL;
            } else {
                data->tamNomeLinha = strlen(field); 
                data->nomeLinha = strdup(field); // Cria-se uma copia do field para evitar que seja sobrescrito posteriormente
            }
            break;

        case 4:
            if(verifyIfNullField(field)) data->codProxEstacao = -1; 
            else data->codProxEstacao = atoi(field);
            break;

        case 5: 
            if(verifyIfNullField(field)) data->distProxEstacao = -1; 
            else data->distProxEstacao = atoi(field);
            break;

        case 6:
            if(verifyIfNullField(field)) data->codLinhaIntegra = -1; 
            else data->codLinhaIntegra = atoi(field);
            break;

        case 7: 
            if(verifyIfNullField(field)) data->codEstIntegra = -1; 
            else data->codEstIntegra = atoi(field);
            break;
    }
}

// Funcao para verificar se o nome da estacao lida do arquivo binario ja foi encontrada anteriormente, para controle do nroEstacoes no header
void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd){
   
    // Loop para verificar se o nome da estacao ja esta no array de estacoes diferentes
    for (int i = 0; i < *stationsQtd; i++){
        if (strcmp(name, (*diffStationNames)[i]) == 0){ 
            return; // Se encontrado apenas retorna
        }
    }

    // Se saiu do loop, nao foi encontrado. Entao eh uma estacao nova
    (*stationsQtd)++; // Incrementa a quantidade de estacoes diretamente no ponteiro para que seja atualizada na funcao que chamou

    // Cria um ponteiro temporario para realocar o tamanho do array de estacoes diferentes, para conseguir adicionar o novo nome encontrado
    char **temp = realloc(*diffStationNames, sizeof(char*) * (*stationsQtd)); 
    
    // Se ocorrer um problema ao realocar, nao atualiza o ponteiro e retorna
    if (temp == NULL){
        printf("Falha ao alocar ponteiro para controle de estacoes diferentes.\n");
        (*stationsQtd)--;
        return;   
    }
    
    // Atualiza o ponteiro do array de estacoes diferentes e adiciona o novo nome
    *diffStationNames = temp;
    (*diffStationNames)[(*stationsQtd) - 1] = strdup(name);
    return;
}

// Funcao para verificar se o par de estacao ja foi encontrado anteriormente no CSV
void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares){

    // Loop para verificar se o par ja esta no array de pares diferentes
    for (int i = 0; i < *nPares; i++){
        if ((*listPar)[i].orig == orig && (*listPar)[i].dest == dest){
            return; 
        }
    }

    // Se nao estiver no array, eh um par diferente e deve ser adicionado
    (*nPares)++;

    // Cria-se um ponteiro temporario para realocar o tamanho do array de pares, para conseguir adicionar o novo par encontrado
    Par *temp = realloc(*listPar, sizeof(Par) * (*nPares));
    
    // Se ocorrer um problema ao realocar, nao atualiza o ponteiro e retorna
    if (temp == NULL){
        printf("Falha ao alocar ponteiro para controle de pares diferentes.\n");
        (*nPares)--;
        return;   
    }
    
    // Atualiza o ponteiro do array de pares diferentes e adiciona o novo par
    *listPar = temp;
    (*listPar)[(*nPares) - 1].orig = orig;
    (*listPar)[(*nPares) - 1].dest = dest;
    return;
}


// Usadas na funcionalide [3]


// Funcao dada para ler os campos iniciados com ""
void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

// Funcao para pegar e retornar o valor inteiro de um campo, verificando se eh nulo ou nao
int getValue(char *valor) {
    if (strcmp(valor, "") == 0) {
        return -1;
    }
    return atoi(valor); // converte para int
}

// Funcao para verificar se a string lida da match com a string de busca, considerando o caso de busca por campo nulo (string de busca vazia)
int checkStringMatch(char *data_string, char *valor_busca) {
    if (strcmp(valor_busca, "") == 0) {
        return (data_string == NULL);
    }
    if (data_string == NULL) {
        return 0;
    }
    return (strcmp(data_string, valor_busca) == 0);
}