# Arquitetura-de-Computadores
Repositório da disciplina de Arquitetura de Computadores — UFMA, 2026.1.
Simulador de Pipeline MIPS-Lite

Simulador educacional de arquitetura MIPS desenvolvido em linguagem C, com foco na implementação de pipeline de 5 estágios.

📚 Objetivo

O projeto tem como finalidade simular o funcionamento interno de um processador MIPS simplificado, permitindo visualizar:

Memória de instruções
Memória de dados
Registradores
Program Counter (PC)
Registradores de pipeline
Execução em estágios
Controle de hazards e stalls

O simulador foi desenvolvido de forma modular, separando estrutura de dados, memória e lógica do pipeline.

🏗️ Estrutura do Projeto
arquitetura-de-computadores/
│
├── main.c
├── memory.c
├── memory.h
├── types.h
└── README.md
⚙️ Componentes
types.h

Arquivo responsável por centralizar:

Constantes globais
Definições de memória
Quantidade de registradores
Structs dos registradores de pipeline
Variáveis globais compartilhadas

Registradores implementados:

IF/ID
ID/EX
EX/MEM
MEM/WB
memory.c

Responsável pelo gerenciamento do estado persistente do simulador:

Inicialização das memórias
Inicialização dos registradores
Inicialização do PC
Carregamento do programa na memória de instruções
memory.h

Header contendo as funções públicas do módulo de memória.

main.c

Arquivo principal utilizado para:

Inicializar o simulador
Carregar instruções
Exibir estado inicial da arquitetura
🧠 Arquitetura Simulada

O simulador utiliza uma arquitetura inspirada no pipeline clássico MIPS de 5 estágios:

IF  →  ID  →  EX  →  MEM  →  WB

Onde:

Estágio	Descrição
IF	Busca da instrução
ID	Decodificação
EX	Execução da ALU
MEM	Acesso à memória
WB	Escrita no banco de registradores
💾 Configurações da Memória
Componente	Tamanho
Memória de instruções	1024 bytes
Memória de dados	1024 bytes
Registradores	32
Tamanho da instrução	4 bytes
▶️ Como Compilar
Windows (PowerShell)
gcc main.c memory.c -o simulador
▶️ Como Executar
.\simulador.exe
📌 Instruções carregadas inicialmente
ADDI $t0, $zero, 10
ADD  $s0, $t0, $zero
SW   $s0, 0($t1)

Além de instruções NOP para esvaziamento do pipeline.

🛠️ Tecnologias Utilizadas
Linguagem C
GCC
VS Code