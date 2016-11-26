/*
// Projeto SO - exercicio 1, version 1
s Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/
#include "i-banco.h"
#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>



int main (int argc, char** argv) {

    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];
    int i;

    inicializarContas();

    /* SET SIGNAL HANDLERS */
    signal(SIGUSR1, sigusr1Handler);

    /* Initialize semaphore */
    if(sem_init(&hasCommand, 0, numCommands)){
      perror("Semaphore 'hasCommand' not created.\n");
    }
    if(sem_init(&tooManyCommands, 0 ,CMD_BUFFER_DIM)){
      perror("Semaphore 'tooManyCommands' not created. \n");
    }

    /* Initialize mutexes */

    if(pthread_mutex_init(&bufferMutex, NULL)){
      perror("Could not create 'bufferMutex'. \n");
    }

    for(i = 0; i < NUM_CONTAS; i++){
      if(pthread_mutex_init(&accountsMutexes[i], NULL)){
        perror("Could not create accountsMutexes.\n");
      }
    }
    /* Initialize Condition Variable */

    if(pthread_cond_init(&podeSimular, NULL)){
      perror("Could not create 'podeSimular'. \n");
    }

    /* Create thread pool */
    for (i = 0; i < NUM_TRABALHADORAS; i++) {
      if(pthread_create(&tid[i], 0, worker, NULL)){
          printf("Erro na criacao da tarefa.\n");
          exit(EXIT_FAILURE);
        }
    }

    printf("Bem-vinda/o ao i-banco\n\n");

    /* Initialize logfile */

    if(!(logfile = fopen("logfile.txt", "w"))){
      perror("Could not create logfile.txt.\n");
    }

    /* printf("PPID = %d\n", getpid());  DEBUG PRINT */

    while (1) {
        int numargs, status;
        pid_t pid;
        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);
        /* EOF (end of file) do stdin ou comando "sair" */
        if (numargs < 0 ||
	        (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0)) || sigtermflag) {
            sairflag = 1;
            if(numargs == 2 && (strcmp(args[1], COMANDO_SAIR_AGORA) == 0)){
              kill(0, SIGUSR1);
            }
            printf("i-banco vai terminar.\n--\n");
            /* Send end command to threads and wait for them to end*/
            for (i=0; i<NUM_TRABALHADORAS;i++){
              writeCommand(OP_SAIR,-1,-1,-1);
            }
            for(i=0;i<NUM_TRABALHADORAS;i++){
              if(pthread_join(tid[i],NULL)){
                perror("Thread had problems in ending");
              }
            }
            /* Check children status, and print the message accordingly */
            while((pid = wait(&status))>0){
              if(WIFEXITED(status)){
                printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", pid);
              }
              else if(WIFSIGNALED(status)){
                printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", pid);
              }
            }

            /* End Program */
            printf("--\ni-banco terminou.\n");
            fclose(logfile);
            exit(EXIT_SUCCESS);
        }

        else if (numargs == 0)
            /* Nenhum argumento; e volta a pedir */
            continue;

        /* Debitar */
        else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
          int idConta, valor;
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
	           continue;
            }
            idConta = atoi(args[1]);
            valor = atoi(args[2]);
            writeCommand(OP_DEBITAR,idConta,-1,valor);

       }

       /* Creditar */
       else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
        int idConta, valor;
        if (numargs < 3) {
          printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
          continue;
        }

        idConta = atoi(args[1]);
        valor = atoi(args[2]);
        writeCommand(OP_CREDITAR, idConta,-1, valor);
      }
        /* Ler Saldo */
        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
          int idConta;

          if (numargs < 2) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
            continue;
          }

          idConta = atoi(args[1]);
          writeCommand(OP_LER_SALDO, idConta,-1, -1);

        }
        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {
          int idContaOrigem,idContaDestino, valor;
          if (numargs < 4) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);
            continue;
          }

          idContaOrigem = atoi(args[1]);
          idContaDestino = atoi(args[2]);
          valor = atoi(args[3]);
          writeCommand(OP_TRANSFERIR, idContaOrigem,idContaDestino, valor);
        }
        /* Simular */
        else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
          int anos;
          if (numargs < 2) {
            printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
            continue;
          }
          anos = atoi(args[1]);
          pthread_mutex_lock(&bufferMutex);
          while(numCommands){ pthread_cond_wait(&podeSimular,&bufferMutex);}
          simular(anos);
          pthread_mutex_unlock(&bufferMutex);
        }

        else {
          printf("Comando desconhecido. Tente de novo.\n");
        }
    }
}

/* Handler function for SIGUSR1 */
void sigusr1Handler(){
  sigusr1flag = 1;
  signal(SIGUSR1, sigusr1Handler);
}


/* Write the command that was read from stdin to cmd_buffer */
void writeCommand(int operacao, int idConta,int idContaDestino, int valor){
  if(sem_wait(&tooManyCommands)){
    perror("Could not wait semaphore 'tooManyCommands'. \n");
  }
  pthread_mutex_lock(&bufferMutex);
  (cmd_buffer[buff_write_idx]).operacao = operacao;
  (cmd_buffer[buff_write_idx]).idConta = idConta;
  (cmd_buffer[buff_write_idx]).idContaDestino = idContaDestino;
  (cmd_buffer[buff_write_idx]).valor = valor;
  buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;
  numCommands++;
  pthread_mutex_unlock(&bufferMutex);

  if(sem_post(&hasCommand)){
    perror("Could not post semaphore 'hasCommand'. \n");
  }
}

int readCommand(){
  comando_t tempCommand;
  /* waits for the buffer to have a command */
  if(sem_wait(&hasCommand)){
    perror("Could not wait semaphore 'hasCommand'. \n");
  }
  pthread_mutex_lock(&bufferMutex);
  /* Critical!!! */
  tempCommand = cmd_buffer[buff_read_idx];
  buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;
  if(sem_post(&tooManyCommands)){
    perror("Could not post semaphore 'tooManyCommands'. \n");
  }
  /* Critical!!! */
  pthread_mutex_unlock(&bufferMutex);
  switch(tempCommand.operacao){
    int saldo;
    case OP_DEBITAR :
      if (debitar (tempCommand.idConta, tempCommand.valor) < 0)
        printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      break;

    case OP_CREDITAR :
      if (creditar (tempCommand.idConta, tempCommand.valor) < 0)
        printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      break;

    case OP_LER_SALDO :
      saldo = lerSaldo(tempCommand.idConta);
      if (saldo < 0)
        printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, tempCommand.idConta);
      else
        printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO,
              tempCommand.idConta, saldo);
      break;

    case OP_TRANSFERIR:
      if (transferir (tempCommand.idConta,tempCommand.idContaDestino, tempCommand.valor) < 0)
        printf("%s(%d, %d, %d): Erro\n\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      else
        printf("%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      break;

    case OP_SAIR :
      // fprintf(logfile, "%lu: %s\n", pthread_self(), COMANDO_SAIR);
      return 1;
      break;
  }
  pthread_mutex_lock(&bufferMutex);
  numCommands--;
  pthread_cond_signal(&podeSimular);
  pthread_mutex_unlock(&bufferMutex);
  return 0;
}

/* Makes sure threads never stop working! Unless when they're told to.*/
void* worker(){
  int sairFlag = 0;
  do{
    sairFlag = readCommand();
  }while(!sairFlag);
  return NULL;
}
