# Projeto 1 - Estrutura de Dados II
### Armazenamento e Recuperação de Dados: Estações e Linhas de Metrô/CPTM

Este projeto consiste no desenvolvimento de um sistema de gerenciamento de dados de transporte público, permitindo a conversão de arquivos CSV para um formato binário customizado, além da listagem e busca eficiente de registros.

O sistema foi projetado para garantir a integridade dos dados e o acesso rápido via RRN (Relative Record Number).

---

## 🚀 Funcionalidades Implementadas

O programa processa registros contendo informações detalhadas sobre as estações e linhas. As seguintes operações estão disponíveis:

1. **Geração de Arquivo Binário (`createTable`):**
   - Converte um arquivo `.csv` em um arquivo binário `.bin`.
   - Utiliza registros de **tamanho fixo (128 bytes)** para otimização de busca.
   - Implementa um **Cabeçalho (Header)** de 17 bytes para controle de status e metadados.
   - Tratamento de campos variáveis e preenchimento de bytes excedentes com o caractere `$`.

2. **Recuperação de Dados (`listTable`):**
   - Percorre o arquivo binário e exibe todos os registros ativos (não removidos logicamente).

3. **Busca Parametrizada (`listTableWhere`):**
   - Realiza filtragem baseada em critérios fornecidos pelo usuário para qualquer campo do registro.

4. **Acesso Direto (`listTableRRN`):**
   - Recupera um registro específico de forma instantânea utilizando o cálculo de deslocamento por RRN.

---

## 📁 Estrutura do Projeto

A organização dos arquivos segue a estrutura abaixo:

```text
├── modules/        # Cabeçalhos e Implementações das funções
│   ├── functions.c # Lógica principal das funcionalidades
│   ├── functions.h # Protótipos e definições
│   ├── structs.h   # Definições das estruturas HeaderRecord e DataRecord
│   ├── utils.c     # Funções auxiliares (binarioNaTela, leitura de strings)
│   └── utils.h
├── src/            # Ponto de entrada
│   └── programaTrab.c # Função main e controle de fluxo
├── data/           # Arquivos de dados (.csv e .bin)
└── Makefile        # Automação de compilação, execução e empacotamento
```

## 🛠️ Especificações Técnicas

* **Consistência:** O campo `status` no cabeçalho garante que operações de leitura só ocorram em arquivos que foram fechados corretamente após a escrita (`status = '1'`).
* **Campos Variáveis:** Strings como `nomeEstacao` e `nomeLinha` são precedidas por um inteiro indicando seu tamanho, garantindo flexibilidade dentro do registro fixo.
* **Performance:** A contagem de estações únicas e pares de estações é feita em tempo de inserção, evitando varreduras repetitivas no arquivo.
* **Gerenciamento de Memória:** Uso rigoroso de `malloc` e `free`, garantindo a ausência de vazamentos de memória (*memory leaks*).

---

## 💻 Instruções de Uso

O projeto utiliza um `Makefile` para automatizar as etapas de compilação e execução, garantindo a compatibilidade com o ambiente de correção **RunCodes**.

### 1. Compilação
Para compilar o projeto e gerar o executável `programaTrab`, utilize o comando:
```bash
make all

### 2. Execução
Para rodar o programa (o ambiente de teste redirecionará as entradas automaticamente), utilize:
```bash
make run
```

### 3. Limpeza
Para remover o executável, arquivos binários temporários e arquivos comprimidos, utilize:
```bash
make clean
```

### 4. Submissão
Para gerar o arquivo .zip com a estrutura correta para envio (incluindo as pastas src, modules e o Makefile), utilize:
```bash 
make zip
```

--- 

## Autores
 - Gustavo Fantato Fernandes
 - Victor Kayky Zaneti Antunes