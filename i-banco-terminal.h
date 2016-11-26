#ifndef I_BANCO_TERMINAL_H
#define I_BANCO_TERMINAL_H

#include "commandlinereader.h"
#include <stdio.h>


#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"
#define COMANDO_SAIR_TERMINAL "sair-terminal"
#define COMANDO_TRANSFERIR "transferir"
#define OP_DEBITAR 0
#define OP_CREDITAR 1
#define OP_LER_SALDO 2
#define OP_TRANSFERIR 3
#define OP_SAIR 4
#define OP_SIMULAR 5
#define MAXARGS 4
#define BUFFER_SIZE 100
#define NUM_TRABALHADORAS 3 /* Number of threads in pool */
#define CMD_BUFFER_DIM (2 * NUM_TRABALHADORAS) /* command buffer size */

/* Command */
typedef struct {
  int operacao;
  int idConta;
  int idContaDestino;
  int valor;
} comando_t;

int cmdpipe_fd;
int answerpipe_fd;
char *cmdpipe = "/tmp/i-banco-pipe";
char *answerpipe = "/tmp/i-banco-answer";
char buf[BUFFER_SIZE];

void writeCommand(int operacao, int idConta,int idContaDestino, int valor);

#endif
