# Real-Time-Programming

Trabalhos feitos pra matéria de Programacao em Tempo Real, no intuito de aprender sobre sistemas RTOS.

## LAB22
Para este segundo trabalho, a execução da simulação é o ponto mais importante. Portanto, o arquivo .txt do resultado da simulação é sempre gerado. Mas pra uma visualização mais conveniente, um gráfico usando o Pandas e o MatplotLib do Python vai auxiliar. Mas isso exige novos módulos que podem ser facilmente instalado usando:

sudo apt update
sudo apt install python3-pandas python3-matplotlib

## LAB1

No primeiro trabalho, a idéia era construir apenas as APIs que serão usadas para a execução dos trabalhos seguintes. Por isso:
    
    - A src/main.c está *vazia* propositalmente, visto que virá aí a implementação dos trabalhos seguintes.
    - A implementação dos testes representam a execução do LAB1.
    - Ao executar "make all", o MakeFile se responsabiliza por executar os testes e demonstrar no terminal o resultado final do trabalho. 