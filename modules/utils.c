#include "structs.h"
#include "utils.h"

/*
 * Use essa função para comparação no run.codes.
 * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
 * Ela vai abrir de novo para leitura e depois fechar.
 */
char *custom_strsep(char **stringp, const char *delim) {
    if (*stringp == NULL) return NULL;
    char *start = *stringp;
    char *end = strpbrk(start, delim);
    if (end) {
        *end = '\0';
        *stringp = end + 1;
    } else {
        *stringp = NULL;
    }
    return start;
}

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

/*
 * Use essa função para ler um campo string delimitado entre aspas (").
 * Chame ela na hora que for ler tal campo.
 */
void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

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

/*
 * Lê uma linha inteira do arquivo CSV para um buffer de memória.
 */
char *readCSVLine(char line[], int line_size, FILE *csvFile){
    return fgets(line, line_size, csvFile);
}

/*
 * Extrai o conteúdo de uma coluna específica do CSV baseada no seu índice.
 * Lida com o fatiamento da string separada por vírgulas.
 */
char *parseCSVField(char *line, int index){
    char *field = NULL; 
    char *ptr = line; // Mantém o endereço original, pois custom_strsep altera o ponteiro

    // Avança pelos tokens da string até alcançar a coluna desejada
    for (int i = 0; i <= index; i++){ 
        field = custom_strsep(&ptr, ",");
        if (field == NULL){
            return NULL;
        }
    }

    // Limpa caracteres de quebra de linha caso seja a última coluna (Windows/Linux)
    if (field != NULL){
        field[strcspn(field, "\r\n")] = '\0'; 
    }
    
    return field;
}

/*
 * Verifica se o campo extraído está vazio, garantindo o tratamento de NULOs.
 */
int verifyIfNullField(char *field){
    if (field == NULL || field[0] == '\0'){
        return 1;
    }
    return 0;
}

/*
 * Motor de conversão: Recebe uma string lida do CSV e a converte para o tipo
 * correto dentro da struct DataRecord, dependendo da coluna (índice i).
 */
void switchDataRecord(DataRecord *data, int i, char *field){
    switch(i){
        case 0: // codEstacao (Sempre preenchido)
            data->codEstacao = atoi(field);
            break;

        case 1: // nomeEstacao (Tamanho variável)
            data->tamNomeEstacao = strlen(field);
            // strdup aloca memória para a cópia, evitando sobrescrita no próximo loop
            data->nomeEstacao = strdup(field); 
            break;

        case 2: // codLinha
            if(verifyIfNullField(field)) data->codLinha = -1; 
            else data->codLinha = atoi(field);
            break;

        case 3: // nomeLinha (Tamanho variável)
            if(verifyIfNullField(field)){
                data->tamNomeLinha = 0; 
                data->nomeLinha = NULL;
            } else {
                data->tamNomeLinha = strlen(field); 
                data->nomeLinha = strdup(field);
            }
            break;

        case 4: // codProxEstacao
            if(verifyIfNullField(field)) data->codProxEstacao = -1; 
            else data->codProxEstacao = atoi(field);
            break;

        case 5: // distProxEstacao
            if(verifyIfNullField(field)) data->distProxEstacao = -1; 
            else data->distProxEstacao = atoi(field);
            break;

        case 6: // codLinhaIntegra
            if(verifyIfNullField(field)) data->codLinhaIntegra = -1; 
            else data->codLinhaIntegra = atoi(field);
            break;

        case 7: // codEstIntegra
            if(verifyIfNullField(field)) data->codEstIntegra = -1; 
            else data->codEstIntegra = atoi(field);
            break;
    }
}

/*
 * Registra estações únicas para atualização do cabeçalho.
 * Usa alocação dinâmica para crescer a lista de nomes apenas quando necessário.
 */
void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd){
    // Busca linear para verificar se a estação já foi registrada
    for (int i = 0; i < *stationsQtd; i++){
        if (strcmp(name, (*diffStationNames)[i]) == 0){ 
            return; // Estação já existe, aborta a inserção
        }
    }

    // Se é uma estação nova, expande a matriz dinamicamente
    (*stationsQtd)++;
    char **temp = realloc(*diffStationNames, sizeof(char*) * (*stationsQtd));
    
    if (temp == NULL) return; // Falha de alocação de segurança
    
    *diffStationNames = temp;
    // Insere a nova estação na última posição disponível
    (*diffStationNames)[(*stationsQtd) - 1] = strdup(name);
}

/*
 * Registra pares únicos de estações (origem/destino) para o cabeçalho.
 * Segue a mesma lógica de alocação sob demanda da função de estações.
 */
void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares){
    for (int i = 0; i < *nPares; i++){
        if ((*listPar)[i].orig == orig && (*listPar)[i].dest == dest){
            return; // Par já mapeado
        }
    }

    // Se é um par novo, expande a lista de structs Par
    (*nPares)++;
    Par *temp = realloc(*listPar, sizeof(Par) * (*nPares));
    
    if (temp == NULL) return;
    
    *listPar = temp;
    (*listPar)[(*nPares) - 1].orig = orig;
    (*listPar)[(*nPares) - 1].dest = dest;
}