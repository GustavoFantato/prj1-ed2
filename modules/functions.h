#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Descricao das funcoes utilizadas no programa // 

/*
# FUNCIONALIDADE [1] - # 

-> Leitura de varios registros obtidos a partir de um arquivo de entrada .csv (que esta em /data) e armazenamento desses registros em um arquivo de dados binário
-> Deve-se usar a funcao binarioNaTela (dada em utils.h). Ela deve ser usada depois que o arquivo é fechado e antes de terminar a execucao da funcionalidade

ENTRADA: 1 arquivoEntrada.csv e arquivoSaida.bin
 | - arquivoEntrada.csv: contem os valores dos campos dos registros a serem armazenados no arquivo binario
 | - arquivoSaida.bin: arquivo binario gerado conforme as instrucoes abaixo

SAIDA: 
 | Caso tudo ocorra bem: a saida deve ser o arquivo no formato binario usando a funcao fornecida binarioNaTela.
 | Caso ocorra um erro: "Falha no processamento do arquivo."
*/