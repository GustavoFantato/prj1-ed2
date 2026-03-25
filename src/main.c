#include <stdio.h>

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
