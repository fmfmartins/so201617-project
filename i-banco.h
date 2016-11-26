#ifndef I_BANCO_H
#define I_BANCO_H

#include "commandlinereader.h"
#include "contas.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"
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


int sigflag = 0; /* flag signal global */
int sairflag = 0;
extern int forks; /* number of forks created */
pthread_t tid[NUM_TRABALHADORAS]; /* array containing id of threads */
comando_t cmd_buffer[CMD_BUFFER_DIM]; /* command buffer */
int buff_write_idx = 0, buff_read_idx = 0; /* write and read indexes */
int numCommands = 0; /* total waiting commands */
sem_t hasCommand, tooManyCommands;
pthread_mutex_t bufferMutex; /* Mutex for cmd buffer */
extern pthread_mutex_t accountsMutexes[];
pthread_cond_t podeSimular;
FILE* logfile;

int sigusr1flag = 0; /* flag signal USR 1 global */
int sigtermflag = 0; /* flag signal TERM global to i-banco */

int main (int argc, char** argv);
void sigusr1Handler();
void sigtermHandler();
void writeCommand(int operacao, int idConta,int idContaDestino, int valor);
void writecmd(comando_t cmd);
int readCommand();
void* worker(); /* thread main handling function */


#endif
