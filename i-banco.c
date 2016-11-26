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
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>



int main (int argc, char** argv) {

    int i, cmdpipe_fd, answerpipe_fd;
    char *cmdpipe = "/tmp/i-banco-pipe";
    char *answerpipe = "/tmp/i-banco-answer";

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
    printf("save5\n");
    printf("Bem-vindo ao i-banco\n\n");
    printf("save6\n");
    /* Initialize logfile */
    if(!(logfile = fopen("logfile.txt", "w"))){
      perror("Could not create logfile.txt.\n");
    }
    /* Initialize pipes */
    printf("save4\n");
    unlink(cmdpipe);
    if(mkfifo(cmdpipe, 0777)){
      perror("Could not create cmdpipe.");
    }
    printf("saveanswerpipe\n");
    unlink(answerpipe);
    if(mkfifo(answerpipe, 0777)){
      perror("Could not create answerpipe.")
    }

    /* printf("PPID = %d\n", getpid());  DEBUG PRINT */

    while (1) {
        int status;
        comando_t cmd;
        pid_t pid;
        printf("save3\n");
        cmdpipe_fd = open(cmdpipe, O_RDONLY, S_IWUSR | S_IRUSR);
        printf("save1\n");
        read(cmdpipe_fd, &cmd, sizeof(comando_t));
        printf("save9\n");
        close(cmdpipe_fd);
        printf("save2\n");
        writecmd(cmd);

        answerpipe_fd = open(answerpipe, O_WRONLY, S_IWUSR | S_IRUSR);

        /* EOF (end of file) do stdin ou comando "sair" */
        if (cmd.operacao == OP_SIMULAR){
          printf("nunogay\n");
          pthread_mutex_lock(&bufferMutex);
          while(numCommands){ pthread_cond_wait(&podeSimular, &bufferMutex); }
          printf ("dog\n");
          simular(cmd.valor);
          printf("fixe\n");
          pthread_mutex_unlock(&bufferMutex);
          printf("estupido\n");
        }
        if (cmd.operacao == OP_SAIR) {
            sairflag = 1;
            if(cmd.idContaDestino == 1){
              kill(0, SIGUSR1);
            }
            printf("i-banco vai terminar.\n--\n");
            /* Send end command to threads and wait for them to end*/
            for (i=0; i<NUM_TRABALHADORAS;i++){
               writecmd(cmd);
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
    }
}

/* Handler function for SIGUSR1 */
void sigusr1Handler(){
  sigusr1flag = 1;
  printf("OLA");
  signal(SIGUSR1, sigusr1Handler);
}


/* Write the command that was read from stdin to cmd_buffer */
void writecmd(comando_t cmd){
  printf("save8\n");
  if(sem_wait(&tooManyCommands)){
    perror("Could not wait semaphore 'tooManyCommands'. \n");
  }
  pthread_mutex_lock(&bufferMutex);
  cmd_buffer[buff_write_idx]= cmd;
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
    char message[50];
    case OP_DEBITAR :
      if (debitar (tempCommand.idConta, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d): OK\n\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      write(answerpipe_fd, message, sizeof(char)*strlen(message));
      break;

    case OP_CREDITAR :
      if (creditar (tempCommand.idConta, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d): OK\n\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      write(answerpipe_fd, message, sizeof(char)*strlen(message));
      break;

    case OP_LER_SALDO :
      saldo = lerSaldo(tempCommand.idConta);
      if (saldo < 0)
        sprintf(message, "%s(%d): Erro.\n\n", COMANDO_LER_SALDO, tempCommand.idConta);
      else
        sprintf(message, "%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO,
              tempCommand.idConta, saldo);
      write(answerpipe_fd, message, sizeof(char)*strlen(message));
      break;

    case OP_TRANSFERIR:
      if (transferir (tempCommand.idConta,tempCommand.idContaDestino, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d, %d): Erro\n\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      write(answerpipe_fd, message, sizeof(char)*strlen(message));
      break;
    case OP_SAIR :
      // fprintf(logfile, "%lu: %s\n", pthread_self(), COMANDO_SAIR);
      return 1;
      break;
  }
  close(answerpipe_fd);
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
