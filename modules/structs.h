#ifndef STRUCTS_H
#define STRUCTS_H

#define REGISTER_SIZE 80 // Tamanho total dos registros de dados
#define HEADER_SIZE 17 // Tamanho total do registro de cabecalho        
#define FIX_SIZE_FIELDS 37 // Tamanho total dos campos de tamanho fixo
#define REGISTER_QTD 8 // Quantidade campos de cada registro de dados


/*
 # Registro de cabecalho - 17 bytes totais #

 Observacoes:
 1- O registro de cabecalho deve seguir estritamente a ordem definida abaixo
 2- Os campos são de tamanhos fixos. Logo, os valores que forem armazenados não devem ser finalizados por '\0'

*/

typedef struct headerRecord {
    char status; // Consistencia do arquivo. '0' para inconsistente e '1' para consistente
    int topo;  // byte offset de um registro logicamente removido. Inicializado com -1         
    int proxRRN; // proximo RRN disponivel         
    int nroEstacoes; // quantidade de estacoes diferentes
    int nroParesEstacao; // quantidade de pares (CodEstacao, CodProxEstacao) diferentes que estao armazenados no arquivo de dados
} HeaderRecord;

/*
# Registro de Dados - 80 bytes #
-> Campos de tamanhos fixo e variáveis
-> Para os de tamanho variável deve ser usado o método de indicador de tamanho
-> Campos nulos são representados por espaços em branco

Observacoes:
1. Cada registro deve seguir a ordem definida na sua representacao grafica (dado no documento de instrucoes do projeto)
2. As strings de tamanho variavel sao identificadas pelo seu tamanho, nao devendo ser finalizada com '\0'
3. Os campos codEstacao e nomeEstacao nao aceitam valores nulos. Os demais aceitam. O arquivo com os dados de entrada ja garante essas caracteristicas
    3.1. Para os campos de tamanho fixo, os valores nulos devem ser representados por -1
    3.2 Para os campos de tamanho variavel, armazenar um valor nulo significa armazenar o tamanho zero no indicador de tamanho 
*/

typedef struct dataRecord {

    char removido;       
    int proximo;   


    int codEstacao; 
    int codLinha; 
    int codProxEstacao; 
    int distProxEstacao; 
    int codLinhaIntegra;
    int codEstIntegra;

    int tamNomeEstacao; // Indicador de tamanho
    char *nomeEstacao;   

    int tamNomeLinha; // Indicador de tamanho
    char *nomeLinha;    
} DataRecord;

// Struct utilizada na funcionalidade [1], para controle dos pares de estacoes diferentes, para o armazenamento do nroParesEstacao no header
typedef struct par {
    int orig; // origem (CodEstacao)
    int dest; // destino (CodProxEstacao)
} Par;

#endif