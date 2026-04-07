/* * TRABALHO PRÁTICO - ALGORITMOS E ESTRUTURAS DE DADOS II
 * * INTEGRANTES DO GRUPO: 
 * - Gustavo Fantato Fernandes (16986132)
 * - Victor Kayky Zaneti (15491132)
 */

#include <stdio.h>
#include "../modules/functions.h"

int main() {
    int func;
    
    // Leitura do codigo da funcionalidade solicitada
    if (scanf("%d", &func) != 1) {
        return 0; // Encerra execucao se o ocorrer erro ao ler a opcao de funcionalidade
    }

    // Criacao das variaveis do nome dos arquivos
    char arquivoEntrada[100];
    char arquivoSaida[100];

    // Associar a funcionalidade lida com a funcao correspondente
    switch (func) {
        
        case 1:
            // [1] - createTable
            scanf("%s %s", arquivoEntrada, arquivoSaida);
            createTable(arquivoEntrada, arquivoSaida);
            break;

        case 2:
            // [2] - listTable
            scanf("%s", arquivoEntrada);
            listTable(arquivoEntrada);
            break;

        case 3:
            // [3] - listTableWhere
            {
                int n; 
                scanf("%s %d", arquivoEntrada, &n);
                listTableWhere(arquivoEntrada, n);
            }
            break;

        case 4:
            // [4] - listTableRRN
            {
                int RRN;
                scanf("%s %d", arquivoEntrada, &RRN);
                listTableRRN(arquivoEntrada, RRN);
            }
            break;

        default:
            // Funcionalidade nao existente
            break;
    }

    return 0;
}