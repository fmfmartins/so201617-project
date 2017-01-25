/*
// Projeto SO - exercicio 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
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
      perror("Could not create 'bufferMutex'.\n");
    }

    for(i = 0; i < NUM_CONTAS; i++){
      if(pthread_mutex_init(&accountsMutexes[i], NULL)){
        perror("Could not create 'accountsMutexes'.\n");
      }
    }

    if(pthread_mutex_init(&logMutex, NULL)){
      perror("Could not create 'logMutex'.\n");
    }
    /* Initialize Condition Variable */

    if(pthread_cond_init(&podeSimular, NULL)){
      perror("Could not create 'podeSimular'.\n");
    }

    /* Create thread pool */
    for (i = 0; i < NUM_TRABALHADORAS; i++) {
      if(pthread_create(&tid[i], 0, worker, NULL)){
          printf("Erro na criacao da tarefa.\n");
          exit(EXIT_FAILURE);
        }
    }
    printf("Bem-vindo ao i-banco\n\n");
    /* Initialize logfile */
    if(!(logfile = fopen("log.txt", "w"))){
      perror("Could not create logfile.txt.\n");
    }
    /* Initialize pipes */
    unlink(cmdpipe);
    if(mkfifo(cmdpipe, 0777)){
      perror("Could not create cmdpipe.");
    }
    cmdpipe_fd = open(cmdpipe, O_RDONLY, S_IWUSR | S_IRUSR);
    while (1) {
        int status;
        comando_t cmd;
        pid_t pid;
        if(read(cmdpipe_fd, &cmd, sizeof(comando_t)) == 0 ) {
        	close(cmdpipe_fd);
        	cmdpipe_fd = open(cmdpipe, O_RDONLY, S_IWUSR | S_IRUSR);
        	continue;
        }
        /* EOF (end of file) do stdin ou comando "sair" */
        if (cmd.operacao == OP_SIMULAR){
          pthread_mutex_lock(&bufferMutex);
          while(numCommands){ pthread_cond_wait(&podeSimular, &bufferMutex); }
          simular(cmd.valor);
          pthread_mutex_unlock(&bufferMutex);
        }
        else if (cmd.operacao == OP_SAIR) {
            sairflag = 1;
            if(cmd.valor == 1){
              kill(0, SIGUSR1);
            }
            printf("i-banco vai terminar.\n--\n");
            /* Send end command to threads and wait for them to end */
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
            close(cmdpipe_fd);
            unlink(cmdpipe);
            exit(EXIT_SUCCESS);
        }
        else{
            writecmd(cmd);
        }
    }
}

/* Handler function for SIGUSR1 */
void sigusr1Handler(){
  sigusr1flag = 1;
  signal(SIGUSR1, sigusr1Handler);
}


/* Write the command that was read from stdin to cmd_buffer */
void writecmd(comando_t cmd){
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
  char message[BUFFER_SIZE];
  comando_t tempCommand;
  char tempPipeName[26]; /* Temporary pipe name */
  int tempFD;/* Temporary file descriptor */
  /* Waits for the buffer to have a command */
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
    
      if (debitar (tempCommand.idConta, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d): Erro\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d): OK\n", COMANDO_DEBITAR, tempCommand.idConta,
              tempCommand.valor);
      break;

    case OP_CREDITAR :
      if (creditar (tempCommand.idConta, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d): Erro\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d): OK\n", COMANDO_CREDITAR, tempCommand.idConta,
              tempCommand.valor);
      break;

    case OP_LER_SALDO :
      saldo = lerSaldo(tempCommand.idConta);
      if (saldo < 0)
        sprintf(message, "%s(%d): Erro.\n", COMANDO_LER_SALDO, tempCommand.idConta);
      else
        sprintf(message, "%s(%d): O saldo da conta é %d.\n", COMANDO_LER_SALDO,
              tempCommand.idConta, saldo);
      break;

    case OP_TRANSFERIR:
      if (transferir (tempCommand.idConta,tempCommand.idContaDestino, tempCommand.valor) < 0)
        sprintf(message, "%s(%d, %d, %d): Erro\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      else
        sprintf(message, "%s(%d, %d, %d): OK\n", COMANDO_TRANSFERIR, tempCommand.idConta,
              tempCommand.idContaDestino,tempCommand.valor);
      break;
    case OP_SAIR :
      return 1;
      break;
  }
  /*Send answer message*/
  sprintf(tempPipeName,"%s-%d",answerpipe,tempCommand.tpid);
  tempFD = open(tempPipeName, O_WRONLY, S_IWUSR | S_IRUSR);
  write(tempFD, message, sizeof(char)*strlen(message));
  close(tempFD);
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
