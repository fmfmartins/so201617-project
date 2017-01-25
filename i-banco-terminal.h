#ifndef I_BANCO_TERMINAL_H
#define I_BANCO_TERMINAL_H

#include "commandlinereader.h"
#include <stdio.h>
#include <time.h>


#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"
#define COMANDO_SAIR_TERMINAL "sair-terminal"
#define COMANDO_TRANSFERIR "transferir"
#define OP_TERMINAL_SAIR -1
#define OP_DEBITAR 0
#define OP_CREDITAR 1
#define OP_LER_SALDO 2
#define OP_TRANSFERIR 3
#define OP_SAIR 4
#define OP_SIMULAR 5
#define MAXARGS 4
#define BUFFER_SIZE 100
#define NAO "n"
#define SIM "y"

/* Command structure */
typedef struct {
  int operacao;
  int idConta;
  int idContaDestino;
  int valor;
  pid_t tpid;
} comando_t;


int sigpipeflag = 0 ;
int cmdpipe_fd;
int answerpipe_fd;
char *cmdpipe;
char *answerpipe = "/tmp/i-banco-answer";
char pipeName[26];
char buf[BUFFER_SIZE];
pid_t tpid;
time_t inicio,fim;
double tempo;


void wipebuf(char *x);
void writeCommand(int operacao, int idConta,int idContaDestino, int valor);
void sigpipehandler();


#endif
