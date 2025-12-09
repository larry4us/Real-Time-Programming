# Real-Time-Programming

Trabalhos feitos pra matéria de Programacao em Tempo Real, no intuito de aprender sobre sistemas RTOS.

# LAB5 — Execução em RTOS e Comparação com GPOS

Neste quinto trabalho, o código do sistema de controle distribuído foi adaptado para operar em um ambiente de Tempo Real, permitindo uma comparação direta entre:

- **GPOS (Lab 3)** — temporização relativa e escalonamento `SCHED_OTHER`
- **RTOS (Lab 5)** — temporização absoluta e escalonamento `SCHED_FIFO`

As principais evoluções introduzidas no Lab 5 incluem:

- Uso de **temporização absoluta** via `clock_nanosleep()` com `TIMER_ABSTIME`, eliminando deriva temporal acumulada.
- Configuração de **escalonamento em tempo real** através de `SCHED_FIFO`, com prioridades atribuídas de acordo com o algoritmo RMS.
- Aplicação de **bloqueio de memória** (`mlockall`) evitando page faults e garantindo acesso determinístico à RAM.
- Execução automatizada dos cenários **com e sem carga**, tanto em GPOS quanto em RTOS, via `make full_test`.
- Execução paralela do **cyclictest**, registrando latências externas do sistema durante cada cenário.

Essa versão demonstra claramente a diferença entre ambientes determinísticos e não determinísticos, evidenciando como o RTOS reduz jitter e mantém deadlines mesmo sob carga.

## LAB3
Neste terceiro trabalho, aleḿ das recomendações anteriores, também adicionei uma visualização em PDF pra deixar a exibição dos dados mais conveniente. Por isso, há um novo módulo reportLab (gerenciador de PDF) que pode ser instalado facilmente com:

sudo apt install python3-reportlab

## LAB2
Para este segundo trabalho, a execução da simulação é o ponto mais importante. Portanto, o arquivo .txt do resultado da simulação é sempre gerado. Mas pra uma visualização mais conveniente, um gráfico usando o Pandas e o MatplotLib do Python vai auxiliar. Mas isso exige novos módulos que podem ser facilmente instalado usando:

- sudo apt update
- sudo apt install python3-pandas python3-matplotlib

## LAB1

No primeiro trabalho, a idéia era construir apenas as APIs que serão usadas para a execução dos trabalhos seguintes. Por isso:
    
    - A src/main.c está *vazia* propositalmente, visto que virá aí a implementação dos trabalhos seguintes.
    - A implementação dos testes representam a execução do LAB1.
    - Ao executar "make all", o MakeFile se responsabiliza por executar os testes e demonstrar no terminal o resultado final do trabalho. 
