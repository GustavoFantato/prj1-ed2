#ifndef STRUCTS_H
#define STRUCTS_H

// Constantes arquiteturais de dimensionamento do arquivo binário
#define REGISTER_SIZE 80      // Tamanho físico em bytes de um registro de dados no disco
#define HEADER_SIZE 17        // Tamanho físico em bytes do cabeçalho
#define FIX_SIZE_FIELDS 37    // Soma dos bytes fixos do registro (char + int + 6*int + 2*int)
#define REGISTER_QTD 8        // Quantidade total de campos manipulados por registro

/*
 * Estrutura do Cabeçalho (17 bytes)
 * Gerencia os metadados do arquivo binário, garantindo o controle de integridade 
 * e fornecendo o estado atualizado (proxRRN, totais) em tempo de execução O(1).
 */
typedef struct headerRecord {
    char status;         // '0' (Arquivo aberto/inconsistente) ou '1' (Fechado/consistente)
    int topo;            // Byte offset da pilha de registros logicamente removidos (-1 se vazia)
    int proxRRN;         // RRN do próximo slot disponível no final do arquivo
    int nroEstacoes;     // Controle da quantidade de estações únicas
    int nroParesEstacao; // Controle de pares únicos (Origem -> Destino)
} HeaderRecord;

/*
 * Estrutura do Registro de Dados (80 bytes no disco)
 * A ordem dos atributos nesta struct foi projetada para refletir exatamente 
 * a ordem física de escrita/leitura sequencial no arquivo binário.
 */
typedef struct dataRecord {
    // --- Controle de Remoção Lógica ---
    char removido;       // '0' (Registro Ativo) ou '1' (Logicamente Removido)
    int proximo;         // RRN do próximo registro removido (-1 se não houver)

    // --- Campos de Tamanho Fixo ---
    // Valores NULOS são armazenados fisicamente como -1.
    int codEstacao;      // Chave (Nunca é nulo)
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;

    // --- Campos de Tamanho Variável ---
    // Valores NULOS possuem indicador de tamanho igual a 0. 
    // Em memória secundária, não recebem o caractere terminador '\0'.
    int tamNomeEstacao;  // Indicador de tamanho
    char *nomeEstacao;   // Ponteiro para string dinâmica na RAM

    int tamNomeLinha;    // Indicador de tamanho
    char *nomeLinha;     // Ponteiro para string dinâmica na RAM
} DataRecord;

/*
 * Estrutura Auxiliar (Transiente)
 * Existe apenas em memória principal (RAM) para apoiar a lógica de contagem 
 * de pares únicos (Origem-Destino) durante a extração do CSV. 
 * Não é gravada diretamente no disco.
 */
typedef struct par {
    int orig;
    int dest;
} Par;

#endif