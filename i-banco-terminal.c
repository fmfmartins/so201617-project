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



int main(int argc, char** argv){
  char *args[MAXARGS + 1];
  char buffer[BUFFER_SIZE];
  int numargs;

  /* Open pipe for write */

while(1){
  cmdpipe_fd = open(cmdpipe, O_WRONLY, S_IRUSR | S_IWUSR);
  printf("kek\n");
  numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);
  if (numargs == 0)
    /* Nenhum argumento; e volta a pedir */
    continue;
   else if (strcmp(args[0],COMANDO_SAIR_TERMINAL)==0){
     /* FIX FLUSH */
     close(cmdpipe_fd);
     exit(EXIT_SUCCESS);
   }
   else if (strcmp(args[0],COMANDO_SAIR)==0){
     if(numargs >1 && strcmp(args[1],COMANDO_SAIR_AGORA)==0){
       writeCommand(OP_SAIR,0,1,0);
     }
     else{
        writeCommand(OP_SAIR,0,0,0);
     }
     close(cmdpipe_fd);
     exit(EXIT_SUCCESS);
   }
   else if(strcmp(args[0],COMANDO_SIMULAR)==0){
     int anos ;
     if (numargs < 2) {
         printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
      continue;
     }
     anos = atoi(args[1]);
     writeCommand(OP_SIMULAR,0,0,anos);
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
  }
  printf("kek3\n");
  answerpipe_fd = open(answerpipe, O_RDONLY, S_IRUSR | S_IWUSR);
  printf("kek2\n");
  read(answerpipe_fd, buf, BUFFER_SIZE);
  printf(buf);
  close(answerpipe_fd);
 }
}




/* Write the command that was read from stdin to cmd_buffer */

void writeCommand(int operacao, int idConta,int idContaDestino, int valor){
  comando_t cmd;
  cmd.operacao = operacao;
  cmd.idConta = idConta;
  cmd.idContaDestino = idContaDestino;
  cmd.valor = valor;
  write(cmdpipe_fd, &cmd, sizeof(comando_t));
  close(cmdpipe_fd);
}
