/* * TRABALHO PRÁTICO - ALGORITMOS E ESTRUTURAS DE DADOS II
 * * INTEGRANTES DO GRUPO: 
 * - Gustavo Fantato Fernandes (16986132)
 * - Victor Kayky Zaneti (15491132)
 */

#include <stdio.h>
#include "../modules/functions.h"

int main() {
    int funcionalidade;
    
    // Leitura do código da funcionalidade solicitada
    if (scanf("%d", &funcionalidade) != 1) {
        return 0; // Encerra execução caso o buffer esteja vazio ou inválido
    }

    char arquivoEntrada[100];
    char arquivoSaida[100];

    // Roteamento para a funcionalidade correspondente
    switch (funcionalidade) {
        
        case 1:
            // [1] - createTable: Conversão de CSV para Binário
            scanf("%s %s", arquivoEntrada, arquivoSaida);
            createTable(arquivoEntrada, arquivoSaida);
            break;

        case 2:
            // [2] - listTable: Full Table Scan (Varredura Sequencial)
            scanf("%s", arquivoEntrada);
            listTable(arquivoEntrada);
            break;

        case 3:
            // [3] - listTableWhere: Busca Dinâmica O(n) por múltiplos critérios
            {
                int n; 
                scanf("%s %d", arquivoEntrada, &n);
                listTableWhere(arquivoEntrada, n);
            }
            break;

        case 4:
            // [4] - listTableRRN: Busca Direta O(1) via Relative Record Number
            {
                int rrnBusca;
                scanf("%s %d", arquivoEntrada, &rrnBusca);
                listTableRRN(arquivoEntrada, rrnBusca);
            }
            break;

        default:
            // Funcionalidade não mapeada
            break;
    }

    return 0;
}