/* 
INTEGRANTES DO GRUPO: 
Aluno 1:
 - Gustavo Fantato Fernandes
 - 16986132

 Aluno 2:
 - Victor Kayky Zaneti
 - 15491132
*/

#include <stdio.h>
#include "C:\Users\Victor Zaneti\Desktop\prj1-ed2\modules\functions.h" // Ajuste o caminho conforme a sua pasta

int main() {
    int funcionalidade;
    
    // Lê qual funcionalidade o usuário (ou o run.codes) quer executar
    if (scanf("%d", &funcionalidade) != 1) {
        return 0; // Se não conseguir ler nada, encerra.
    }

    if (funcionalidade == 1) {
        char arquivoEntrada[100];
        char arquivoSaida[100];
        // Funcionalidade 1 recebe 2 strings: csv e binário
        scanf("%s %s", arquivoEntrada, arquivoSaida);
        createTable(arquivoEntrada, arquivoSaida);
        
    } else if (funcionalidade == 2) {
        char arquivoEntrada[100];
        // Funcionalidade 2 recebe 1 string: o nome do binário gerado
        scanf("%s", arquivoEntrada);
        listTable(arquivoEntrada);

    } else if (funcionalidade == 3) {
        char arquivoEntrada[100];
        int n; // Número de buscas
        
        scanf("%s %d", arquivoEntrada, &n);
        listTableWhere(arquivoEntrada, n);
        
    } else if (funcionalidade == 4) {
        char arquivoEntrada[100];
        int rrnBusca;
        
        // Lê o arquivo binário e o RRN desejado
        scanf("%s %d", arquivoEntrada, &rrnBusca);
        listTableRRN(arquivoEntrada, rrnBusca);
    }

    return 0;
}