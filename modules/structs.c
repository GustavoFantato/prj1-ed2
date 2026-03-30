#include <stdio.h>
#include <stdlib.h>

/* 

# Registro de cabecalho - 17 bytes totais #

Observacoes:
 1- O registro de cabecalho deve seguir estritamente a ordem definida abaixo
 2- Os campos são de tamanhos fixos. Logo, os valores que forem armazenados não devem ser finalizados por '\0'
 3- Neste projeto, o conceito de pagina de disco nao esta sendo considerado 

- status: consistencia do arquivo
 | 0 -> arquivo de dados inconsistente
 | 1 -> arquivo de dados consistente
 | Obs: ao abrir um arquivo para escrita, seu status deve ser mudado para 0 e mudado para 1 ao finalizar o uso do arquivo
 | Tamanho: string de 1 byte (char)

 - topo: armazena o byte offset
 | byte offset -> guarda o byteoffset de um reg. logicamente removido
 | -1 -> caso não haja registros logicamente removidos
 | Tamanho: int de 4 bytes

 - proxRRN: armazena o valor do prox. RRN disponível
 | Obs: deve ser iniciado com o valor '0' e alterado quando necessário
 | Tamanho: int de 4 bytes


 - nroEstacoes: quantidade de estacoes diferentes que estao armazenadas no arquivo de dados
 | Obs: se duas ou mais estacoes tem o mesmo nome, elas sao consideradas a MESMA estacao
 | Tamanho: int de 4 bytes

 - nroParesEstacao: quantidade de pares (CodEstacao, CodProxEstacao) diferentes que estao armazenados no arquivo de dados
 | Tamanho: int de 4 bytes 

*/

typedef struct {
    char status; // '0' ou '1'
    int top; // byte offset ou -1
    int proxRRN; // deve iniciado com 0
    int nroEstacoes;
    int nroParesEstacao;

} HeaderRecord;


/*

# Registro de Dados - 80 bytes #
-> Campos de tamanhos fixo e variáveis
-> Para os de tamanho variável deve ser usado o método de indicador de tamanho
-> Os dados são fornecidos em um .csv (que esta em /data)
-> No .csv o separador de campos é vírgula ',' e o primeiro registro especifica o que cada coluna representa (ou seja, contém a descrição do conteúdo das colunas)
-> Campos nulos são representados por espaços em branco

Observacoes:
1. Cada registro deve seguir a ordem definida na sua representacao grafica (dado no documento de instrucoes do projeto)
2. As strings de tamanho variavel sao identificadas pelo seu tamanho, nao devendo ser finalizada com '\0'
3. Os campos codEstacao e nomeEstacao nao aceitam valores nulos. Os demais aceitam. O arquivo com os dados de entrada ja garante essas caracteristicas
    3.1. Para os campos de tamanho fixo, os valores nulos devem ser representados por -1
    3.2 Para os campos de tamanho variavel, armazenar um valor nulo significa armazenar o tamanho zero no indicador de tamanho 
4. Deve ser feita a diferenciacao entre o espaco utilizado e o lixo. Sempre que houver lixo ele deve ser identificado pelo caracter '$'. Nenhum byte do registro deve permanecer vazio, ou seja, cada byte deve armazenar um valor válido ou o '$'
5. Não existe a necessidade de truncamento de dados. O arquivo .csv com os dados de entrada já garante essa caracteristica 
6. O conceito de pagina de disco nao esta sendo considerado neste projeto

-- Para os campos de tamanho fixo: --
 - codEstacao: codigo da estacao
 | Tamanho: int de 4 bytes 
 
 - codLinha: codigo da linha
 | Tamanho: int de 4 bytes

 - codProxEstacao: codigo da proxima estacao 
 | Tamanho: int de 4 bytes

 - distProxEstacao: distancia ate a proxima estacao 
 | Tamanho: int de 4 bytes

 - codLinhaIntegra: codigo da linha que faz a integracao entre as linhas
 | Tamanho: int de 4 bytes

 - codEstIntegra: codigo da estacao que faz a integracao entre as linhas
 | Tamanho: int de 4 bytes

 - removido: indica se o registro esta logicamente removido
 | 1 -> registro esta logicamente marcado como removido
 | 0 -> registro nao esta marcado como removido
 | Tamanho: string de 1 byte (char)

 - proximo: armazena o RRN do proximo registro logicamente removido
 | Deve ser inicializado com -1 quando necessário  
 | Tamanho: int de 4 bytes

 - tamNomeEstacao: armazena o tamanho da string de tamanho variavel referente ao nome da estacao 
 | Tamanho: int de 4 bytes

 - tamNomeLinha: armazena o tamanho da string de tamanho variavel referente ao nome da linha
 | Tamanho: int de 4 bytes

 -- Para os campos de tamanho variável: --

 - nomeEstacao: nome da estacao
 - nomeLinha: nome da linha
*/


typedef struct{
    // Tamanho Fixo  
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    char removido;
    int proximo;

    // Tamanho Variável
    char *nomeEstacao;
    char *nomeLinha;
    int tamNomeEstacao; // Indicador de tamanho
    int tamNomeLinha; // Indicador de tamanho

} DataRecord;