#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.c"

/*
 * Use essa função para comparação no run.codes.
 * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
 *
 * Ela vai abrir de novo para leitura e depois fechar
 * (você não vai perder pontos por isso se usar ela).
 */
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
 *	Use essa função para ler um campo string delimitado entre aspas (").
 *	Chame ela na hora que for ler tal campo. Por exemplo:
 *
 *	A entrada está da seguinte forma:
 *		nomeDoCampo "MARIA DA SILVA"
 *
 *	Para ler isso para as strings já alocadas str1 e str2 do seu programa,
 * você faz: scanf("%s", str1); // Vai salvar nomeDoCampo em str1
 *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2
 * (sem as aspas)
 *
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

char *readCSVLine(char line[], int line_size, FILE *csvFile){
    return fgets(line, line_size, csvFile);
}


// Funcao que vai retornar o conteudo do field, de acordo com o indice passado
char *parseCSVField(char *line, int index){
    char *field = NULL; // campo a ser retornado
    // index eh a posicao para ler o campo
    char *ptr = line; // a funcao strsep() muda o endereco de memoria, entao criamos um ponteiro para nao perder o endereco original

    for (int i = 0; i <= index; i++){ // loop para ler os fields da linha, ate chegar no index desejado
        field = strsep(&ptr, ",");
    }

    field[strcspn(field, "\r\n")] = '\0'; // caso seja o ultimo campo da linha, temos que tirar o \n e, talvez um \r, que vem junto, substituindo pelo \0

    return field;
}


// Funcao que vai verificar se o campo lido eh nulo ou nao, de acordo com as regras do projeto.
int verifyIfNullField(char *field){
    if (field[0] == NULL || field[0] == '\0'){
        return 1;
    }
    return 0;
}

void switchDataRecord(DataRecord *data, int i, char *field){

    switch(i){
        case 0:
            data->codEstacao = atoi(field);
            break;
        case 1:
            data->nomeEstacao = strdup(field); // Precisa ser uma copia do field, pois field eh uma string que vai ser sobrescrita no proximo loop
            break;
        case 2:
            if(verifyIfNullField(field)){
                data->codLinha = -1; 
            } else {
                data->codLinha = atoi(field);
            }
            break;
        case 3:
            if(verifyIfNullField(field)){
                data->tamNomeLinha = 0; 
                data->nomeLinha = NULL;
            } else {
                data->tamNomeLinha = strlen(field); 
                data->nomeLinha = strdup(field);
            }
            break;
        case 4:
            if(verifyIfNullField(field)){
                data->codProxEstacao = -1; 
            } else {
                data->codProxEstacao = atoi(field);
            }
            break;
        case 5:
            if(verifyIfNullField(field)){
                data->distProxEstacao = -1; 
            } else {
                data->distProxEstacao = atoi(field);
            }
            break;
        case 6:
            if(verifyIfNullField(field)){
                data->codLinhaIntegra = -1; 
            } else {
                data->codLinhaIntegra = atoi(field);
            }
            break;
        case 7:
            if(verifyIfNullField(field)){
                data->codEstIntegra = -1; 
            } else {
                data->codEstIntegra = atoi(field);
            }
            break;
    }
}