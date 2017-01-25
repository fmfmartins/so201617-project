#include "commandlinereader.h"
#include "i-banco-terminal.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>



int main(int argc, char** argv){
  char *args[MAXARGS + 1];
  char buffer[BUFFER_SIZE];
  int numargs;
  if (argc < 2){
    printf("Missing argument (pipename).\n");
    exit(EXIT_FAILURE);
  }
  else if (argc > 2){
    printf("Too many arguments given.\n");
    exit(EXIT_FAILURE);
  }

  tpid = getpid();
  sprintf(pipeName,"%s-%d",answerpipe,tpid);
  if(mkfifo(pipeName, 0777)){
    perror("Could not create answerpipe.");
  }
  signal(SIGPIPE,sigpipehandler);

  /* Open the pipe given the argument for write */
  cmdpipe = argv[1];
  if((cmdpipe_fd = open(cmdpipe, O_WRONLY, S_IRUSR | S_IWUSR)) == -1){
    perror("Nao foi possivel aceder ao pipe ");
    exit(EXIT_FAILURE);
  }
  printf("Bem vindo/a a um Terminal i-banco\n\n");
  while(1){
    numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);
    if (numargs == 0)
    /* Nenhum argumento; e volta a pedir */
    continue;

    else if (strcmp(args[0],COMANDO_SAIR_TERMINAL)==0){
      close(cmdpipe_fd);
      close(answerpipe_fd);
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(args[0],COMANDO_SAIR)==0){
      if(numargs >1 && strcmp(args[1],COMANDO_SAIR_AGORA)==0){
        writeCommand(OP_SAIR,0,0,1);
      }
      else{
        writeCommand(OP_SAIR,0,0,0);
      }
      continue;
    }
    else if(strcmp(args[0],COMANDO_SIMULAR)==0){
      int anos ;
      if (numargs < 2) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
        continue;
      }
      anos = atoi(args[1]);
      writeCommand(OP_SIMULAR,0,0,anos);
      continue;
    }
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

    /* Transferir */
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

    else {
      printf("Comando desconhecido. Tente de novo.\n");
      continue;
    }
    if(sigpipeflag){
      char tryagain;
      close(cmdpipe_fd);
      printf("Comando nao executado e perdido\n");
      while(1){
        printf("Quer tentar restabelecer conexao? (y/n) \n> ");
        tryagain = getchar();
        if (tryagain =='y'){
          if (!access(cmdpipe,O_WRONLY)){
            cmdpipe_fd = open(cmdpipe, O_WRONLY, S_IRUSR | S_IWUSR);
            printf("Conexao restabelecida.\n");
            sigpipeflag = 0;
            break;
          }
          else{
            printf("Conexao nao estabelecida.\n");
          }
          getchar(); /* consumes \n */
        }
        else if (tryagain =='n'){
          break;
        }
      }
      continue;
    }
    answerpipe_fd = open(pipeName, O_RDONLY, S_IRUSR | S_IWUSR);
    wipebuf(buf);
    read(answerpipe_fd, buf, sizeof(char)*BUFFER_SIZE);
    close(answerpipe_fd);
    time(&fim);
    tempo = (double)(fim) -(double) (inicio);
    printf("%sTempo de execucao : %f\n\n",buf,tempo);
  }
}

/* Cleans the buffer <bufs> by filling it with '\0' */
void wipebuf(char* bufs){
  int a=0;
  for(a=0;a<BUFFER_SIZE;a++){
    bufs[a] = '\0';
  }
}

/* Write the command that was read from stdin to cmd_buffer */
void writeCommand(int operacao, int idConta,int idContaDestino, int valor){
  comando_t cmd;
  cmd.operacao = operacao;
  cmd.idConta = idConta;
  cmd.idContaDestino = idContaDestino;
  cmd.valor = valor;
  cmd.tpid = tpid;
  write(cmdpipe_fd, &cmd, sizeof(comando_t));
  time(&inicio);
}

/* Auxiliary function used to handle the SIGPIPE signal */
void sigpipehandler(){
  sigpipeflag = 1;
  signal(SIGPIPE,sigpipehandler);
}
